#include <include/cef_app.h>
#include <include/cef_scheme.h>

class CEFSimple : public CefApp, public CefRenderProcessHandler, public CefV8Handler {
	// Error Handler
	static CefRefPtr<CefResourceHandler> HandleError(std::string strError, unsigned int uiError);

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;

	virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	IMPLEMENT_REFCOUNTING(CEFSimple);
};
