#include "stdafx.h"

bool CEFV8Handler::Execute(const CefString& name,
	CefRefPtr<CefV8Value> object,
	const CefV8ValueList& arguments,
	CefRefPtr<CefV8Value>& retval,
	CefString& exception) {

 if (name == "orangeConnect") {
	 if (arguments[0]->GetStringValue().length() > 30)
	 {
		 CChat::Get()->AddChatMessage("Can't connect to the server", { 255, 0, 0, 255 });
		 return true;
	 }

	 strcpy_s(CGlobals::Get().serverIP, 32, arguments[0]->GetStringValue().ToString().c_str());
	 CGlobals::Get().serverPort = arguments[1]->GetIntValue();
	 CGlobals::Get().name = arguments[2]->GetStringValue();

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

	 //retval = CefV8Value::CreateString("My Value!");
	 return true;
 }

 return false;
	}