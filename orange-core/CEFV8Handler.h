#pragma once

class CEFV8Handler : public CefV8Handler {
public:
	CEFV8Handler() {}

	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE;

	// Provide the reference counting implementation for this class.
	IMPLEMENT_REFCOUNTING(CEFV8Handler);
};