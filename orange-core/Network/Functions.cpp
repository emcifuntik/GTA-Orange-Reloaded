#include "stdafx.h"

namespace FPlayer
{
	void PreloadModels(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		size_t size;
		bitStream->Read(size);

		for (int i = 0; i < size; i++)
		{
			Hash model;
			bitStream->Read(model);
			if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
			{
				STREAMING::REQUEST_MODEL(model);
				while (!STREAMING::HAS_MODEL_LOADED(model))
					scriptWait(0);
			}
		}

		CChat::Get()->AddChatMessage("Models loaded!", {255, 255, 255, 255});
	}

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
		if (!CLocalPlayer::Get()->Spawned) CLocalPlayer::Get()->Spawn();
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

	void Create3DText(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float x, y, z, oX, oY, oZ, fontSize;
		int color, outColor;
		std::string content;
		TextAttachedTo attachType = TextAttachedTo::NON_ATTACHED;
		RakNet::RakNetGUID attachedTo;

		bitStream->Read(guid);
		bitStream->Read(x);
		bitStream->Read(y);
		bitStream->Read(z);
		bitStream->Read(color);
		bitStream->Read(outColor);
		bitStream->Read(fontSize);
		RakString contentRak;
		bitStream->Read(contentRak);
		content = contentRak.C_String();
		bitStream->Read(attachType);
		bitStream->Read(attachedTo);
		bitStream->Read(oX);
		bitStream->Read(oY);
		bitStream->Read(oZ);
		(new CNetwork3DText(guid, x, y, z, color, outColor, content, attachType, attachedTo))->SetFontSize(fontSize);
	}

	void Attach3DTextToVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float oX, oY, oZ;
		TextAttachedTo attachType = TextAttachedTo::VEHICLE_ATTACHED;
		RakNet::RakNetGUID attachedTo;

		bitStream->Read(guid);
		bitStream->Read(attachedTo);
		bitStream->Read(oX);
		bitStream->Read(oY);
		bitStream->Read(oZ);
		CNetwork3DText::GetByGUID(guid)->AttachToVehicle(attachedTo, oX, oY, oZ);
	}

	void Attach3DTextToPlayer(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		float oX, oY, oZ;
		TextAttachedTo attachType = TextAttachedTo::PLAYER_ATTACHED;
		RakNet::RakNetGUID attachedTo;

		bitStream->Read(guid);
		bitStream->Read(attachedTo);
		bitStream->Read(oX);
		bitStream->Read(oY);
		bitStream->Read(oZ);
		CNetwork3DText::GetByGUID(guid)->AttachToPlayer(attachedTo, oX, oY, oZ);
	}

	void Change3DTextContent(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		std::string content;

		bitStream->Read(guid);
		RakString contentRak;
		bitStream->Read(contentRak);
		content = contentRak.C_String();
		CNetwork3DText::GetByGUID(guid)->SetText(content);
	}

	void Delete3DText(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		RakNet::RakNetGUID guid;
		bitStream->Read(guid);
		CNetwork3DText::DeleteByGUID(guid);
	}

	void CreateObject(RakNet::BitStream *bitStream, RakNet::Packet *packet)
	{
		ObjectData data;
		bitStream->Read(data);
		new CNetworkObject(data);
	}
}

