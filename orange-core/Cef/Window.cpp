#include "stdafx.h"
namespace CEF
{
	bool Window::initialized = false;

	void Window::Init()
	{
		if (initialized)
			return;
		initialized = true;
		HMODULE libcef = LoadLibraryA((CGlobals::Get().orangePath + "/cef/libcef.dll").c_str());

		if (!libcef)
		{
			MessageBoxW(NULL, L"Could not load cef/libcef.dll.", L"GTA:Orange", MB_ICONSTOP | MB_OK);
			ExitProcess(0);
		}
		CefSettings settings;
		CefMainArgs mainArgs(GetModuleHandle(NULL));
		if (CefExecuteProcess(mainArgs, nullptr, nullptr) == -1)
		{
			CefString(&settings.log_file).FromString((CGlobals::Get().orangePath + "/cef/cefdebug.log").c_str());
			settings.log_severity = cef_log_severity_t::LOGSEVERITY_VERBOSE;
			settings.no_sandbox = true;

			settings.remote_debugging_port = 10228;
			settings.pack_loading_disabled = false; // true;
			settings.windowless_rendering_enabled = true;
			cef_string_utf16_set(L"en-US", 5, &settings.locale, true);
			CefString(&settings.browser_subprocess_path).FromString((CGlobals::Get().orangePath + "/cef/csp.exe").c_str());
			settings.multi_threaded_message_loop = true;
			settings.windowless_rendering_enabled = true;

			CefInitialize(mainArgs, settings, nullptr, nullptr);
		}
	}

	Window::Window(std::string uri, SIZE size, POINT position, bool plugins, bool javascript)
	{
		this->size = size;
		this->position = position;
		CefBrowserSettings browserSettings;
		browserSettings.windowless_frame_rate = 60;
		browserSettings.javascript_access_clipboard = cef_state_t::STATE_ENABLED;
		browserSettings.caret_browsing = cef_state_t::STATE_ENABLED;
		browserSettings.universal_access_from_file_urls = cef_state_t::STATE_ENABLED; // Also filtered by resource interceptor, but set this nevertheless
		browserSettings.file_access_from_file_urls = cef_state_t::STATE_ENABLED;
		browserSettings.webgl = cef_state_t::STATE_ENABLED;
		browserSettings.javascript_open_windows = cef_state_t::STATE_ENABLED;
		browserSettings.plugins = plugins ? cef_state_t::STATE_ENABLED : cef_state_t::STATE_DISABLED;
		browserSettings.javascript = javascript ? cef_state_t::STATE_ENABLED : cef_state_t::STATE_DISABLED;
		CefWindowInfo windowInfo;
		windowInfo.SetAsWindowless(CGlobals::Get().gtaHwnd, true);
		CefBrowserHost::CreateBrowser(windowInfo, this, uri.c_str(), browserSettings, nullptr);
	}


