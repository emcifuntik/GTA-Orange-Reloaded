#pragma once

class CEFApp : public CefApp, public CefRenderProcessHandler, public CefV8Handler {
	// Error Handler
	//static CefRefPtr<CefResourceHandler> HandleError(const SString& strError, unsigned int uiError);

	CefRefPtr<CefFrame> m_pFrame;

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override
	{
		m_pFrame = frame;

		context->GetGlobal()->SetValue("TriggerEvent", CefV8Value::CreateFunction("TriggerEvent", this), V8_PROPERTY_ATTRIBUTE_NONE);
	};

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
	{
		return this;
	};

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override
	{
		if (arguments.size() == 0)
			return true;

		CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("TriggerEvent");

		auto args = message->GetArgumentList();
		args->SetSize(arguments.size());

		for (int i = 0; i < arguments.size(); i++)
		{
			auto arg = arguments[i];

			if (arg->IsBool()) args->SetBool(i, arg->GetBoolValue());
			else if (arg->IsDouble()) args->SetDouble(i, arg->GetDoubleValue());
			else if (arg->IsInt() || arg->IsUInt()) args->SetInt(i, arg->GetIntValue());
			else if (arg->IsNull()) args->SetNull(i);
			else if (arg->IsString()) args->SetString(i, arg->GetStringValue());
			else args->SetString(i, "Unsupported variable type");
		}

		m_pFrame->GetBrowser()->SendProcessMessage(PID_BROWSER, message);
		return true;
	};

	IMPLEMENT_REFCOUNTING(CEFApp);
};