#include "stdafx.h"

namespace node
{
	namespace orange
	{
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
			env->SetMethod(target, "onServerCommand", OnServerCommandFunction);

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