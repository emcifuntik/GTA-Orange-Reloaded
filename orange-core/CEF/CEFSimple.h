#include <include/cef_app.h>
#include <include/cef_scheme.h>

class CEFSimple : public CefApp {
	// Error Handler
	//static CefRefPtr<CefResourceHandler> HandleError(const SString& strError, unsigned int uiError);

	virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	IMPLEMENT_REFCOUNTING(CEFSimple);
};
