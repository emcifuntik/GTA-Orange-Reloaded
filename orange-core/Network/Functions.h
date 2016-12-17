#pragma once

namespace FPlayer
{
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

	void CreateBlip(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DeleteBlip(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void SetBlipScale(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipColor(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipSprite(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipAsShortRange(RakNet::BitStream * bitStream, RakNet::Packet *packet);
	void SetBlipRoute(RakNet::BitStream * bitStream, RakNet::Packet * packet);

	void SetInfoMsg(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet);

	void CreateMarker(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void DeleteMarker(RakNet::BitStream * bitStream, RakNet::Packet * packet);
}