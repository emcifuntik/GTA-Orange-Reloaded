#include "stdafx.h"

CEFCore::CEFCore()
{

}

void CEFCore::init()
{
	CefMainArgs mainArgs;
	void* sandboxInfo = nullptr;
	CefRefPtr<CEFSimple> app(new CEFSimple);
	log << "TEST1" << std::endl;
	CefSettings settings;

	CefString(&settings.browser_subprocess_path).FromString((CGlobals::Get().orangePath + "/cef/CEF-Launcher.exe").c_str());
	CefString(&settings.resources_dir_path).FromString((CGlobals::Get().orangePath + "/cef/").c_str());
	CefString(&settings.locales_dir_path).FromString((CGlobals::Get().orangePath + "/cef/locales").c_str());
	CefString(&settings.log_file).FromString((CGlobals::Get().orangePath + "/cef/cefdebug.log").c_str());

	settings.log_severity = cef_log_severity_t::LOGSEVERITY_VERBOSE;

	settings.multi_threaded_message_loop = true;
	settings.windowless_rendering_enabled = true;

	bool test = CefInitialize(mainArgs, settings, app, sandboxInfo);
	log << "TEST2" << test << std::endl;
	return;
}

void CEFCore::CreateWebView(unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent)
{
	// Create our webview implementation
	CefRefPtr<CEFView> pWebView = new CEFView(bIsLocal, bTransparent);
	pWebView->Initialise();
	return;
}
