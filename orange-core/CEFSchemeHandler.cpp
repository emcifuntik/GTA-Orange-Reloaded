#include "stdafx.h"
#include <fstream>

class CEFResourceHandler : public CefResourceHandler
{
private:
	std::string file, mime;
	std::ifstream ifile;
	bool closed = false;

public:
	CEFResourceHandler()
	{
	}

	virtual ~CEFResourceHandler()
	{
	}

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
	{
		std::string url = request->GetURL();
		std::wstring scheme, path;

		//log << __FUNCTION__ << ": " << request->GetURL().ToString() << std::endl;

		CefURLParts parts;
		CefParseURL(url, parts);

		scheme = CefString(&parts.scheme);
		path = CefString(&parts.path);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		//log << converter.to_bytes(scheme) << " " << converter.to_bytes(path) << std::endl;

		if (scheme == L"http")
		{
			file = CGlobals::Get().orangePath + "/" + converter.to_bytes(path).substr(1);
			//log << "file: " << file << std::endl;
		}
		else
		{
		}

		auto hash = file.find_first_of(L'#');

		if (hash != std::string::npos)
		{
			file.erase(hash);
		}

		hash = file.find_first_of(L'?');

		if (hash != std::string::npos)
		{
			file.erase(hash);
		}

		auto handleOpen = [=]()
		{
			ifile.open(file, std::ios::binary | std::ios::ate);
			std::string ext = file.substr(file.rfind('.') + 1);

			mime = CefGetMimeType(ext);

			callback->Continue();
		};

		handleOpen();
		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl)
	{
		response->SetMimeType(mime);

		if (ifile.bad())
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(200);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		map.insert(std::make_pair("cache-control", "no-cache, must-revalidate"));
		response->SetHeaderMap(map);

		if (ifile.good())
		{
			//log << "OK" << std::endl;
			response_length = ifile.tellg();
			log << "length: " << response_length << std::endl;
			ifile.seekg(0, std::ios::beg);
		}
		else
		{
			response_length = 0;
		}
	}

	virtual void Cancel()
	{
		closed = true;
		ifile.close();
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
	{
		if (ifile.good())
		{
			log << bytes_to_read << std::endl;
			ifile.read((char*)data_out, bytes_to_read);
			bytes_read = bytes_to_read;

			//log << (char*)data_out << std::endl;

			callback->Continue();
			return true;
		}

		callback->Continue();
		return false;
	}

	IMPLEMENT_REFCOUNTING(CEFResourceHandler);
};

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
		//log << "OK" << std::endl;
		return new CEFResourceHandler();
	}

	return nullptr;
}