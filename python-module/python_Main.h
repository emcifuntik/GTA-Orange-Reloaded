#pragma once

class pythonFunctions
{
public:
	static void EventTriggered(char * Event, PyObject* args);
	static void PrintError();

	static void init();
	static bool loadResource(const char* resource);

	static PyObject* PyInit_GTAOrange(void);

	//static PyObject* GTAOrange_LoadClientScript(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_TriggerClientEvent(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_AddServerEvent(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_TriggerServerEvent(PyObject* self, PyObject* args);

	//Player
	static PyObject* GTAOrange_KickPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerPosition(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerPosition(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_IsPlayerInRange(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerHeading(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerHeading(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_RemovePlayerWeapons(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GivePlayerWeapon(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GivePlayerAmmo(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GivePlayerMoney(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerMoney(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_ResetPlayerMoney(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerMoney(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerModel(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerModel(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerName(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerName(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerHealth(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerHealth(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerArmour(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerArmour(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerColor(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerColor(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_BroadcastClientMessage(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SendClientMessage(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetPlayerIntoVehicle(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_DisablePlayerHud(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetPlayerGUID(PyObject* self, PyObject* args);

	//World
	static PyObject* GTAOrange_Print(PyObject* self, PyObject* args);
	//static PyObject* GTAOrange_Hash(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_PlayerExists(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_VehicleExists(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_CreateVehicle(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_DeleteVehicle(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetVehiclePosition(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_GetVehiclePosition(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_CreateObject(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_DeleteObject(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_CreatePickup(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_CreateBlipForAll(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_CreateBlipForPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_DeleteBlip(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipColor(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipScale(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipRoute(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipSprite(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipName(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetBlipAsShortRange(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_AttachBlipToPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_AttachBlipToVehicle(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_CreateMarkerForAll(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_CreateMarkerForPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_DeleteMarker(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_SendNotification(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_SetInfoMsg(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_UnsetInfoMsg(PyObject* self, PyObject* args);

	static PyObject* GTAOrange_Create3DTextForAll(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_Create3DTextForPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_Attach3DTextToVehicle(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_Attach3DTextToPlayer(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_Set3DTextContent(PyObject* self, PyObject* args);
	static PyObject* GTAOrange_Delete3DText(PyObject* self, PyObject* args);
};

