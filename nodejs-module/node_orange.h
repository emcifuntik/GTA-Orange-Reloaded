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

		void CreateVehicle(const v8::FunctionCallbackInfo<v8::Value>& args);
		void DeleteVehicle(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehiclePosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehiclePosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleColours(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleColours(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleTyresBulletproof(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleTyresBulletproof(const v8::FunctionCallbackInfo<v8::Value>& args);
		/*void SetVehicleCustomPrimaryColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleCustomPrimaryColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleCustomSecondaryColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleCustomSecondaryColor(const v8::FunctionCallbackInfo<v8::Value>& args);*/
		void SetVehicleEngineStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleEngineStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleLocked(const v8::FunctionCallbackInfo<v8::Value>& args);
		void IsVehicleLocked(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleBodyHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleEngineHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleTankHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleHealth(const v8::FunctionCallbackInfo<v8::Value>& args);
		/*void SetVehicleNumberPlate(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleNumberPlate(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleNumberPlateStyle(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleNumberPlateStyle(const v8::FunctionCallbackInfo<v8::Value>& args);*/
		void SetVehicleSirenState(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleSirenState(const v8::FunctionCallbackInfo<v8::Value>& args);
		/*void SetVehicleWheelColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleWheelColor(const v8::FunctionCallbackInfo<v8::Value>& args);
		void SetVehicleWheelType(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehicleWheelType(const v8::FunctionCallbackInfo<v8::Value>& args);*/
		void GetVehicleDriver(const v8::FunctionCallbackInfo<v8::Value>& args);
		void GetVehiclePassengers(const v8::FunctionCallbackInfo<v8::Value>& args);

	}
}