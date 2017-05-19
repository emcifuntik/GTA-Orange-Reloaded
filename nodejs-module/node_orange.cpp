#include "stdafx.h"

namespace node
{
	namespace orange
	{
		void * OnEventCallback(uv_callback_t *handle, void *value)
		{
			OnEventCallbackStruct* callback = (OnEventCallbackStruct*)value;

			Isolate* isolate = Isolate::GetCurrent();
			HandleScope scope(isolate);
			Isolate::Scope isolate_scope(isolate);
			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_EVENT);
			v8::Local<v8::Context> context = Context::New(isolate);
			Context::Scope context_scope(context);

			Local<Array> argsArray = Array::New(isolate, callback->args->size());
			for (int i = 0; i < callback->args->size(); i++)
			{
				MValue value = callback->args->at(i);
				switch (value.type)
				{
				case M_STRING:
					argsArray->Set(i, String::NewFromUtf8(isolate, value.getString()));
					break;
				case M_INT:
					argsArray->Set(i, Integer::New(isolate, value.getInt()));
					break;
				case M_BOOL:
					argsArray->Set(i, Boolean::New(isolate, value.getBool()));
					break;
				case M_ULONG:
				case M_DOUBLE:
					argsArray->Set(i, Number::New(isolate, value.getDouble()));
					break;
				case M_ARRAY:
					//TODO:
					break;
				}
			}
			const unsigned argc = 2;
			Local<Value> argv[argc] = { String::NewFromUtf8(isolate, callback->event), argsArray };
			Local<Function> callbackFunc = Local<Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(Null(isolate), argc, argv);
			free(value);
			return NULL;
		}

		void * OnResourceLoadCallback(uv_callback_t *handle, void *value)
		{
			char* str = (char*)value;
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope scope(isolate);

			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_RESOURCE_LOAD);

			const unsigned argc = 1;
			Local<Value> argv[argc] = { String::NewFromUtf8(isolate, str) };
			Local<Function> callbackFunc = Local<Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(Null(isolate), argc, argv);
			free(value);
			return NULL;
		}

		static void OnEventFunction(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope scope(isolate);
			Persistent<Function>* callback = new Persistent<Function>();
			callback->Reset(isolate, args[0].As<Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnEventCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_EVENT, callbackInfo);
		}

		static void OnResourceLoadFunction(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope scope(isolate);
			Persistent<Function>* callback = new Persistent<Function>();
			callback->Reset(isolate, args[0].As<Function>());

			uv_callback_t *uv_callback = (uv_callback_t*)malloc(sizeof(uv_callback_t));

			uv_callback_init(uv_default_loop(), uv_callback, OnResourceLoadCallback, UV_DEFAULT);

			CallbackInfo* callbackInfo = new CallbackInfo();
			callbackInfo->callback = uv_callback;
			callbackInfo->function = callback;

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_RESOURCE_LOAD, callbackInfo);
		}

		static void Print(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = args.GetIsolate();
			HandleScope scope(isolate);
			if (args.Length() < 1) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			if (!args[0]->IsString()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "First argument is not string")));
				return;
			}
			String::Utf8Value message(args[0]->ToString());
			API::Get().Print(*message);
		}

		static void CreateVehicle(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = args.GetIsolate();
			HandleScope scope(isolate);
			if (args.Length() < 5) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			/*if (!args[0]->IsNumber()) {
			isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "First argument is not string")));
			return;
			}*/
			//API::Get().CreateVehicle(523724515, 0.0f, 0.0f, 0.0f, 0.0f);
		}

		static void SetPlayerPosition(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = args.GetIsolate();
			HandleScope scope(isolate);

			if (args.Length() < 4) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			if (!args[0]->IsNumber()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "PlayerID is not number")));
				return;
			}
			if (!args[1]->IsNumber()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "X position is not number")));
				return;
			}
			if (!args[2]->IsNumber()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Y position is not number")));
				return;
			}
			if (!args[3]->IsNumber()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Z position is not number")));
				return;
			}
			long playerId = (long)args[0]->NumberValue();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			API::Get().SetPlayerPosition(playerId, x, y, z);
		}

		static void GetPlayerPosition(const FunctionCallbackInfo<Value>& args)
		{
			Isolate* isolate = args.GetIsolate();
			HandleScope scope(isolate);

			if (args.Length() < 1) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}

			if (!args[0]->IsNumber()) {
				isolate->ThrowException(Exception::TypeError(
					String::NewFromUtf8(isolate, "PlayerID is not number")));
				return;
			}

			long playerId = (long)args[0]->NumberValue();

			CVector3 position = API::Get().GetPlayerPosition(playerId);

			Local<Array> positionArray = Array::New(isolate, 3);
			positionArray->Set(0, Number::New(isolate, position.fX));
			positionArray->Set(1, Number::New(isolate, position.fY));
			positionArray->Set(2, Number::New(isolate, position.fZ));

			args.GetReturnValue().Set(positionArray);
		}

		void Initialize(Local<Object> target, Local<Value> unused, Local<Context> context)
		{
			Environment* env = Environment::GetCurrent(context);

			env->SetMethod(target, "onResourceLoad", OnResourceLoadFunction);
			env->SetMethod(target, "onEvent", OnEventFunction);
			env->SetMethod(target, "print", Print);
			env->SetMethod(target, "createVehicle", CreateVehicle);
			env->SetMethod(target, "setPlayerPosition", SetPlayerPosition);
		}
	}
}

NODE_MODULE_CONTEXT_AWARE_BUILTIN(orange, node::orange::Initialize);