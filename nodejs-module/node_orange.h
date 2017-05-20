#pragma once

namespace node
{
	namespace orange
	{
		//void LoadClientScript(const v8::FunctionCallbackInfo<v8::Value>& args)
		void TriggerClientEvent(const v8::FunctionCallbackInfo<v8::Value>& args);
		//Player
		void KickPlayer(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerPosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerPosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		void IsPlayerInRange(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerHeading(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerHeading(const v8::FunctionCallbackInfo<v8::Value>& args);
		void RemovePlayerWeapons(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GivePlayerWeapon(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GivePlayerAmmo(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GivePlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args);
		void ResetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerModel(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerModel(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerName(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerName(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerArmour(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerArmour(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void BroadcastClientMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SendClientMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetPlayerIntoVehicle(const v8::FunctionCallbackInfo<v8::Value>& args);
		void DisablePlayerHud(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetPlayerGUID(const v8::FunctionCallbackInfo<v8::Value>& args);
	}
}