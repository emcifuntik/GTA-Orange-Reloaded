#include "stdafx.h"

namespace node
{
	namespace orange
	{
		void * OnEventCallback(uv_callback_t *handle, void *value)
		{
			OnEventCallbackStruct* callback = (OnEventCallbackStruct*)value;

			v8::Isolate* isolate = v8::Isolate::GetCurrent();
			v8::HandleScope scope(isolate);
			v8::Isolate::Scope isolate_scope(isolate);
			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_EVENT);
			v8::Local<v8::Context> context = NodeModule::GetModule()->GetContext();
			v8::Context::Scope context_scope(context);

			v8::Local<v8::Array> argsArray = v8::Array::New(isolate, (int)callback->args->size());
			for (int i = 0; i < callback->args->size(); i++)
			{
				MValue value = callback->args->at(i);
				switch (value.type)
				{
				case M_STRING:
					argsArray->Set(i, v8::String::NewFromUtf8(isolate, value.getString()));
					break;
				case M_INT:
					argsArray->Set(i, v8::Integer::New(isolate, value.getInt()));
					break;
				case M_BOOL:
					argsArray->Set(i, v8::Boolean::New(isolate, value.getBool()));
					break;
				case M_ULONG:
					argsArray->Set(i, v8::Uint32::New(isolate, value.getULong()));
					break;
				case M_DOUBLE:
					argsArray->Set(i, v8::Number::New(isolate, value.getDouble()));
					break;
				case M_ARRAY:
					//TODO:
					break;
				}
			}
			v8::Local<v8::Value> argv[] = { v8::String::NewFromUtf8(isolate, callback->event), argsArray };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), 2, argv);
			free(callback->args);
			free(callback->event);
			free(value);
			return NULL;
		}

		void * OnResourceLoadCallback(uv_callback_t *handle, void *value)
		{
			char* str = (char*)value;
			v8::Isolate* isolate = v8::Isolate::GetCurrent();
			v8::HandleScope scope(isolate);
			v8::Local<v8::Context> context = NodeModule::GetModule()->GetContext();
			v8::Context::Scope context_scope(context);

			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_RESOURCE_LOAD);

			v8::Local<v8::Value> argv[] = { v8::String::NewFromUtf8(isolate, str) };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), 1, argv);
			free(value);
			return NULL;
		}

		void * OnPlayerCommandCallback(uv_callback_t *handle, void *value)
		{
			v8::Isolate* isolate = v8::Isolate::GetCurrent();
			v8::HandleScope scope(isolate);
			v8::Local<v8::Context> context = NodeModule::GetModule()->GetContext();
			v8::Context::Scope context_scope(context);

			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_PLAYER_COMMAND);

			OnPlayerCommandCallbackStruct* callback = (OnPlayerCommandCallbackStruct*)value;

			v8::Local<v8::Value> argv[] = { v8::Number::New(isolate, *callback->playerid), v8::String::NewFromUtf8(isolate, callback->command) };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), 2, argv);
			//free(callback->command);
			free(callback->playerid);
			free(value);
			return NULL;
		}

		void * OnServerCommandCallback(uv_callback_t *handle, void *value)
		{
			v8::Isolate* isolate = v8::Isolate::GetCurrent();
			v8::HandleScope scope(isolate);
			v8::Local<v8::Context> context = NodeModule::GetModule()->GetContext();
			v8::Context::Scope context_scope(context);

			char* command = (char*)value;

			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_SERVER_COMMAND);

			v8::Local<v8::Value> argv[] = { v8::String::NewFromUtf8(isolate, command) };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), 1, argv);

			free(value);
			return NULL;
		}

		void OnEventFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			v8::Persistent<v8::Function>* callback = new v8::Persistent<v8::Function>();
			callback->Reset(isolate, args[0].As<v8::Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnEventCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_EVENT, callbackInfo);
		}

		void OnResourceLoadFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			v8::Persistent<v8::Function>* callback = new v8::Persistent<v8::Function>();
			callback->Reset(isolate, args[0].As<v8::Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnResourceLoadCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_RESOURCE_LOAD, callbackInfo);
		}

		void OnPlayerCommandFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			v8::Persistent<v8::Function>* callback = new v8::Persistent<v8::Function>();
			callback->Reset(isolate, args[0].As<v8::Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnPlayerCommandCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_PLAYER_COMMAND, callbackInfo);
		}

		void OnServerCommandFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			v8::Persistent<v8::Function>* callback = new v8::Persistent<v8::Function>();
			callback->Reset(isolate, args[0].As<v8::Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnServerCommandCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_SERVER_COMMAND, callbackInfo);
		}
	}
}