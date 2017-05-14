// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include <include/cef_app.h>
#include <include/cef_scheme.h>

class CEFSimple : public CefApp, public CefSchemeHandlerFactory {
	// Error Handler
	//static CefRefPtr<CefResourceHandler> HandleError(const SString& strError, unsigned int uiError);

	virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	// CefSchemeHandlerFactory methods
	virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request) override;

	IMPLEMENT_REFCOUNTING(CEFSimple);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
