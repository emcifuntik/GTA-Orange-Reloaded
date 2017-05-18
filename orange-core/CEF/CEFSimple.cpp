// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"

#include <string>
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_stream_resource_handler.h>
#include <include/cef_parser.h>

/*CefRefPtr<CefResourceHandler> CEFSimple::HandleError(const SString& strError, unsigned int uiError)
{
	auto stream = CefStreamReader::CreateForData((void*)strError.c_str(), strError.length());
	return new CefStreamResourceHandler(
		uiError, strError, "text/plain", CefResponse::HeaderMap(), stream);
}*/

void CEFSimple::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	//std::cout << __FUNCTION__ << std::endl;
	CefRefPtr<CefV8Value> global = context->GetGlobal();
	global->SetValue("orangeConnect", CefV8Value::CreateFunction("orangeConnect", this), V8_PROPERTY_ATTRIBUTE_READONLY);
}

CefRefPtr<CefRenderProcessHandler> CEFSimple::GetRenderProcessHandler()
{
	//std::cout << __FUNCTION__ << std::endl;
	return this;
}

bool CEFSimple::Execute(const CefString & name, CefRefPtr<CefV8Value> object, const CefV8ValueList & arguments, CefRefPtr<CefV8Value>& retval, CefString & exception)
{
	log << name.ToString() << " called from js" << std::endl;

	if (name == "orangeConnect") {
		log << "Connecting to " << arguments[0]->GetStringValue().ToString() << ":" << arguments[1]->GetIntValue() << std::endl;
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

void CEFSimple::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{

}

void CEFSimple::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-gpu-compositing");
	command_line->AppendSwitch("disable-gpu");
	//command_line->AppendSwitch("disable-d3d11");
	command_line->AppendSwitch("enable-begin-frame-scheduling");
}