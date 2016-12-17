#include "stdafx.h"


CVehicle::CVehicle(Hash model, float x, float y, float z, float heading):CEntity(0)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model) && STREAMING::IS_MODEL_A_VEHICLE(model))
	{
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model))
			scriptWait(0);
		Handle = VEHICLE::CREATE_VEHICLE(model, x, y, z, heading, true, false);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	}
}

CVehicle::CVehicle(Hash model, float x, float y, float z, float heading, bool spawn) :CEntity(0)
{
	if(spawn)
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model) && STREAMING::IS_MODEL_A_VEHICLE(model))
		{
			STREAMING::REQUEST_MODEL(model);
			while (!STREAMING::HAS_MODEL_LOADED(model))
				scriptWait(0);
			Handle = VEHICLE::CREATE_VEHICLE(model, x, y, z, heading, true, false);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		}
}


CVehicle::~CVehicle()
{
	VEHICLE::DELETE_VEHICLE(&Handle);
}

bool CVehicle::IsTaxiLightOn()
{
	return (VEHICLE::IS_TAXI_LIGHT_ON(Handle) ? true : false);
}

void CVehicle::SetColours(int first, int second)
{
	VEHICLE::SET_VEHICLE_COLOURS(Handle, first, second);
}

void CVehicle::SetCustomPrimaryColor(int red, int green, int blue)
{
	VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle, red, green, blue);
}

color_t CVehicle::GetCustomPrimaryColor()
{
	color_t tempColor;
	tempColor.alpha = 255;
	VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle, (int*)&(tempColor.red), (int*)&(tempColor.green), (int*)&(tempColor.blue));
	return tempColor;
}

void CVehicle::ClearCustomPrimaryColor()
{
	VEHICLE::CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle);
}

bool CVehicle::IsPrimaryColorCustom()
{
	return (VEHICLE::GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM(Handle) ? true : false);
}

void CVehicle::SetCustomSecondaryColor(int red, int green, int blue)
{
	VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle, red, green, blue);
}

color_t CVehicle::GetCustomSecondaryColor()
{
	color_t tempColor;
	tempColor.alpha = 255;
	VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle, (int*)&(tempColor.red), (int*)&(tempColor.green), (int*)&(tempColor.blue));
	return tempColor;
}

void CVehicle::ClearCustomSecondaryColor()
{
	VEHICLE::CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle);
}

bool CVehicle::IsSecondaryColorCustom()
{
	return (VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(Handle) ? true : false);
}

void CVehicle::SetPaintFade(float fade)
{
	VEHICLE::_SET_VEHICLE_PAINT_FADE(Handle, fade);
}

float CVehicle::SetPaintFade()
{
	return VEHICLE::_GET_VEHICLE_PAINT_FADE(Handle);
}

void CVehicle::SetSiren(bool state)
{
	VEHICLE::SET_VEHICLE_SIREN(Handle, state);
}

bool CVehicle::IsSirenOn()
{
	return (VEHICLE::IS_VEHICLE_SIREN_ON(Handle) ? true : false);
}

void CVehicle::SetNumberPlateText(std::string text)
{
	VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(Handle, (char*)text.c_str());
}

float CVehicle::GetDirtLevel()
{
	return VEHICLE::GET_VEHICLE_DIRT_LEVEL(Handle);
}

void CVehicle::SetDirtLevel(float level)
{
	return VEHICLE::SET_VEHICLE_DIRT_LEVEL(Handle, level);
}

void CVehicle::SetExtraColors(int pearlescent, int wheel)
{
	VEHICLE::SET_VEHICLE_EXTRA_COLOURS(Handle, pearlescent, wheel);
}

void CVehicle::GetExtraColors(int& pearlescent, int& wheel)
{
	VEHICLE::GET_VEHICLE_EXTRA_COLOURS(Handle, &pearlescent, &wheel);
}

int CVehicle::GetConvertibleRoofState()
{
	return VEHICLE::GET_CONVERTIBLE_ROOF_STATE(Handle);
}

bool CVehicle::IsNeonEnabled(int index) // 0 - 3
{
	return (VEHICLE::_IS_VEHICLE_NEON_LIGHT_ENABLED(Handle, index) ? true : false);
}

void CVehicle::SetNeonEnabled(int index, bool enabled)
{
	VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(Handle, index, enabled);
}

void CVehicle::GetNeonLightColor(int& red, int& green, int& blue)
{
	VEHICLE::_GET_VEHICLE_NEON_LIGHTS_COLOUR(Handle, &red, &green, &blue);
}

void CVehicle::SetNeonLightColor(int red, int green, int blue)
{
	VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(Handle, red, green, blue);
}

void CVehicle::GetTyreSmokeColor(int& red, int& green, int& blue)
{
	VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(Handle, &red, &green, &blue);
}

void CVehicle::SetTyreSmokeColor(int red, int green, int blue)
{
	VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(Handle, red, green, blue);
}

int CVehicle::GetWindowTint()
{
	return VEHICLE::GET_VEHICLE_WINDOW_TINT(Handle);
}

void CVehicle::SetWindowTint(int tint)
{
	VEHICLE::SET_VEHICLE_WINDOW_TINT(Handle, tint);
}

void CVehicle::TaskMove(float x, float y, float z, float speed)
{
	AI::TASK_VEHICLE_DRIVE_TO_COORD(driver->GetHandle(), Handle, x, y, z, speed, 0, GetModel(), 1074528293, 0.f, 0.f);
}

void CVehicle::SetDriver(CPedestrian *driver)
{
	this->driver = driver;
}

void CVehicle::RemoveDriver()
{
	SetDriver(nullptr);
}
