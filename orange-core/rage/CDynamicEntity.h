#pragma once
namespace rage {
	class CDynamicEntity : public rage::CEntity
	{
	public:
		char pad_0x00D0[0x20]; //0x00D0
		rage::CPortalTracker* portalTracker; //0x00F0 
	}; //Size=0x00F8
}