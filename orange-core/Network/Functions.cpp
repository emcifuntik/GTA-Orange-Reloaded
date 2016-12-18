#include "stdafx.h"

namespace FPlayer
{
	void SendNotification(RakNet::BitStream *bitStream, RakNet::Packet *packet) // string message
	{
		RakNet::RakString message;
		bitStream->Read(message);
	}

	void SendClientMessage(RakNet::BitStream *bitStream, RakNet::Packet *packet) // string message, color_t color
	{
		RakNet::RakString message;
		color_t color;
		bitStream->Read(message);
		bitStream->Read(color);
		CChat::Get()->AddChatMessage(message.C_String(), color);
	}

	void GivePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) // Hash weapon, int ammo
	{
		Hash weapon;
		unsigned int ammo;
		bitStream->Read(weapon);
		bitStream->Read(ammo);
		CLocalPlayer::Get()->GiveWeapon(weapon, ammo);
	}

	void RemovePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) // Hash weapon
	{
		Hash weapon;
		bitStream->Read(weapon);
		CLocalPlayer::Get()->RemoveWeapon(weapon);
	}

	void RemoveAllWeapons(RakNet::BitStream *bitStream, RakNet::Packet *packet) // no arguments
	{
		CLocalPlayer::Get()->RemoveAllWeapons();
	}

	void GivePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) // Hash weapon, unsigned int ammo
	{
		Hash weapon;
		unsigned int ammo;
		bitStream->Read(weapon);
		bitStream->Read(ammo);
		CLocalPlayer::Get()->AddAmmo(weapon, ammo);
	}

	void RemovePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) // Hash weapon, unsigned int ammo
	{
		Hash weapon;
		unsigned int ammo;
		bitStream->Read(weapon);
		bitStream->Read(ammo);
		CLocalPlayer::Get()->RemoveAmmo(weapon, ammo);
	}

	void SetPlayerMoney(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{

	}

	void SetPlayerPos(RakNet::BitStream *bitStream, RakNet::Packet *packet) // CVector3 pos
	{
		CVector3 pos;
		bitStream->Read(pos);
		CLocalPlayer::Get()->SetCoordsKeepVehicle(pos.fX, pos.fY, pos.fZ);
	}

	void SetPlayerHeading(RakNet::BitStream *bitStream, RakNet::Packet *packet) // float heading
	{
		float heading;
		bitStream->Read(heading);
		CLocalPlayer::Get()->SetHeading(heading);
	}

	void SetPlayerHealth(RakNet::BitStream *bitStream, RakNet::Packet *packet) // unsigned short health
	{
		unsigned short health;
		bitStream->Read(health);
		CLocalPlayer::Get()->SetHealth(health);
	}

	void SetPlayerArmour(RakNet::BitStream *bitStream, RakNet::Packet *packet) // unsigned short armour
	{
		unsigned short armour;
		bitStream->Read(armour);
		CLocalPlayer::Get()->SetArmour(armour);
	}

	void SetPlayerModel(RakNet::BitStream *bitStream, RakNet::Packet *packet) // Hash model
	{
		Hash model;
		bitStream->Read(model);
		CLocalPlayer::Get()->ChangeModel(model);
	}

	void CreateBlip(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float x, y, z, scale;
		int color, sprite;

		bitStream->Read(guid);

		bitStream->Read(x);
		bitStream->Read(y);
		bitStream->Read(z);

		bitStream->Read(scale);
		bitStream->Read(color);
		bitStream->Read(sprite);
		
		new CNetworkBlip(guid, x, y, z, scale, color, sprite);
	}

	void DeleteBlip(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		bitStream->Read(guid);

		CNetworkBlip::GetByGUID(guid)->~CNetworkBlip();
	}

	void SetBlipScale(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float scale;

		bitStream->Read(guid);
		bitStream->Read(scale);

		CNetworkBlip::GetByGUID(guid)->SetScale(scale);
	}

	void SetBlipColor(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		int color;

		bitStream->Read(guid);
		bitStream->Read(color);

		CNetworkBlip::GetByGUID(guid)->SetColor(color);
	}

	void SetBlipSprite(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		int sprite;

		bitStream->Read(guid);
		bitStream->Read(sprite);

		CNetworkBlip::GetByGUID(guid)->SetSprite(sprite);
	}

	void SetBlipAsShortRange(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		bool _short;

		bitStream->Read(guid);
		bitStream->Read(_short);

		CNetworkBlip::GetByGUID(guid)->SetAsShortRange(_short);
	}

	void SetBlipRoute(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		bool route;

		bitStream->Read(guid);
		bitStream->Read(route);

		CNetworkBlip::GetByGUID(guid)->SetRoute(route);
	}

	void SetInfoMsg(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		bool set;
		bitStream->Read(set);

		if (set)
		{
			RakNet::RakString msg;
			bitStream->Read(msg);

			CNetworkUI::Get()->SetScreenInfo(msg.C_String());
		}
		else {
			CNetworkUI::Get()->UnsetScreenInfo();
		}
	}

	void CreateVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		VehicleData data;
		bitStream->Read(data);
		CNetworkVehicle *veh = new CNetworkVehicle();
		veh->m_GUID = data.GUID;
		veh->SetVehicleData(data, 0);
	}

	void CreateMarker(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float x, y, z, height, radius;
		int color;

		bitStream->Read(guid);

		bitStream->Read(x);
		bitStream->Read(y);
		bitStream->Read(z);

		bitStream->Read(height);
		bitStream->Read(radius);
		bitStream->Read(color);

		color_t c;
		Utils::HexToRGBA(color, c.red, c.green, c.blue, c.alpha);

		new CNetworkMarker(guid, x, y, z, height, radius, c);
	}

	void DeleteMarker(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		bitStream->Read(guid);

		CNetworkMarker::GetByGUID(guid)->~CNetworkMarker();
	}

	void CreateObject(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		ObjectData data;
		bitStream->Read(data);
		log << "Creating object: " << data.ToString() << std::endl;

		new CNetworkObject(data);
	}
}