	Window::~Window()
	{
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefClient::OnProcessMessageReceived            //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefClient.html#OnProcessMessageReceived(CefRefPtr%3CCefBrowser%3E,CefProcessId,CefRefPtr%3CCefProcessMessage%3E) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
	{
		//CefRefPtr<CefListValue> argList = message->GetArgumentList();
		//if (message->GetName() == "TriggerLuaEvent")
		//{
		//	if (!m_bIsLocal)
		//		return true;

		//	// Get event name
		//	CefString eventName = argList->GetString(0);

		//	// Get number of arguments from IPC process message
		//	int numArgs = argList->GetInt(1);

		//	// Get args
		//	std::vector<std::string> args;
		//	for (int i = 2; i < numArgs + 2; ++i)
		//	{
		//		args.push_back(argList->GetString(i));
		//	}

		//	// Queue event to run on the main thread
		//	auto func = std::bind(&CWebBrowserEventsInterface::Events_OnTriggerEvent, m_pEventsInterface, SString(eventName), args);
		//	g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnProcessMessageReceived1");

		//	// The message was handled
		//	return true;
		//}
		//if (message->GetName() == "InputFocus")
		//{
		//	// Retrieve arguments from process message
		//	m_bHasInputFocus = argList->GetBool(0);

		//	// Queue event to run on the main thread
		//	auto func = std::bind(&CWebBrowserEventsInterface::Events_OnInputFocusChanged, m_pEventsInterface, m_bHasInputFocus);
		//	g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnProcessMessageReceived2");
		//}

		//// The message wasn't handled
		return false;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRenderHandler::GetViewRect                  //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#GetViewRect(CefRefPtr%3CCefBrowser%3E,CefRect&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
	{
		/*if (m_bBeingDestroyed)
		return false;

		IDirect3DSurface9* pD3DSurface = m_pWebBrowserRenderItem->m_pD3DRenderTargetSurface;
		if (!pD3DSurface)
		return false;

		D3DSURFACE_DESC SurfaceDesc;
		pD3DSurface->GetDesc(&SurfaceDesc);

		rect.x = 0;
		rect.y = 0;
		rect.width = SurfaceDesc.Width;
		rect.height = SurfaceDesc.Height;*/
		rect.x = position.x;
		rect.y = position.x;
		rect.width = size.cx;
		rect.height = size.cy;
		return true;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRenderHandler::OnPopupSize                  //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPopupSize(CefRefPtr<CefBrowser>,constCefRect&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
	{
		/*m_RenderPopupOffsetX = rect.x;
		m_RenderPopupOffsetY = rect.y;*/
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRenderHandler::OnPaint                      //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPaint(CefRefPtr%3CCefBrowser%3E,PaintElementType,constRectList&,constvoid*,int,int) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
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
		//if (m_bBeingDestroyed)
		//	return;

		//IDirect3DSurface9* pD3DSurface = m_pWebBrowserRenderItem->m_pD3DRenderTargetSurface;
		//if (!pD3DSurface)
		//	return;

		///*
		//DirectX9 is not thread safe, so locking the texture from another thread causes D3DDevice::Present to block forever (-> client freeze)
		//The only way we can get rid of this problem is (imo) locking CEF until the texture has been written (unlocking/notifying is done in CWebCore::DoEventQueuePulse).
		//If someone knows a better solution than this weird construction, feel free to contact me (Jusonex).
		//*/

		//auto f = [&]() {
		//	if (m_bBeingDestroyed)
		//		return;

		//	D3DLOCKED_RECT LockedRect;
		//	D3DSURFACE_DESC SurfaceDesc;

		//	pD3DSurface->GetDesc(&SurfaceDesc);
		//	pD3DSurface->LockRect(&LockedRect, NULL, 0);

		//	// Dirty rect implementation, don't use this as loops are significantly slower than memcpy
		//	/*auto surfaceData = (int*)LockedRect.pBits;
		//	auto sourceData = (const int*)buffer;
		//	auto pitch = LockedRect.Pitch;
		//	for (auto& rect : dirtyRects)
		//	{
		//	for (int y = rect.y; y < rect.y+rect.height; ++y)
		//	{
		//	for (int x = rect.x; x < rect.x+rect.width; ++x)
		//	{
		//	int index = y * pitch / 4 + x;
		//	surfaceData[index] = sourceData[index];
		//	}
		//	}
		//	}*/

		//	// Copy entire texture
		//	int theoreticalPitch = width * 4;
		//	if (LockedRect.Pitch == theoreticalPitch)
		//		memcpy(LockedRect.pBits, buffer, theoreticalPitch * height);
		//	else
		//	{
		//		// We've to skip a few rows if this is a popup draw
		//		// The pitch will never equal the theoretical pitch (see above) by definition
		//		int skipRows = 0;
		//		int skipPixels = 0;
		//		if (paintType == PaintElementType::PET_POPUP)
		//		{
		//			skipRows = m_RenderPopupOffsetY;
		//			skipPixels = m_RenderPopupOffsetX;
		//		}

		//		uint32 destAddress = (uint32)LockedRect.pBits + skipPixels * 4;
		//		for (int i = 0; i < height; ++i)
		//		{
		//			memcpy((void*)(destAddress + LockedRect.Pitch * (i + skipRows)), (void*)((uint32)buffer + theoreticalPitch * i), theoreticalPitch);
		//		}
		//	}

		//	pD3DSurface->UnlockRect();
		//};
		//g_pCore->GetWebCore()->AddEventToEventQueue(f, this, "OnPaint");

		//std::unique_lock<std::mutex> lock(m_PaintMutex);
		//m_PaintCV.wait(lock);

		//static bool createdClient;

		//static HANDLE lastParentHandle;

		//HANDLE parentHandle = GetModuleHandle(NULL);
		//if (parentHandle)
		//{
		//	if (lastParentHandle != parentHandle)
		//	{
		//		lastParentHandle = parentHandle;

		//		ID3D11Device* device = pD3D11_Device;

		//		ID3D11Resource* resource = nullptr;
		//		if (SUCCEEDED(device->OpenSharedResource(parentHandle, __uuidof(IDXGIResource), (void**)&resource)))
		//		{
		//			ID3D11Texture2D* texture;
		//			assert(SUCCEEDED(resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture)));

		//			NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
		//			ID3D11Texture2D* oldTexture = nullptr;
		//			

		//			if (wm->GetParentTexture())
		//			{
		//				oldTexture = wm->GetParentTexture();
		//			}

		//			wm->SetParentTexture(texture);

		//			// only release afterward to prevent the parent texture being invalid
		//			if (oldTexture)
		//			{
		//				oldTexture->Release();
		//			}

		//			createdClient = true;
		//		}
		//	}
		//}
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRenderHandler::OnCursorChange               //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnCursorChange(CefRefPtr%3CCefBrowser%3E,CefCursorHandle) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& cursorInfo)
	{
		//// Find the cursor index by the cursor handle
		//unsigned char cursorIndex = static_cast < unsigned char > (type);

		//// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnChangeCursor, m_pEventsInterface, cursorIndex);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnCursorChange");
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLoadHandler::OnLoadStart                    //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadStart(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
	{
		//SString strURL = UTF16ToMbUTF8(frame->GetURL());
		//if (strURL == "blank")
		//	return;

		//// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnLoadingStart, m_pEventsInterface, strURL, frame->IsMain());
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnLoadStart");
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLoadHandler::OnLoadEnd                      //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadEnd(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,int) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
	{
		//// Set browser volume once again
		//SetAudioVolume(m_fVolume);

		//if (frame->IsMain())
		//{
		//	SString strURL = UTF16ToMbUTF8(frame->GetURL());

		//	// Queue event to run on the main thread
		//	auto func = std::bind(&CWebBrowserEventsInterface::Events_OnDocumentReady, m_pEventsInterface, strURL);
		//	g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnLoadEnd");
		//}
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLoadHandler::OnLoadError                    //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLoadHandler.html#OnLoadError(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,ErrorCode,constCefString&,constCefString&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedURL)
	{
		//SString strURL = UTF16ToMbUTF8(frame->GetURL());

		//// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnLoadingFailed, m_pEventsInterface, strURL, errorCode, SString(errorText));
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnLoadError");
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRequestHandler::OnBeforeBrowe               //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRequestHandler.html#OnBeforeBrowse(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,CefRefPtr%3CCefRequest%3E,bool) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool isRedirect)
	{
		/*
		From documentation:
		The |request| object cannot be modified in this callback.
		CefLoadHandler::OnLoadingStateChange will be called twice in all cases. If the navigation is allowed CefLoadHandler::OnLoadStart and CefLoadHandler::OnLoadEnd will be called.
		If the navigation is canceled CefLoadHandler::OnLoadError will be called with an |errorCode| value of ERR_ABORTED.
		*/

		//CefURLParts urlParts;
		//if (!CefParseURL(request->GetURL(), urlParts))
		//	return true; // Cancel if invalid URL (this line will normally not be executed)

		//bool bResult;
		//WString scheme = urlParts.scheme.str;
		//if (scheme == L"http" || scheme == L"https")
		//{
		//	SString host = UTF16ToMbUTF8(urlParts.host.str);
		//	if (host != "mta")
		//	{
		//		if (IsLocal() || g_pCore->GetWebCore()->GetURLState(host, true) != eURLState::WEBPAGE_ALLOWED)
		//			bResult = true; // Block remote here
		//		else
		//			bResult = false; // Allow
		//	}
		//	else
		//		bResult = false;
		//}
		//else if (scheme == L"mtalocal")
		//	bResult = false; // Allow mtalocal:// URLs
		//else
		//	bResult = true; // Block other schemes

		//// Check if we're in the browser's main frame or only a frame element of the current page
		//bool bIsMainFrame = frame->IsMain();

		//// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnNavigate, m_pEventsInterface, SString(request->GetURL()), bResult, bIsMainFrame);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnNavigate");

		//// Return execution to CEF
		//return bResult;
		return false;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefRequestHandler::OnBeforeResourceLoad        //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRequestHandler.html#OnBeforeResourceLoad(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,CefRefPtr%3CCefRequest%3E) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	CefRequestHandler::ReturnValue Window::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback)
	{
		//// Mostly the same as Window::OnBeforeBrowse
		//CefURLParts urlParts;
		//if (!CefParseURL(request->GetURL(), urlParts))
		//	return RV_CANCEL; // Cancel if invalid URL (this line will normally not be executed)

		//// Add some information to the HTTP header
		//{
		//	CefRequest::HeaderMap headerMap;
		//	request->GetHeaderMap(headerMap);
		//	auto iter = headerMap.find("User-Agent");

		//	if (iter != headerMap.end())
		//	{
		//		// Add MTA:SA "watermark"
		//		iter->second = iter->second.ToString() + "; " MTA_CEF_USERAGENT;

		//		// Add 'Android' to get the mobile version
		//		SString strPropertyValue;
		//		if (GetProperty("mobile", strPropertyValue) && strPropertyValue == "1")
		//			iter->second = iter->second.ToString() + "; Mobile Android";

		//		request->SetHeaderMap(headerMap);
		//	}
		//}

		//WString scheme = urlParts.scheme.str;
		//if (scheme == L"http" || scheme == L"https")
		//{
		//	SString domain = UTF16ToMbUTF8(urlParts.host.str);
		//	if (domain != "mta")
		//	{
		//		if (IsLocal())
		//			return RV_CANCEL; // Block remote requests in local mode generally

		//		eURLState urlState = g_pCore->GetWebCore()->GetURLState(domain, true);
		//		if (urlState != eURLState::WEBPAGE_ALLOWED)
		//		{
		//			// Trigger onClientBrowserResourceBlocked event
		//			auto func = std::bind(&CWebBrowserEventsInterface::Events_OnResourceBlocked, m_pEventsInterface,
		//				SString(request->GetURL()), domain, urlState == eURLState::WEBPAGE_NOT_LISTED ? 0 : 1);
		//			g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnResourceBlocked");

		//			return RV_CANCEL; // Block if explicitly forbidden
		//		}

		//		// Allow
		//		return RV_CONTINUE;
		//	}
		//	else
		//		return RV_CONTINUE;
		//}
		//else if (scheme == L"mtalocal")
		//{
		//	// Allow :)
		//	return RV_CONTINUE;
		//}

		//// Trigger onClientBrowserResourceBlocked event
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnResourceBlocked, m_pEventsInterface, SString(request->GetURL()), "", 2); // reason 1 := blocked protocol scheme
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnResourceBlocked");

		//// Block everything else
		//return RV_CANCEL;
		return RV_CONTINUE;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLifeSpanHandler::OnBeforeClose              //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnBeforeClose(CefRefPtr%3CCefBrowser%3E) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnBeforeClose(CefRefPtr<CefBrowser> browser)
	{
		// Remove events owned by this webview and invoke left callbacks
		//g_pCore->GetWebCore()->RemoveWebViewEvents(this);
		//NotifyPaint();

		//m_pWebView = nullptr;

		//// Remove focused web view reference
		//if (g_pCore->GetWebCore()->GetFocusedWebView() == this)
		//	g_pCore->GetWebCore()->SetFocusedWebView(nullptr);
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLifeSpanHandler::OnBeforePopup              //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnBeforePopup(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,constCefString&,constCefString&,constCefPopupFeatures&,CefWindowInfo&,CefRefPtr%3CCefClient%3E&,CefBrowserSettings&,bool*) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access)
	{
		//// ATTENTION: This method is called on the IO thread

		//// Trigger the popup/new tab event
		//SString strTagetURL = UTF16ToMbUTF8(target_url);
		//SString strOpenerURL = UTF16ToMbUTF8(frame->GetURL());

		//// Queue event to run on the main thread
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnPopup, m_pEventsInterface, strTagetURL, strOpenerURL);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnBeforePopup");

		//// Block popups generally
		return true;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefLifeSpanHandler::OnAfterCreated             //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnAfterCreated(CefRefPtr<CefBrowser>) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		m_Browser = browser;
		//DBG
		//browser->GetHost()->Invalidate(CefBrowserHost::PaintElementType::PET_VIEW);
		//// Set web view reference
		//m_pWebView = browser;

		//// Call created event callback
		//auto func = std::bind(&CWebBrowserEventsInterface::Events_OnCreated, m_pEventsInterface);
		//g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnAfterCreated");
	}


	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefJSDialogHandler::OnJSDialog                 //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefJSDialogHandler.html#OnJSDialog(CefRefPtr%3CCefBrowser%3E,constCefString&,constCefString&,JSDialogType,constCefString&,constCefString&,CefRefPtr%3CCefJSDialogCallback%3E,bool&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, CefJSDialogHandler::JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message)
	{
		// TODO: Provide a way to influence Javascript dialogs via Lua
		// e.g. addEventHandler("onClientBrowserDialog", browser, function(message, defaultText) continueBrowserDialog("My input") end)

		// Suppress the dialog
		suppress_message = true;
		return false;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefDialogHandler::OnFileDialog                 //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDialogHandler.html#OnFileDialog(CefRefPtr%3CCefBrowser%3E,FileDialogMode,constCefString&,constCefString&,conststd::vector%3CCefString%3E&,CefRefPtr%3CCefFileDialogCallback%3E) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_name, const std::vector< CefString >& accept_types, int selected_accept_filter, CefRefPtr< CefFileDialogCallback > callback)
	{
		// Don't show the dialog
		return true;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefDisplayHandler::OnTitleChange               //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDisplayHandler.html#OnTitleChange(CefRefPtr%3CCefBrowser%3E,constCefString&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	void Window::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
	{
		//m_CurrentTitle = UTF16ToMbUTF8(title);
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefDisplayHandler::OnTooltip                   //
	// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefDisplayHandler.html#OnTooltip(CefRefPtr%3CCefBrowser%3E,CefString&) //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& title)
	{
		// Queue event to run on the main thread
		/*auto func = std::bind(&CWebBrowserEventsInterface::Events_OnTooltip, m_pEventsInterface, UTF16ToMbUTF8(title));
		g_pCore->GetWebCore()->AddEventToEventQueue(func, this, "OnTooltip");*/

		return true;
	}

	////////////////////////////////////////////////////////////////////
	//                                                                //
	// Implementation: CefDisplayHandler::OnConsoleMessage            //
	// http://magpcss.org/ceforum/apidocs/projects/%28default%29/CefDisplayHandler.html#OnConsoleMessage%28CefRefPtr%3CCefBrowser%3E,constCefString&,constCefString&,int%29 //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	bool Window::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
	{
		//// Redirect console message to debug window (if development mode is enabled)
		//if (g_pCore->GetWebCore()->IsTestModeEnabled())
		//{
		//	g_pCore->GetWebCore()->AddEventToEventQueue([message, source]() {
		//		g_pCore->DebugPrintfColor("[BROWSER] Console: %s (%s)", 255, 0, 0, UTF16ToMbUTF8(message).c_str(), UTF16ToMbUTF8(source).c_str());
		//	}, this, "OnConsoleMessage");

		//}

		return true;
	}
};