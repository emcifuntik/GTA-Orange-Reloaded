#include "stdafx.h"

namespace node
{
	namespace orange
	{
		//unsigned long CreateBlipForAll(std::string name, float x, float y, float z, float scale, int color, int sprite);
		void CreateBlipForAll(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			std::string name = *v8::String::Utf8Value(args[0]->ToString());
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			float scale = (float)args[4]->NumberValue();
			int color = (int)args[5]->NumberValue();
			int sprite = (int)args[6]->NumberValue();

			args.GetReturnValue().Set((long)API::Get().CreateBlipForAll(name, x, y, z, scale, color, sprite));
		}
		//unsigned long CreateBlipForPlayer(long playerid, std::string name, float x, float y, float z, float scale, int color, int sprite);
		void CreateBlipForPlayer(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long playerId = (long)args[0]->Int32Value();
			std::string name = *v8::String::Utf8Value(args[1]->ToString());
			float x = (float)args[2]->NumberValue();
			float y = (float)args[3]->NumberValue();
			float z = (float)args[4]->NumberValue();
			float scale = (float)args[5]->NumberValue();
			int color = (int)args[6]->NumberValue();
			int sprite = (int)args[7]->NumberValue();

			args.GetReturnValue().Set((long)API::Get().CreateBlipForPlayer(playerId, name, x, y, z, scale, color, sprite));
		}
		//void DeleteBlip(unsigned long guid);
		void DeleteBlip(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();

			API::Get().DeleteBlip(guid);
		}
		//void SetBlipColor(unsigned long guid, int color);
		void SetBlipColor(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			int color = (int)args[1]->NumberValue();

			API::Get().SetBlipColor(guid, color);
		}
		//void SetBlipScale(unsigned long guid, float scale);
		void SetBlipScale(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			float scale = (float)args[1]->NumberValue();

			API::Get().SetBlipScale(guid, scale);
		}
		//void SetBlipRoute(unsigned long guid, bool route);
		void SetBlipRoute(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			bool route = args[1]->BooleanValue();

			API::Get().SetBlipRoute(guid, route);
		}
		//void SetBlipSprite(unsigned long guid, int sprite);
		void SetBlipSprite(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			int sprite = (int)args[1]->NumberValue();

			API::Get().SetBlipSprite(guid, sprite);
		}
		//void SetBlipName(unsigned long guid, std::string name);
		void SetBlipName(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			std::string name = *v8::String::Utf8Value(args[1]->ToString());

			API::Get().SetBlipName(guid, name);
		}
		//void SetBlipAsShortRange(unsigned long guid, bool _short);
		void SetBlipAsShortRange(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			bool _short = args[1]->BooleanValue();

			API::Get().SetBlipAsShortRange(guid, _short);
		}
		//void AttachBlipToPlayer(unsigned long _guid, long player);
		void AttachBlipToPlayer(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			long playerId = args[1]->Int32Value();

			API::Get().AttachBlipToPlayer(guid, playerId);
		}
		//void AttachBlipToVehicle(unsigned long _guid, unsigned long vehicle);
		void AttachBlipToVehicle(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			unsigned long guid = (unsigned long)args[0]->Uint32Value();
			unsigned long vehicle = args[1]->Uint32Value();

			API::Get().AttachBlipToVehicle(guid, vehicle);
		}
	}
}