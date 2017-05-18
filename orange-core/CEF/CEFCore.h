#pragma once

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_sandbox_win.h>
#include <include/cef_parser.h>
#include "stdafx.h"

class CEFCore
{
	static CEFCore* instance;
public:
	std::vector<CefRefPtr<CEFView>> views;
	CefRefPtr<CEFSimple> app;

	std::map<std::string, std::function<void(CefRefPtr<CefListValue> args)>> m_handlers;

	CEFCore();

	std::function<void(CefRefPtr<CefListValue> args)> GetHandler(std::string name)
	{
		return m_handlers[name];
	}
	void RegisterJSFunc(std::string name, std::function<void(CefRefPtr<CefListValue> args)> f);

	static CEFCore* Get();
	void init();
	CefRefPtr<CEFView> CreateWebView(std::string url, unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent);
};