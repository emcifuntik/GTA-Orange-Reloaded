/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CEFView.cpp
*  PURPOSE:     Web view class
*
*****************************************************************************/
#include "stdafx.h"
#include <include/cef_parser.h>
#include <include/cef_task.h>

CEFView::CEFView(bool bIsLocal, bool bTransparent)
{
	m_bIsLocal = bIsLocal;
	m_bIsTransparent = bTransparent;
	m_bBeingDestroyed = false;
	m_fVolume = 1.0f;
	//memset(m_mouseButtonStates, 0, sizeof(m_mouseButtonStates));

	// Initialise properties
	//    m_Properties["mobile"] = "0";
}

CEFView::~CEFView()
{
	/*    if ( IsMainThread () )
	{
	if ( g_pCore->GetWebCore ()->GetFocusedWebView () == this )
	g_pCore->GetWebCore ()->SetFocusedWebView ( nullptr );
	}*/

	// Ensure that CefRefPtr::~CefRefPtr doesn't try to release it twice (it has already been released in CEFView::OnBeforeClose)
	m_pWebView = nullptr;

	// Make sure we don't dead lock the CEF render thread
	m_RenderData.cv.notify_all();
}

void CEFView::Initialise()
{
	//ID3D11Texture2D* m_pTexture;

	auto viewPort = GTA::CViewportGame::Get();

	// Setup the description of the texture
	D3D11_TEXTURE2D_DESC m_TextureDesc;
	ZeroMemory(&m_TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	m_TextureDesc.Width = viewPort->Width;
	m_TextureDesc.Height = viewPort->Height;
	m_TextureDesc.MipLevels = 1;
	m_TextureDesc.ArraySize = 1;
	m_TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_TextureDesc.SampleDesc.Count = 1;
	m_TextureDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE /*| D3D11_BIND_RENDER_TARGET*/;
	m_TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// Create the empty texture
	CGlobals::Get().d3dDevice->CreateTexture2D(&m_TextureDesc, nullptr, &CGlobals::Get().m_pTexture);

	// Setup the shader resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC m_SrvDesc;
	m_SrvDesc.Format = m_TextureDesc.Format;
	m_SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	m_SrvDesc.Texture2D.MostDetailedMip = 0;
	m_SrvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	CGlobals::Get().d3dDevice->CreateShaderResourceView(CGlobals::Get().m_pTexture, &m_SrvDesc, &CGlobals::Get().m_pTextureView);

	// Initialise the web session (which holds the actual settings) in in-memory mode
	CefBrowserSettings browserSettings;
	//browserSettings.windowless_frame_rate = g_pCore->GetFrameRateLimit();
	browserSettings.javascript_access_clipboard = cef_state_t::STATE_DISABLED;
	browserSettings.universal_access_from_file_urls = cef_state_t::STATE_DISABLED; // Also filtered by resource interceptor, but set this nevertheless
	browserSettings.file_access_from_file_urls = cef_state_t::STATE_DISABLED;
	browserSettings.webgl = cef_state_t::STATE_ENABLED;
	browserSettings.javascript_open_windows = cef_state_t::STATE_DISABLED;

	browserSettings.plugins = cef_state_t::STATE_DISABLED;
	if (!m_bIsLocal)
	{
		bool bEnabledJavascript = false;
		browserSettings.javascript = bEnabledJavascript ? cef_state_t::STATE_ENABLED : cef_state_t::STATE_DISABLED;
	}

	CefWindowInfo windowInfo;
	windowInfo.SetAsWindowless(CGlobals::Get().gtaHwnd, true);

	//bool test = CefBrowserHost::CreateBrowser(windowInfo, this, "https://gta-orange.net/hidev/cef.html", browserSettings, nullptr);
	bool test = CefBrowserHost::CreateBrowser(windowInfo, this, "https://codepen.io/Alca/pen/JWZwEa", browserSettings, nullptr);
}
bool CEFView::CanGoBack()
{
	if (!m_pWebView)
		return false;

	return m_pWebView->CanGoBack();
}

bool CEFView::CanGoForward()
{
	if (!m_pWebView)
		return false;

	return m_pWebView->CanGoForward();
}

bool CEFView::GoBack()
{
	if (!m_pWebView)
		return false;

	if (!m_pWebView->CanGoBack())
		return false;

	m_pWebView->GoBack();
	return true;
}

bool CEFView::GoForward()
{
	if (!m_pWebView)
		return false;

	if (!m_pWebView->CanGoForward())
		return false;

	m_pWebView->GoForward();
	return true;
}

void CEFView::Refresh(bool bIgnoreCache)
{
	if (!m_pWebView)
		return;

	if (bIgnoreCache)
	{
		m_pWebView->ReloadIgnoreCache();
	}
	else
	{
		m_pWebView->Reload();
	}
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefClient::OnProcessMessageReceived            //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefClient.html#OnProcessMessageReceived(CefRefPtr%3CCefBrowser%3E,CefProcessId,CefRefPtr%3CCefProcessMessage%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	CefRefPtr<CefListValue> argList = message->GetArgumentList();
	if (message->GetName() == "TriggerLuaEvent")
	{
		if (!m_bIsLocal)
			return true;

		// Get event name
		CefString eventName = argList->GetString(0);

		// Get number of arguments from IPC process message
		int numArgs = argList->GetInt(1);

		// Get args
		std::vector<std::string> args;
		for (int i = 2; i < numArgs + 2; ++i)
		{
			args.push_back(argList->GetString(i));
		}

		// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnTriggerEvent, m_pEventsInterface, SString(eventName), args);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnProcessMessageReceived1");

		// The message was handled
		return true;
	}
	if (message->GetName() == "InputFocus")
	{
		// Retrieve arguments from process message
		m_bHasInputFocus = argList->GetBool(0);

		// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnInputFocusChanged, m_pEventsInterface, m_bHasInputFocus);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnProcessMessageReceived2");
	}

	// The message wasn't handled
	return false;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::GetViewRect                  //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#GetViewRect(CefRefPtr%3CCefBrowser%3E,CefRect&) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	if (m_bBeingDestroyed)
		return false;

	auto viewPort = GTA::CViewportGame::Get();

	rect.x = 0;
	rect.y = 0;
	rect.width = viewPort->Width;
	rect.height = viewPort->Height;
	return true;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::OnPopupShow                  //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPopupShow(CefRefPtr<CefBrowser>,bool) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	log << "OnPopupShow" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::OnPopupSize                  //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPopupSize(CefRefPtr<CefBrowser>,constCefRect&) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	log << "OnPopupSize" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::OnPaint                      //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPaint(CefRefPtr%3CCefBrowser%3E,PaintElementType,constRectList&,constvoid*,int,int) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
{
	CGlobals::Get().cefmutex.lock();

	CGlobals::Get().cefsize.fX = width;
	CGlobals::Get().cefsize.fY = height;
	CGlobals::Get().cefbuffer = (void*)buffer;
	CGlobals::Get().dirtybuffer = true;

	CGlobals::Get().cefmutex.unlock();
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::OnCursorChange               //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnCursorChange(CefRefPtr%3CCefBrowser%3E,CefCursorHandle) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& cursorInfo)
{
	log << "OnCursorChange" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLoadHandler::OnLoadStart                    //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadStart(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType)
{
	log << "OnLoadStart" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLoadHandler::OnLoadEnd                      //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadEnd(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,int) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	log << "OnLoadEnd" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLoadHandler::OnLoadError                    //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadError(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,ErrorCode,constCefString&,constCefString&) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedURL)
{
	log << "OnLoadError " << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRequestHandler::OnBeforeBrowe               //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRequestHandler.html#OnBeforeBrowse(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,CefRefPtr%3CCefRequest%3E,bool) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool isRedirect)
{

	log << "OnBeforeBrowse" << std::endl;
	// Return execution to CEF
	return false;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRequestHandler::OnBeforeResourceLoad        //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRequestHandler.html#OnBeforeResourceLoad(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,CefRefPtr%3CCefRequest%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
CefRequestHandler::ReturnValue CEFView::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback)
{
	//log << "ReturnValue" << std::endl;
	return RV_CONTINUE;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLifeSpanHandler::OnBeforeClose              //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnBeforeClose(CefRefPtr%3CCefBrowser%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	log << "OnBeforeClose" << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLifeSpanHandler::OnBeforePopup              //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnBeforePopup(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,constCefString&,constCefString&,constCefPopupFeatures&,CefWindowInfo&,CefRefPtr%3CCefClient%3E&,CefBrowserSettings&,bool*) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access)
{
	log << "OnBeforePopup" << std::endl;
	return true;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLifeSpanHandler::OnAfterCreated             //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnAfterCreated(CefRefPtr<CefBrowser>) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	log << "OnAfterCreated" << std::endl;
	// Set web view reference
	m_pWebView = browser;
}


////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefJSDialogHandler::OnJSDialog                 //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefJSDialogHandler.html#OnJSDialog(CefRefPtr%3CCefBrowser%3E,constCefString&,constCefString&,JSDialogType,constCefString&,constCefString&,CefRefPtr%3CCefJSDialogCallback%3E,bool&) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, CefJSDialogHandler::JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message)
{
	log << "OnJSDialog" << std::endl;
	suppress_message = true;
	return false;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefDialogHandler::OnFileDialog                 //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDialogHandler.html#OnFileDialog(CefRefPtr%3CCefBrowser%3E,FileDialogMode,constCefString&,constCefString&,conststd::vector%3CCefString%3E&,CefRefPtr%3CCefFileDialogCallback%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_name, const std::vector< CefString >& accept_types, int selected_accept_filter, CefRefPtr< CefFileDialogCallback > callback)
{
	log << "OnFileDialog" << std::endl;
	return true;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefDisplayHandler::OnTitleChange               //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDisplayHandler.html#OnTitleChange(CefRefPtr%3CCefBrowser%3E,constCefString&) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	log << "OnTitleChange: " << std::endl;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefDisplayHandler::OnTooltip                   //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDisplayHandler.html#OnTooltip(CefRefPtr%3CCefBrowser%3E,CefString&) //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& title)
{
	log << "OnTooltip" << std::endl;

	return true;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefDisplayHandler::OnConsoleMessage            //
// http://magpcss.org/ceforum/apidocs/projects/%28default%29/CefDisplayHandler.html#OnConsoleMessage%28CefRefPtr%3CCefBrowser%3E,constCefString&,constCefString&,int%29 //
//                                                                //
////////////////////////////////////////////////////////////////////
bool CEFView::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
	log << "OnConsoleMessage" << std::endl;

	return true;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefContextMenuHandler::OnBeforeContextMenu     //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefContextMenuHandler.html#OnBeforeContextMenu(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,CefRefPtr%3CCefContextMenuParams%3E,CefRefPtr%3CCefMenuModel%3E) //
//                                                                //
////////////////////////////////////////////////////////////////////
void CEFView::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	log << "OnBeforeContextMenu" << std::endl;
	// Show no context menu
	model->Clear();
}