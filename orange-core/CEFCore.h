#pragma once

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_sandbox_win.h>
#include <include/cef_parser.h>
#include "stdafx.h"

class CWebBrowserItem;

class CEFCore
{
	public:
		CEFCore();
		void init();
		void CreateWebView(unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, bool bTransparent);
};