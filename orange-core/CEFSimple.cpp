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
	// Register custom MTA scheme (has to be called in all proceseses)
	registrar->AddCustomScheme("mtalocal", false, false, false, false, false, true);
}

void CEFSimple::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-gpu-compositing");
	command_line->AppendSwitch("disable-gpu");
	//command_line->AppendSwitch("disable-d3d11");
	command_line->AppendSwitch("enable-begin-frame-scheduling");
}

CefRefPtr<CefResourceHandler> CEFSimple::Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request)
{
	log << "TEST" << std::endl;
	// browser or frame are NULL if the request does not orginate from a browser window
	// This is for exmaple true for the application cache or CEFURLRequests
	// (http://www.html5rocks.com/en/tutorials/appcache/beginner/)
	if (!browser || !frame)
		return nullptr;
	
	/*CWebCore* pWebCore = static_cast<CWebCore*> (g_pCore->GetWebCore());
	auto pWebView = pWebCore->FindWebView(browser);
	if (!pWebView || !pWebView->IsLocal())
		return nullptr;

	CefURLParts urlParts;
	if (!CefParseURL(request->GetURL(), urlParts))
		return nullptr;

	if (scheme_name == "mtalocal") // Backward compatibility
	{
		// Get full path
		SString path = UTF16ToMbUTF8(urlParts.path.str).substr(2);

		// Check if we're dealing with an external resource
		if (path[0] == ':')
		{
			size_t end = path.find_first_of('/');
			if (end != std::string::npos)
			{
				SString resourceName = path.substr(1, end - 1);
				SString resourcePath = path.substr(end);

				// Call this function recursively and use the mta scheme instead
				request->SetURL("http://mta/local/" + resourceName + resourcePath);
				return Create(browser, frame, "http", request);
			}
			return HandleError("404 - Not found", 404);
		}

		// Redirect mtalocal://* to http://mta/local/*, call recursively
		request->SetURL("http://mta/local/" + path);
		return Create(browser, frame, "http", request);
	}

	SString host = UTF16ToMbUTF8(urlParts.host.str);
	if (scheme_name == "http" && host == "mta")
	{
		// Scheme format: http://mta/resourceName/file.html or http://mta/local/file.html for the current resource

		// Get resource name and path
		SString path = UTF16ToMbUTF8(urlParts.path.str).substr(1); // Remove slash at the front
		size_t slashPos = path.find('/');
		if (slashPos == std::string::npos)
			return HandleError("404 - Not found", 404);

		SString resourceName = path.substr(0, slashPos);
		SString resourcePath = path.substr(slashPos + 1);

		if (resourcePath.empty())
			return HandleError("404 - Not found", 404);

		// Get mime type from extension
		CefString mimeType;
		size_t pos = resourcePath.find_last_of('.');
		if (pos != std::string::npos)
			mimeType = CefGetMimeType(resourcePath.substr(pos + 1));

		// Make sure we provide a mime type, even 
		// when we cannot deduct it from the file extension
		if (mimeType.empty())
			mimeType = "application/octet-stream";

		if (pWebView->HasAjaxHandler(resourcePath))
		{
			std::vector<SString> vecGet;
			std::vector<SString> vecPost;

			if (urlParts.query.str != nullptr)
			{
				SString strGet = UTF16ToMbUTF8(urlParts.query.str);
				std::vector<SString> vecTmp;
				strGet.Split("&", vecTmp);

				SString key; SString value;
				for (auto&& param : vecTmp)
				{
					param.Split("=", &key, &value);
					vecGet.push_back(key);
					vecGet.push_back(value);
				}
			}

			CefPostData::ElementVector vecPostElements;
			auto postData = request->GetPostData();
			if (postData.get())
			{
				request->GetPostData()->GetElements(vecPostElements);

				SString key; SString value;
				for (auto&& post : vecPostElements)
				{
					// Limit to 5MiB and allow byte data only
					size_t bytesCount = post->GetBytesCount();
					if (bytesCount > 5 * 1024 * 1024 || post->GetType() != CefPostDataElement::Type::PDE_TYPE_BYTES)
						continue;

					// Make string from buffer
					std::unique_ptr<char[]> buffer{ new char[bytesCount] };
					post->GetBytes(bytesCount, buffer.get());
					SStringX param(buffer.get(), bytesCount);

					// Parse POST data into vector
					std::vector<SString> vecTmp;
					param.Split("&", vecTmp);

					for (auto&& param : vecTmp)
					{
						param.Split("=", &key, &value);
						vecPost.push_back(key);
						vecPost.push_back(value);
					}
				}
			}

			auto handler = new CAjaxResourceHandler(vecGet, vecPost, mimeType);
			pWebView->HandleAjaxRequest(resourcePath, handler);
			return handler;
		}
		else
		{
			// Calculate MTA resource path
			if (resourceName != "local")
				path = ":" + resourceName + "/" + resourcePath;
			else
				path = resourcePath;

			// Calculate absolute path
			if (!pWebView->GetFullPathFromLocal(path))
				return HandleError("404 - Not found", 404);

			// Verify local files
			if (!pWebView->VerifyFile(path))
				return HandleError("403 - Access Denied", 403);

			// Finally, load the file stream
			auto stream = CefStreamReader::CreateForFile(path);
			if (stream.get())
				return new CefStreamResourceHandler(mimeType, stream);
			return HandleError("404 - Not found", 404);
		}
	}
	*/
	// Return null if there is no matching scheme
	// This falls back to letting CEF handle the request
	return nullptr;
}