#include "stdafx.h"
#include <fstream>
#include "include\wrapper\cef_stream_resource_handler.h"

CefRefPtr<CefResourceHandler> CEFSchemeHandlerFactory::Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request)
{
	CefURLParts urlParts;
	if (!CefParseURL(request->GetURL(), urlParts))
	{
		log << "Failed to parse" << std::endl;
		return nullptr;
	}

	//log << __FUNCTION__ << ": " << request->GetURL().ToString() << ", " << scheme_name.ToString() << ", " << std::endl;

	if (scheme_name == "http" && std::wstring(urlParts.host.str) == L"orange")
	{
		//log << "OK" << std::endl

		std::string url = request->GetURL();
		CefString mimeType;
		std::wstring scheme, path;

		//log << __FUNCTION__ << ": " << request->GetURL().ToString() << std::endl;

		CefURLParts parts;
		CefParseURL(url, parts);

		scheme = CefString(&parts.scheme);
		path = CefString(&parts.path);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		//log << converter.to_bytes(scheme) << " " << converter.to_bytes(path) << std::endl;

		size_t slashPos = path.find('/');

		//if (slashPos == std::string::npos)
		//	return HandleError("404 - Not found", 404);

		CefString resourceName = path.substr(0, slashPos);
		CefString resourcePath = path.substr(slashPos + 1);

		std::string file;
		
		if(resourceName == "ui")
			file = CGlobals::Get().orangePath + "/" + converter.to_bytes(path).substr(1);
		else
			file = CGlobals::Get().orangePath + "/" + converter.to_bytes(path).substr(1);

		auto hash = file.find_first_of(L'#');
		if (hash != std::string::npos)
			file.erase(hash);

		hash = file.find_first_of(L'?');
		if (hash != std::string::npos)
			file.erase(hash);

		std::string ext = file.substr(file.rfind('.') + 1);

		mimeType = CefGetMimeType(ext);

		auto stream = CefStreamReader::CreateForFile(file);
		if (stream.get())
			return new CefStreamResourceHandler(mimeType, stream);
	}

	return nullptr;
}