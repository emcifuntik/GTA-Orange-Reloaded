#pragma once
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_parser.h>
#include <include/cef_task.h>
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/internal/cef_types_wrappers.h>
#include <include/internal/cef_win.h>
#include <include/internal/cef_types.h>
#include <include/cef_origin_whitelist.h>

namespace CEF
{
	class Kuchen
		:
		private CefClient,
		private CefRenderHandler,
		private CefLoadHandler,
		private CefRequestHandler,
		private CefLifeSpanHandler,
		private CefJSDialogHandler,
		private CefDialogHandler,
		private CefDisplayHandler
	{
		static bool initialized;
		CefBrowser *m_Browser = nullptr;
		SIZE size;
		POINT position;

		// CefClient methods
		virtual CefRefPtr<CefRenderHandler>     GetRenderHandler() override { return this; };
		virtual CefRefPtr<CefLoadHandler>       GetLoadHandler() override { return this; };
		virtual CefRefPtr<CefRequestHandler>    GetRequestHandler() override { return this; };
		virtual CefRefPtr<CefLifeSpanHandler>   GetLifeSpanHandler() override { return this; };
		virtual CefRefPtr<CefJSDialogHandler>   GetJSDialogHandler() override { return this; };
		virtual CefRefPtr<CefDialogHandler>     GetDialogHandler() override { return this; };
		virtual CefRefPtr<CefDisplayHandler>    GetDisplayHandler() override { return this; };
		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

		// CefRenderHandler methods
		virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
		virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
		virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height) override;
		virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& cursorInfo) override;

		// CefBase methods
		void AddRef() const {}
		bool Release() const { return true; }
		bool HasOneRef() const { return true; }
		// CefLoadHandler methods
		void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);
		void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
		void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedURL);

		//// CefRequestHandler methods
		virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool isRedirect) override;
		virtual CefRequestHandler::ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) override;

		//// CefLifeSpawnHandler methods
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
		virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access) override;
		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

		//// CefJSDialogHandler methods
		virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, CefJSDialogHandler::JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message) override;

		//// CefDialogHandler methods
		virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_name, const std::vector< CefString >& accept_types, int selected_accept_filter, CefRefPtr< CefFileDialogCallback > callback) override;

		//// CefDisplayHandler methods
		virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
		virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) override;
		virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;
	public:
		static void Init();
		Kuchen(std::string uri, SIZE size, POINT position, bool plugins = true, bool javascript = true);
		~Kuchen();
	};

};
