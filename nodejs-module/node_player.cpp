#include "stdafx.h"

namespace node
{
	namespace orange
	{
		//virtual void LoadClientScript(std::string name, char* buffer, size_t size) = 0;
		//virtual void ClientEvent(const char * name, std::vector<MValue> args, long playerid) = 0;
		void TriggerClientEvent(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			if (args.Length() < 3)
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			if (!args[0]->IsNumber())
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "playerId is not number")));
				return;
			}
			if (!args[1]->IsString())
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "clientEvent is not string")));
				return;
			}
			long playerId = (long)args[0]->Int32Value();
			const char* eventName = (const char*)*v8::String::Utf8Value(args[1]->ToDetailString());

			v8::Handle<v8::Array> argsArray = v8::Handle<v8::Array>::Cast(args[2]);

			std::vector<MValue> args2(argsArray->Length());

			//TODO: Done array conversion later :D
		}
		//Player
		//virtual void KickPlayer(long playerid) = 0;
		void KickPlayer(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();

			API::Get().KickPlayer(playerId);
		}
		//virtual bool SetPlayerPosition(long playerid, float x, float y, float z) = 0;
		void SetPlayerPosition(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			args.GetReturnValue().Set(API::Get().SetPlayerPosition(playerId, x, y, z));
		}
		//virtual CVector3 GetPlayerPosition(long playerid) = 0;
		void GetPlayerPosition(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();

			CVector3 position = API::Get().GetPlayerPosition(playerId);

			v8::Local<v8::Array> positionArray = v8::Array::New(isolate, 3);
			positionArray->Set(0, v8::Number::New(isolate, position.fX));
			positionArray->Set(1, v8::Number::New(isolate, position.fY));
			positionArray->Set(2, v8::Number::New(isolate, position.fZ));

			args.GetReturnValue().Set(positionArray);
		}
		//virtual bool IsPlayerInRange(long playerid, float x, float y, float z, float range) = 0;
		void IsPlayerInRange(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			float range = (float)args[4]->NumberValue();
			args.GetReturnValue().Set(API::Get().IsPlayerInRange(playerId, x, y, z, range));
		}

		//virtual bool SetPlayerHeading(long playerid, float angle) = 0;
		void SetPlayerHeading(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			float angle = (float)args[1]->NumberValue();
			args.GetReturnValue().Set(API::Get().SetPlayerHeading(playerId, angle));
		}
		//virtual float GetPlayerHeading(long playerid) = 0;
		void GetPlayerHeading(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			if (args.Length() < 1)
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "Wrong number of arguments")));
				return;
			}
			if (!args[0]->IsNumber())
			{
				isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(isolate, "PlayerID is not number")));
				return;
			}

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set(API::Get().GetPlayerHeading(playerId));
		}
		//virtual bool RemovePlayerWeapons(long playerid) = 0;
		void RemovePlayerWeapons(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set(API::Get().RemovePlayerWeapons(playerId));
		}
		//virtual bool GivePlayerWeapon(long playerid, long weapon, long ammo) = 0;
		void GivePlayerWeapon(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			long weapon = args[1]->IsString() ? API::Get().Hash(*((v8::String::Utf8Value)args[1]->ToString())) : (long)args[1]->Int32Value();
			long ammo = (long)args[2]->Int32Value();
			args.GetReturnValue().Set(API::Get().GivePlayerWeapon(playerId, weapon, ammo));
		}
		//virtual bool GivePlayerAmmo(long playerid, long weapon, long ammo) = 0;
		void GivePlayerAmmo(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			long weapon = args[1]->IsString() ? API::Get().Hash(*((v8::String::Utf8Value)args[1]->ToString())) : (long)args[1]->Int32Value();
			long ammo = (long)args[2]->Int32Value();
			args.GetReturnValue().Set(API::Get().GivePlayerAmmo(playerId, weapon, ammo));
		}
		//virtual bool GivePlayerMoney(long playerid, long money) = 0;
		void GivePlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			long money = (long)args[1]->Int32Value();
			args.GetReturnValue().Set(API::Get().GivePlayerMoney(playerId, money));
		}
		//virtual bool SetPlayerMoney(long playerid, long money) = 0;
		void SetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			long money = (long)args[1]->Int32Value();
			args.GetReturnValue().Set(API::Get().SetPlayerMoney(playerId, money));
		}
		//virtual bool ResetPlayerMoney(long playerid) = 0;
		void ResetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->NumberValue();
			args.GetReturnValue().Set(API::Get().ResetPlayerMoney(playerId));
		}
		//virtual size_t GetPlayerMoney(long playerid) = 0;
		void GetPlayerMoney(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->NumberValue();
			args.GetReturnValue().Set((long)API::Get().GetPlayerMoney(playerId));
		}
		//virtual bool SetPlayerModel(long playerid, long model) = 0;
		void SetPlayerModel(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			long model = args[1]->IsString() ? API::Get().Hash(*((v8::String::Utf8Value)args[1]->ToString())) : (long)args[1]->Int32Value();
			args.GetReturnValue().Set(API::Get().SetPlayerModel(playerId, model));
		}
		//virtual long GetPlayerModel(long playerid) = 0;
		void GetPlayerModel(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set((long)API::Get().GetPlayerModel(playerId));
		}
		//virtual bool SetPlayerName(long playerid, const char * name) = 0;
		void SetPlayerName(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			v8::String::Utf8Value name(args[1]->ToString());
			args.GetReturnValue().Set(API::Get().SetPlayerName(playerId, *name));
		}
		//virtual std::string GetPlayerName(long playerid) = 0;
		void GetPlayerName(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			std::string name = API::Get().GetPlayerName(playerId);
			args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, name.c_str()));
		}
		//virtual bool SetPlayerHealth(long playerid, float health) = 0;
		void SetPlayerHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			float health = (float)args[1]->NumberValue();
			printf("Health: %f\n", health);
			args.GetReturnValue().Set(API::Get().SetPlayerHealth(playerId, health));
		}
		//virtual float GetPlayerHealth(long playerid) = 0;
		void GetPlayerHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set((long)API::Get().GetPlayerHealth(playerId));
		}
		//virtual bool SetPlayerArmour(long playerid, float armour) = 0;
		void SetPlayerArmour(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			float armour = (float)args[1]->NumberValue();
			args.GetReturnValue().Set(API::Get().SetPlayerArmour(playerId, armour));
		}
		//virtual float GetPlayerArmour(long playerid) = 0;
		void GetPlayerArmour(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set((long)API::Get().GetPlayerArmour(playerId));
		}
		//virtual bool SetPlayerColor(long playerid, unsigned int color) = 0;
		void SetPlayerColor(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			unsigned int color = (unsigned int)args[1]->NumberValue();
			args.GetReturnValue().Set(API::Get().SetPlayerColor(playerId, color));
		}
		//virtual unsigned int GetPlayerColor(long playerid) = 0;
		void GetPlayerColor(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set((unsigned int)API::Get().GetPlayerColor(playerId));
		}
		//virtual void BroadcastClientMessage(const char * message, unsigned int color) = 0;
		void BroadcastClientMessage(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			v8::String::Utf8Value message(args[0]->ToString());
			unsigned int color = (unsigned int)args[1]->NumberValue();
			API::Get().BroadcastClientMessage(*message, color);
		}
		//virtual bool SendClientMessage(long playerid, const char * message, unsigned int color) = 0;
		void SendClientMessage(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			v8::String::Utf8Value message(args[1]->ToString());
			unsigned int color = (unsigned int)args[2]->NumberValue();
			API::Get().SendClientMessage(playerId, *message, color);
		}
		//virtual bool SetPlayerIntoVehicle(long playerid, unsigned long vehicle, char seat) = 0;
		void SetPlayerIntoVehicle(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			unsigned long vehicle = (unsigned long)args[1]->NumberValue();
			char seat = (char)args[2]->NumberValue();
			args.GetReturnValue().Set(API::Get().SetPlayerIntoVehicle(playerId, vehicle, seat));
		}
		//virtual void DisablePlayerHud(long playerid, bool toggle) = 0;
		void DisablePlayerHud(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			bool toggle = args[1]->BooleanValue();
			API::Get().DisablePlayerHud(playerId, toggle);
		}
		//virtual unsigned long GetPlayerGUID(long playerid) = 0;
		void GetPlayerGUID(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			args.GetReturnValue().Set((unsigned int)API::Get().GetPlayerGUID(playerId));
		}
	}
}