#pragma once

typedef DWORD Void;
typedef DWORD Any;
typedef DWORD uint;
typedef DWORD Hash;
typedef int Entity;
typedef int _Player;
typedef int FireId;
typedef int Ped;
typedef int Vehicle;
typedef int Cam;
typedef int CarGenerator;
typedef int Group;
typedef int Train;
typedef int Pickup;
typedef int Object;
typedef int Weapon;
typedef int Interior;
typedef int Blip;
typedef int Texture;
typedef int TextureDict;
typedef int CoverPoint;
typedef int Camera;
typedef int TaskSequence;
typedef int ColourIndex;
typedef int Sphere;
typedef int ScrHandle;

#pragma pack(push, 1)
typedef struct
{
	float x;
	float __padding_x;
	float y;
	float __padding_y;
	float z;
	float __padding_z;
} Vector3;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
} color_t;
#pragma pack(pop)