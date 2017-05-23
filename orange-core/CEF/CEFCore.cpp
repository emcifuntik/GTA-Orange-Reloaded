#include "stdafx.h"

CEFCore* CEFCore::instance = nullptr;

CEFCore::CEFCore()
{

}

CEFCore::~CEFCore()
{
	//for (auto view : views)
	///	delete view;

	//views.erase(views.begin(), views.end());
	CefClearSchemeHandlerFactories();
	CefShutdown();
}

void CEFCore::RegisterJSFunc(std::string name, std::function<void(CefRefPtr<CefFrame> frame, CefRefPtr<CefListValue> args)> f)
{
	m_handlers[name] = f;
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
	app = new CEFSimple;
	CefSettings settings;

	CefString(&settings.browser_subprocess_path).FromString((CGlobals::Get().orangePath + "/cef/CEF-Launcher.exe").c_str());
	CefString(&settings.resources_dir_path).FromString((CGlobals::Get().orangePath + "/cef/").c_str());
	CefString(&settings.locales_dir_path).FromString((CGlobals::Get().orangePath + "/cef/locales").c_str());
	CefString(&settings.log_file).FromString((CGlobals::Get().orangePath + "/cef/cefdebug.log").c_str());

	settings.log_severity = cef_log_severity_t::LOGSEVERITY_VERBOSE;

	settings.multi_threaded_message_loop = true;
	settings.windowless_rendering_enabled = true;

	CefInitialize(mainArgs, settings, app, sandboxInfo);
	CefRegisterSchemeHandlerFactory("http", "orange", new CEFSchemeHandlerFactory);

	RegisterJSFunc("orangeConnect", [](CefRefPtr<CefFrame> frame, CefRefPtr<CefListValue> args) {
		if (args->GetString(1).length() > 30)
		{
			CChat::Get()->AddChatMessage("Can't connect to the server", { 255, 0, 0, 255 });
		}

		log << "Connecting to " << args->GetString(1).ToString() << ":" << args->GetDouble(2) << std::endl;

		strcpy_s(CGlobals::Get().serverIP, 32, args->GetString(1).ToString().c_str());
		CGlobals::Get().serverPort = args->GetDouble(2) != 0 ? args->GetDouble(2) : 7788;
		CGlobals::Get().name = args->GetString(3);

		std::stringstream ss;
		ss << "Connecting to beta-test server"; //<< CGlobals::Get().serverIP << ":" << CGlobals::Get().serverPort;
		CChat::Get()->AddChatMessage(ss.str());

		if (!CNetworkConnection::Get()->Connect(CGlobals::Get().serverIP, CGlobals::Get().serverPort))
			CChat::Get()->AddChatMessage("Can't connect to the server", {
				255, 0, 0, 255 });
		

		CConfig::Get()->sIP = CGlobals::Get().serverIP;
		CConfig::Get()->uiPort = CGlobals::Get().serverPort;
		CConfig::Get()->sNickName = CGlobals::Get().name;
		CConfig::Get()->Save();

		CGlobals::Get().showChat = true;
		return CefV8Value::CreateNull();
	});

	RegisterJSFunc("loadCfg", [](CefRefPtr<CefFrame> frame, CefRefPtr<CefListValue> args) {
		std::stringstream ss;

		ss << "loadCfg(['" << CConfig::Get()->sNickName <<
			"', '" << CConfig::Get()->sIP <<
			"', " << CConfig::Get()->uiPort << "]);";

		frame->ExecuteJavaScript(ss.str(), frame->GetURL(), 0);
	});

	RegisterJSFunc("saveName", [](CefRefPtr<CefFrame> frame, CefRefPtr<CefListValue> args) {
		CConfig::Get()->sNickName = args->GetString(1);
		CConfig::Get()->Save();
	});

	RegisterJSFunc("quit", [](CefRefPtr<CefFrame> frame, CefRefPtr<CefListValue> args) {
		TerminateProcess(GetCurrentProcess(), 0);
	});
}

CefRefPtr<CEFView> CEFCore::CreateWebView(std::string url, unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent)
{
	CefRefPtr<CEFView> pWebView = new CEFView(url, bIsLocal, bTransparent);
	pWebView->Initialise();
	views.push_back(pWebView);

	return pWebView;
}
