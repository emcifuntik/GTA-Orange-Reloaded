#include "stdafx.h"

namespace node
{
	namespace orange
	{
		//virtual unsigned long CreateVehicle(long hash, float x, float y, float z, float heading) = 0;
		void CreateVehicle(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long hash = args[0]->IsString() ? API::Get().Hash(*((v8::String::Utf8Value)args[0]->ToString())) : (long)args[0]->Int32Value();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			float heading = (float)args[4]->NumberValue();
			unsigned long vehicleId = API::Get().CreateVehicle(hash, x, y, z, heading);
			args.GetReturnValue().Set((long)vehicleId);
		}
		//virtual bool DeleteVehicle(unsigned long guid) = 0;
		void DeleteVehicle(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);
			
			long vehicleId = args[0]->Int32Value();
			args.GetReturnValue().Set(API::Get().DeleteVehicle(vehicleId));
		}
		//virtual bool SetVehiclePosition(unsigned long guid, float x, float y, float z) = 0;
		void SetVehiclePosition(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = args[0]->Int32Value();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			args.GetReturnValue().Set((long)API::Get().SetVehiclePosition(vehicleId, x, y, z));
		}
		//virtual CVector3 GetVehiclePosition(unsigned long guid) = 0;
		void GetVehiclePosition(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			CVector3 position = API::Get().GetVehiclePosition(vehicleId);

			v8::Local<v8::Object> vectorObject = v8::Object::New(isolate);
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "x"), v8::Number::New(isolate, position.fX));
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "y"), v8::Number::New(isolate, position.fY));
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "z"), v8::Number::New(isolate, position.fZ));

			args.GetReturnValue().Set(vectorObject);
		}
		//virtual bool SetVehicleRotation(unsigned long guid, float rx, float ry, float rz) = 0;
		void SetVehicleRotation(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = args[0]->Int32Value();
			float x = (float)args[1]->NumberValue();
			float y = (float)args[2]->NumberValue();
			float z = (float)args[3]->NumberValue();
			args.GetReturnValue().Set((long)API::Get().SetVehicleRotation(vehicleId, x, y, z));
		}
		//virtual CVector3 GetVehicleRotation(unsigned long guid) = 0;
		void GetVehicleRotation(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			CVector3 position = API::Get().GetVehicleRotation(vehicleId);

			v8::Local<v8::Object> vectorObject = v8::Object::New(isolate);
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "x"), v8::Number::New(isolate, position.fX));
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "y"), v8::Number::New(isolate, position.fY));
			vectorObject->Set(v8::String::NewFromUtf8(isolate, "z"), v8::Number::New(isolate, position.fZ));

			args.GetReturnValue().Set(vectorObject);
		}
		//virtual bool SetVehicleColours(unsigned long guid, int Color1, int Color2) = 0;
		void SetVehicleColours(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			int color1 = (int)args[1]->NumberValue();
			int color2 = (int)args[2]->NumberValue();

			args.GetReturnValue().Set(API::Get().SetVehicleColours(vehicleId, color1, color2));
		}
		//virtual bool GetVehicleColours(unsigned long guid, int *Color1, int *Color2) = 0;
		void GetVehicleColours(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			int color1 = 0;
			int color2 = 0;

			API::Get().GetVehicleColours(vehicleId, &color1, &color2);

			v8::Local<v8::Object> colorsObject = v8::Object::New(isolate);
			colorsObject->Set(v8::String::NewFromUtf8(isolate, "primary"), v8::Integer::New(isolate, color1));
			colorsObject->Set(v8::String::NewFromUtf8(isolate, "secondary"), v8::Integer::New(isolate, color2));

			args.GetReturnValue().Set(colorsObject);
		}
		//virtual bool SetVehicleTyresBulletproof(unsigned long guid, bool bulletproof) = 0;
		void SetVehicleTyresBulletproof(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			bool isBulletproof = (int)args[1]->BooleanValue();

			args.GetReturnValue().Set(API::Get().SetVehicleTyresBulletproof(vehicleId, isBulletproof));
		}
		//virtual bool GetVehicleTyresBulletproof(unsigned long guid) = 0;
		void GetVehicleTyresBulletproof(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			args.GetReturnValue().Set(API::Get().GetVehicleTyresBulletproof(vehicleId));
		}
		//virtual bool SetVehicleCustomPrimaryColor(unsigned long guid, int rColor, int gColor, int bColor) = 0; //Not implemented
		//virtual bool GetVehicleCustomPrimaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor) = 0; //Not implemented
		//virtual bool SetVehicleCustomSecondaryColor(unsigned long guid, int rColor, int gColor, int bColor) = 0; //Not implemented
		//virtual bool GetVehicleCustomSecondaryColor(unsigned long guid, int *rColor, int *gColor, int *bColor) = 0; //Not implemented
		//virtual bool SetVehicleEngineStatus(unsigned long guid, bool status, bool locked) = 0;
		void SetVehicleEngineStatus(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			bool status = (int)args[1]->BooleanValue();

			args.GetReturnValue().Set(API::Get().SetVehicleEngineStatus(vehicleId, status, false));
		}
		//virtual bool GetVehicleEngineStatus(unsigned long guid) = 0;
		void GetVehicleEngineStatus(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			args.GetReturnValue().Set(API::Get().GetVehicleEngineStatus(vehicleId));
		}
		//bool SetVehicleLocked(unsigned long guid, bool locked);
		void SetVehicleLocked(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			bool locked = (int)args[1]->BooleanValue();

			args.GetReturnValue().Set(API::Get().SetVehicleLocked(vehicleId, locked));
		}
		//bool IsVehicleLocked(unsigned long guid);
		void IsVehicleLocked(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			args.GetReturnValue().Set(API::Get().IsVehicleLocked(vehicleId));
		}
		//virtual bool SetVehicleBodyHealth(unsigned long guid, float health) = 0; //Not implemented
		void SetVehicleBodyHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			float health = (float)args[1]->NumberValue();

			args.GetReturnValue().Set(API::Get().SetVehicleBodyHealth(vehicleId, health));
		}
		//virtual bool SetVehicleEngineHealth(unsigned long guid, float health) = 0; //Not implemented
		void SetVehicleEngineHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			float health = (float)args[1]->NumberValue();

			args.GetReturnValue().Set(API::Get().SetVehicleEngineHealth(vehicleId, health));
		}
		//virtual bool SetVehicleTankHealth(unsigned long guid, float health) = 0; //Not implemented
		void SetVehicleTankHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			float health = (float)args[1]->NumberValue();

			args.GetReturnValue().Set(API::Get().SetVehicleTankHealth(vehicleId, health));
		}
		//virtual bool GetVehicleHealth(unsigned long guid, float *body, float *engine, float *tank) = 0;
		void GetVehicleHealth(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			float body, engine, tank;
			API::Get().GetVehicleHealth(vehicleId, &body, &engine, &tank);

			v8::Local<v8::Object> healthsObject = v8::Object::New(isolate);
			healthsObject->Set(v8::String::NewFromUtf8(isolate, "body"), v8::Number::New(isolate, body));
			healthsObject->Set(v8::String::NewFromUtf8(isolate, "engine"), v8::Number::New(isolate, engine));
			healthsObject->Set(v8::String::NewFromUtf8(isolate, "tank"), v8::Number::New(isolate, tank));

			args.GetReturnValue().Set(healthsObject);
		}
		//virtual bool SetVehicleNumberPlate(unsigned long guid, const char *text) = 0; //Not implemented
		//virtual std::string GetVehicleNumberPlate(unsigned long guid) = 0; //Not implemented
		//virtual bool SetVehicleNumberPlateStyle(unsigned long guid, int style) = 0; //Not implemented
		//virtual int GetVehicleNumberPlateStyle(unsigned long guid) = 0; //Not implemented
		//virtual bool SetVehicleSirenState(unsigned long guid, bool state) = 0;
		void SetVehicleSirenState(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			bool state = (int)args[1]->BooleanValue();

			args.GetReturnValue().Set(API::Get().SetVehicleSirenState(vehicleId, state));
		}
		//virtual bool GetVehicleSirenState(unsigned long guid) = 0;
		void GetVehicleSirenState(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			args.GetReturnValue().Set(API::Get().GetVehicleSirenState(vehicleId));
		}

		//virtual bool SetVehicleWheelColor(unsigned long guid, int color) = 0; //Not implemented
		//virtual int GetVehicleWheelColor(unsigned long guid) = 0; //Not implemented
		//virtual bool SetVehicleWheelType(unsigned long guid, int type) = 0; //Not implemented
		//virtual int GetVehicleWheelType(unsigned long guid) = 0; //Not implemented
		//virtual int GetVehicleDriver(unsigned long guid) = 0;
		void GetVehicleDriver(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();

			args.GetReturnValue().Set(API::Get().GetVehicleDriver(vehicleId));
		}
		//virtual std::vector<unsigned int> GetVehiclePassengers(unsigned long guid) = 0;
		void GetVehiclePassengers(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::Isolate* isolate = args.GetIsolate();
			v8::HandleScope scope(isolate);

			long vehicleId = (long)args[0]->Int32Value();
			std::vector<unsigned int> passangers = API::Get().GetVehiclePassengers(vehicleId);

			v8::Local<v8::Array> passangersArray = v8::Array::New(isolate, passangers.size());
			for (int i = 0; i < passangers.size(); i++)
			{
				passangersArray->Set(i, v8::Integer::New(isolate, passangers.at(i)));
			}
			args.GetReturnValue().Set(passangersArray);
		}
	}
}