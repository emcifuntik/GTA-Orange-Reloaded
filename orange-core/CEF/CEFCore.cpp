#include "stdafx.h"

CEFCore* CEFCore::instance = nullptr;

CEFCore::CEFCore()
{

}

CEFCore * CEFCore::Get()
{
	if (!instance) instance = new CEFCore;
	return instance;
}

void CEFCore::init()
{
	CefMainArgs mainArgs;
	void* sandboxInfo = nullptr;
	CefRefPtr<CEFSimple> app(new CEFSimple);
	CefSettings settings;

	CefString(&settings.browser_subprocess_path).FromString((CGlobals::Get().orangePath + "/cef/CEF-Launcher.exe").c_str());
	CefString(&settings.resources_dir_path).FromString((CGlobals::Get().orangePath + "/cef/").c_str());
	CefString(&settings.locales_dir_path).FromString((CGlobals::Get().orangePath + "/cef/locales").c_str());
	CefString(&settings.log_file).FromString((CGlobals::Get().orangePath + "/cef/cefdebug.log").c_str());

	settings.log_severity = cef_log_severity_t::LOGSEVERITY_VERBOSE;

	settings.multi_threaded_message_loop = true;
	settings.windowless_rendering_enabled = true;

	CefInitialize(mainArgs, settings, app, sandboxInfo);
	CefRegisterSchemeHandlerFactory("ui", "", new CEFSchemeHandlerFactory);
}

CefRefPtr<CEFView> CEFCore::CreateWebView(std::string url, unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent)
{
	CefRefPtr<CEFView> pWebView = new CEFView(url, bIsLocal, bTransparent);
	pWebView->Initialise();
	views.push_back(pWebView);
	return pWebView;
}
