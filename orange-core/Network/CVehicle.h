#pragma once

class CVehicle : public CEntity
{
	CPedestrian *driver = nullptr;
public:
	CVehicle(Hash model, float x, float y, float z, float heading);
	CVehicle(Hash model, float x, float y, float z, float heading, bool spawn);
	~CVehicle();
	bool IsTaxiLightOn();
	void SetColours(int first, int second);
	void SetCustomPrimaryColor(int red, int green, int blue);
	color_t GetCustomPrimaryColor();
	void ClearCustomPrimaryColor();
	bool IsPrimaryColorCustom();
	void SetCustomSecondaryColor(int red, int green, int blue);
	color_t GetCustomSecondaryColor();
	void ClearCustomSecondaryColor();
	bool IsSecondaryColorCustom();
	void SetPaintFade(float fade);
	float SetPaintFade();
	void SetSiren(bool state);
	bool IsSirenOn();
	void SetNumberPlateText(std::string text);
	float GetDirtLevel();
	void SetDirtLevel(float level);
	void SetExtraColors(int pearlescent, int wheel);
	void GetExtraColors(int & pearlescent, int & wheel);
	int GetConvertibleRoofState();
	bool IsNeonEnabled(int index);
	void SetNeonEnabled(int index, bool enabled);
	void GetNeonLightColor(int & red, int & green, int & blue);
	void SetNeonLightColor(int red, int green, int blue);
	void GetTyreSmokeColor(int & red, int & green, int & blue);
	void SetTyreSmokeColor(int red, int green, int blue);
	int GetWindowTint();
	void SetWindowTint(int tint);
	void TaskMove(float x, float y, float z, float speed);
	void SetDriver(CPedestrian * driver);
	void RemoveDriver();
};

