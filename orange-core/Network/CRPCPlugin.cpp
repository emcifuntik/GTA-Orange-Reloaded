#include "stdafx.h"

CRPCPlugin* CRPCPlugin::singleInstance = nullptr;

CRPCPlugin::CRPCPlugin()
{
	CNetworkConnection::Get()->client->AttachPlugin(&rpc);
	BindFunctions();
}

void CRPCPlugin::BindFunctions()
{
	rpc.RegisterSlot("PreloadModels", FPlayer::PreloadModels, 0);

	#pragma region Player functions
	rpc.RegisterSlot("SendNotification", FPlayer::SendNotification, 0);
	rpc.RegisterSlot("SendClientMessage", FPlayer::SendClientMessage, 0);
	rpc.RegisterSlot("GivePlayerWeapon", FPlayer::GivePlayerWeapon, 0);
	rpc.RegisterSlot("RemovePlayerWeapon", FPlayer::RemovePlayerWeapon, 0);
	rpc.RegisterSlot("RemoveAllWeapons", FPlayer::RemoveAllWeapons, 0);
	rpc.RegisterSlot("GivePlayerAmmo", FPlayer::GivePlayerAmmo, 0);
	rpc.RegisterSlot("RemovePlayerAmmo", FPlayer::RemovePlayerAmmo, 0);
	rpc.RegisterSlot("SetPlayerMoney", FPlayer::SetPlayerMoney, 0);
	rpc.RegisterSlot("SetPlayerPos", FPlayer::SetPlayerPos, 0);
	rpc.RegisterSlot("SetPlayerHeading", FPlayer::SetPlayerHeading, 0);
	rpc.RegisterSlot("SetPlayerHealth", FPlayer::SetPlayerHealth, 0);
	rpc.RegisterSlot("SetPlayerArmour", FPlayer::SetPlayerArmour, 0);
	rpc.RegisterSlot("SetPlayerModel", FPlayer::SetPlayerModel, 0);
	#pragma endregion

	#pragma region Blip functions
	rpc.RegisterSlot("CreateBlip", FPlayer::CreateBlip, 0);
	rpc.RegisterSlot("DeleteBlip", FPlayer::DeleteBlip, 0);
	rpc.RegisterSlot("SetBlipScale", FPlayer::SetBlipScale, 0);
	rpc.RegisterSlot("SetBlipColor", FPlayer::SetBlipColor, 0);
	rpc.RegisterSlot("SetBlipSprite", FPlayer::SetBlipSprite, 0);
	rpc.RegisterSlot("SetBlipAsShortRange", FPlayer::SetBlipAsShortRange, 0);
	rpc.RegisterSlot("SetBlipRoute", FPlayer::SetBlipRoute, 0);
	#pragma endregion

	#pragma region Marker functions
	rpc.RegisterSlot("CreateMarker", FPlayer::CreateMarker, 0);
	rpc.RegisterSlot("DeleteMarker", FPlayer::DeleteMarker, 0);
	#pragma endregion

	#pragma region UI functions
	rpc.RegisterSlot("SetInfoMsg", FPlayer::SetInfoMsg, 0);
	#pragma endregion

	#pragma region Vehicle functions
	rpc.RegisterSlot("CreateVehicle", FPlayer::CreateVehicle, 0);
	#pragma endregion

	#pragma region 3DText functions
	rpc.RegisterSlot("Create3DText",			FPlayer::Create3DText, 0);
	rpc.RegisterSlot("Attach3DTextToVehicle",	FPlayer::Attach3DTextToVehicle, 0);
	rpc.RegisterSlot("Attach3DTextToPlayer",	FPlayer::Attach3DTextToPlayer, 0);
	rpc.RegisterSlot("Change3DTextContent",		FPlayer::Change3DTextContent, 0);
	rpc.RegisterSlot("Delete3DText",			FPlayer::Delete3DText, 0);
	#pragma endregion

	#pragma region Object functions
	rpc.RegisterSlot("CreateObject", FPlayer::CreateObject, 0);
	#pragma endregion
}

CRPCPlugin * CRPCPlugin::Get()
{
	if (!singleInstance)
		singleInstance = new CRPCPlugin();
	return singleInstance;
}

CRPCPlugin::~CRPCPlugin()
{
	CNetworkConnection::Get()->client->DetachPlugin(&rpc);
}
