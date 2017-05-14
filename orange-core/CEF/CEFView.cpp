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
	windowInfo.SetAsWindowless(CGlobals::Get().gtaHwnd, false);

	bool test = CefBrowserHost::CreateBrowser(windowInfo, this, "http://google.de/", browserSettings, nullptr);
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

	rect.x = 0;
	rect.y = 0;
	rect.width = 500;
	rect.height = 500;
	log << "GetViewRect" << std::endl;
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
	std::string filename((CGlobals::Get().orangePath + "/webpage.bmp").c_str());

	void* bits;

	// Populate the bitmap info header.
	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = -height;  // minus means top-down bitmap
	info.biPlanes = 1;
	info.biBitCount = 32;
	info.biCompression = BI_RGB;  // no compression
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 1;
	info.biYPelsPerMeter = 1;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	// Create the bitmap and retrieve the bit buffer.
	HDC screen_dc = GetDC(NULL);
	HBITMAP bitmap = CreateDIBSection(screen_dc, reinterpret_cast<BITMAPINFO*>(&info), DIB_RGB_COLORS, &bits, NULL, 0);
	ReleaseDC(NULL, screen_dc);

	// Read the image into the bit buffer.
	if (bitmap == NULL)
		return;

	/*if (!browser->GetImage(PET_VIEW, width, height, bits))
	return;
	*/
	// Populate the bitmap file header.
	BITMAPFILEHEADER file;
	file.bfType = 0x4d42;
	file.bfSize = sizeof(BITMAPFILEHEADER);
	file.bfReserved1 = 0;
	file.bfReserved2 = 0;
	file.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Write the bitmap to file.
	HANDLE file_handle = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (file_handle != INVALID_HANDLE_VALUE)
	{
		DWORD bytes_written = 0;
		WriteFile(file_handle, &file, sizeof(file), &bytes_written, 0);
		WriteFile(file_handle, &info, sizeof(info), &bytes_written, 0);
		WriteFile(file_handle, buffer, width * height * 4, &bytes_written, 0);
		CloseHandle(file_handle);
	}

	DeleteObject(bitmap);
	log << "OnPaint" << std::endl;
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
	log << "ReturnValue" << std::endl;
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