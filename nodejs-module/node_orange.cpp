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

			v8::Local<v8::Array> argsArray = v8::Array::New(isolate, callback->args->size());
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
			const unsigned argc = 2;
			v8::Local<v8::Value> argv[argc] = { v8::String::NewFromUtf8(isolate, callback->event), argsArray };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), argc, argv);
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

			const unsigned argc = 1;
			v8::Local<v8::Value> argv[argc] = { v8::String::NewFromUtf8(isolate, str) };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), argc, argv);
			free(value);
			return NULL;
		}

		void * OnPlayerCommandCallback(uv_callback_t *handle, void *value)
		{
			v8::Isolate* isolate = v8::Isolate::GetCurrent();
			v8::HandleScope scope(isolate);
			v8::Local<v8::Context> context = NodeModule::GetModule()->GetContext();
			v8::Context::Scope context_scope(context);

			CallbackInfo* callbackInfo = NodeModule::GetModule()->GetCallback(CALLBACK_ON_PLAYER_COMMAND_EVENT);

			OnPlayerCommandCallbackStruct* callback = (OnPlayerCommandCallbackStruct*)value;

			const unsigned argc = 2;
			v8::Local<v8::Value> argv[argc] = { v8::Number::New(isolate, *callback->playerid), v8::String::NewFromUtf8(isolate, callback->command) };
			v8::Local<v8::Function> callbackFunc = v8::Local<v8::Function>::New(isolate, *callbackInfo->function);
			callbackFunc->Call(context->Global(), argc, argv);
			//free(callback->command);
			free(callback->playerid);
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

			NodeModule::GetModule()->SetCallback(CALLBACK_ON_PLAYER_COMMAND_EVENT, callbackInfo);
		}

		
		void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			if (args.Length() < 1)
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			if (!args[0]->IsString())
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "First argument is not string")));
				return;
			}
			v8::String::Utf8Value message(args[0]->ToString());
			API::Get().Print(*message);
		}

		void Initialize(v8::Local<v8::Object> target, v8::Local<v8::Value> unused, v8::Local<v8::Context> context)
		{
			Environment* env = Environment::GetCurrent(context);

			env->SetMethod(target, "onResourceLoad", OnResourceLoadFunction);
			env->SetMethod(target, "onEvent", OnEventFunction);
			env->SetMethod(target, "onPlayerCommand", OnPlayerCommandFunction);

			env->SetMethod(target, "print", Print);
			//env->SetMethod(target, "loadClientScript", LoadClientScript);
			env->SetMethod(target, "triggerClientEvent", TriggerClientEvent);
			env->SetMethod(target, "kickPlayer", KickPlayer);
			env->SetMethod(target, "setPlayerPosition", SetPlayerPosition);
			env->SetMethod(target, "getPlayerPosition", GetPlayerPosition);
			env->SetMethod(target, "isPlayerInRange", IsPlayerInRange);
			env->SetMethod(target, "setPlayerHeading", SetPlayerHeading);
			env->SetMethod(target, "getPlayerHeading", GetPlayerHeading);
			env->SetMethod(target, "removePlayerWeapons", RemovePlayerWeapons);
			env->SetMethod(target, "givePlayerWeapon", GivePlayerWeapon);
			env->SetMethod(target, "givePlayerAmmo", GivePlayerAmmo);
			env->SetMethod(target, "givePlayerMoney", GivePlayerMoney);
			env->SetMethod(target, "setPlayerMoney", SetPlayerMoney);
			env->SetMethod(target, "resetPlayerMoney", ResetPlayerMoney);
			env->SetMethod(target, "getPlayerMoney", GetPlayerMoney);
			env->SetMethod(target, "setPlayerModel", SetPlayerModel);
			env->SetMethod(target, "getPlayerModel", GetPlayerModel);
			env->SetMethod(target, "setPlayerName", SetPlayerName);
			env->SetMethod(target, "getPlayerName", GetPlayerName);
			env->SetMethod(target, "setPlayerHealth", SetPlayerHealth);
			env->SetMethod(target, "getPlayerHealth", GetPlayerHealth);
			env->SetMethod(target, "setPlayerArmour", SetPlayerArmour);
			env->SetMethod(target, "getPlayerArmour", GetPlayerArmour);
			env->SetMethod(target, "setPlayerColor", SetPlayerColor);
			env->SetMethod(target, "getPlayerColor", GetPlayerColor);
			env->SetMethod(target, "broadcastClientMessage", BroadcastClientMessage);
			env->SetMethod(target, "sendClientMessage", SendClientMessage);
			env->SetMethod(target, "setPlayerIntoVehicle", SetPlayerIntoVehicle);
			env->SetMethod(target, "disablePlayerHud", DisablePlayerHud);
			env->SetMethod(target, "getPlayerGUID", GetPlayerGUID);

			env->SetMethod(target, "createVehicle", CreateVehicle);
			env->SetMethod(target, "deleteVehicle", DeleteVehicle);
			env->SetMethod(target, "setVehiclePosition", SetVehiclePosition);
			env->SetMethod(target, "getVehiclePosition", GetVehiclePosition);
			env->SetMethod(target, "setVehicleRotation", SetVehicleRotation);
			env->SetMethod(target, "getVehicleRotation", GetVehicleRotation);
			env->SetMethod(target, "setVehicleColours", SetVehicleColours);
			env->SetMethod(target, "getVehicleColours", GetVehicleColours);
			env->SetMethod(target, "setVehicleTyresBulletproof", SetVehicleTyresBulletproof);
			env->SetMethod(target, "getVehicleTyresBulletproof", GetVehicleTyresBulletproof);
			env->SetMethod(target, "setVehicleEngineStatus", SetVehicleEngineStatus);
			env->SetMethod(target, "getVehicleEngineStatus", GetVehicleEngineStatus);
			env->SetMethod(target, "setVehicleLocked", SetVehicleLocked);
			env->SetMethod(target, "isVehicleLocked", IsVehicleLocked);
			env->SetMethod(target, "setVehicleBodyHealth", SetVehicleBodyHealth);
			env->SetMethod(target, "setVehicleEngineHealth", SetVehicleEngineHealth);
			env->SetMethod(target, "setVehicleTankHealth", SetVehicleTankHealth);
			env->SetMethod(target, "getVehicleHealth", GetVehicleHealth);
			env->SetMethod(target, "setVehicleSirenState", SetVehicleSirenState);
			env->SetMethod(target, "getVehicleSirenState", GetVehicleSirenState);
			env->SetMethod(target, "getVehicleDriver", GetVehicleDriver);
			env->SetMethod(target, "getVehiclePassengers", GetVehiclePassengers);

		}
	}
}

NODE_MODULE_CONTEXT_AWARE_BUILTIN(orange, node::orange::Initialize);