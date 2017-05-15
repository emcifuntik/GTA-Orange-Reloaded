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


void CEFSimple::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	registrar->AddCustomScheme("ui", false, false, false, false, false, true);
}

void CEFSimple::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-gpu-compositing");
	command_line->AppendSwitch("disable-gpu");
	//command_line->AppendSwitch("disable-d3d11");
	command_line->AppendSwitch("enable-begin-frame-scheduling");
}