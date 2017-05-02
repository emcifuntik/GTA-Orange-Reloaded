#pragma once

namespace FPlayer
{
	void PreloadModels(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void SendNotification(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SendClientMessage(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void GivePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void RemovePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void RemoveAllWeapons(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void GivePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void RemovePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerMoney(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerPos(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerHeading(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerHealth(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerArmour(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerModel(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetPlayerIntoVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DisableHud(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateBlip(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DeleteBlip(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void SetBlipScale(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipColor(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipSprite(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipAsShortRange(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipRoute(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void AttachBlipToPlayer(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void AttachBlipToVehicle(RakNet::BitStream * bitStream, RakNet::Packet * packet);

	void SetInfoMsg(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehiclePosition(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleRotation(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleColours(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleTyresBulletproof(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleEngineStatus(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleBodyHealth(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleEngineHealth(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleTankHealth(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void SetVehicleSirenState(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DeleteVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateMarker(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DeleteMarker(RakNet::BitStream * bitStream, RakNet::Packet * packet);

	void Create3DText(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void Attach3DTextToVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void Attach3DTextToPlayer(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void Change3DTextContent(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void Delete3DText(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateObject(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void DeleteObject(RakNet::BitStream * bitStream, RakNet::Packet * packet);
}