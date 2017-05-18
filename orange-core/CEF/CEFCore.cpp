#include "stdafx.h"

CEFCore* CEFCore::instance = nullptr;

CEFCore::CEFCore()
{

}

void CEFCore::RegisterJSFunc(std::string name, std::function<void(CefRefPtr<CefListValue> args)> f)
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
	
	//settings.single_process = true;

	CefInitialize(mainArgs, settings, app, sandboxInfo);
	CefRegisterSchemeHandlerFactory("http", "orange", new CEFSchemeHandlerFactory);

	RegisterJSFunc("orangeConnect", [](CefRefPtr<CefListValue> args) {
		log << "Connecting to " << args->GetString(1).ToString() << ":" << args->GetInt(2) << std::endl;
		if (args->GetString(1).length() > 30)
		{
			CChat::Get()->AddChatMessage("Can't connect to the server", { 255, 0, 0, 255 });
		}

		strcpy_s(CGlobals::Get().serverIP, 32, args->GetString(1).ToString().c_str());
		CGlobals::Get().serverPort = args->GetInt(2);
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
}

CefRefPtr<CEFView> CEFCore::CreateWebView(std::string url, unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent)
{
	CefRefPtr<CEFView> pWebView = new CEFView(url, bIsLocal, bTransparent);
	pWebView->Initialise();
	views.push_back(pWebView);
	return pWebView;
}
