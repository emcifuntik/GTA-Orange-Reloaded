/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CWebView.h
*  PURPOSE:     Web view class
*
*****************************************************************************/

#undef GetNextSibling
#undef GetFirstChild

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/cef_context_menu_handler.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <condition_variable>

class CEFView : private CefClient,
	private CefRenderHandler, private CefLoadHandler, private CefRequestHandler, private CefLifeSpanHandler,
	private CefJSDialogHandler, private CefDialogHandler, private CefDisplayHandler, private CefContextMenuHandler
{
public:
	CEFView(std::string url, bool bIsLocal, bool bTransparent = false);
	virtual ~CEFView();
	void Initialise();
	void CreateTexture();
	void CheckResize(int width, int height);

	bool CanGoBack();
	bool CanGoForward();
	bool GoBack();
	bool GoForward();
	void Refresh(bool ignoreCache);

	// CefClient methods
	virtual CefRefPtr<CefRenderHandler>     GetRenderHandler() override { return this; };
	virtual CefRefPtr<CefLoadHandler>       GetLoadHandler() override { return this; };
	virtual CefRefPtr<CefRequestHandler>    GetRequestHandler() override { return this; };
	virtual CefRefPtr<CefLifeSpanHandler>   GetLifeSpanHandler() override { return this; };
	virtual CefRefPtr<CefJSDialogHandler>   GetJSDialogHandler() override { return this; };
	virtual CefRefPtr<CefDialogHandler>     GetDialogHandler() override { return this; };
	virtual CefRefPtr<CefDisplayHandler>    GetDisplayHandler() override { return this; };
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; };
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

	// CefRenderHandler methods
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height) override;
	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& cursorInfo) override;

	// CefLoadHandler methods
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType) override;
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedURL) override;

	// CefRequestHandler methods
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool isRedirect) override;
	virtual CefRequestHandler::ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) override;

	// CefLifeSpawnHandler methods
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access) override;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

	// CefJSDialogHandler methods
	virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, CefJSDialogHandler::JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message) override;

	// CefDialogHandler methods
	virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_name, const std::vector< CefString >& accept_types, int selected_accept_filter, CefRefPtr< CefFileDialogCallback > callback) override;

	// CefDisplayHandler methods
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
	virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) override;
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;

	// CefContextMenuHandler methods
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

	void UpdateTexture();
	void Render();

	CefRefPtr<CefBrowser> m_pWebView;

private:

	ID3D11Texture2D*	m_pTexture = nullptr;
	ID3D11ShaderResourceView* m_pTextureView = nullptr;
	bool                m_bBeingDestroyed;
	bool                m_bIsLocal;
	bool                m_bIsTransparent;
	std::string			m_sURL;
	POINT               m_vecMousePosition;
	bool                m_mouseButtonStates[3];
	float               m_fVolume;
	bool                m_bHasInputFocus;
	std::set<std::string> m_AjaxHandlers;

	struct
	{
		bool changed = false;
		std::mutex dataMutex;
		std::mutex cvMutex;
		std::condition_variable cv;

		const void* buffer;
		int width, height;
		CefRenderHandler::RectList dirtyRects;

		CefRect popupRect;
		bool popupShown = false;
		std::unique_ptr<byte[]> popupBuffer;
	} m_RenderData;

public:
	// Implement smartpointer methods (all Cef-classes require that since they are derived from CefBase)
	IMPLEMENT_REFCOUNTING(CEFView);
};