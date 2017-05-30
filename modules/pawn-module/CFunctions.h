#pragma once
#include "stdafx.h"

class CFunctions
{
	public:
		static CFunctions *singleInstance;
		static CFunctions *Get();
		int amx_CustomInit(AMX *amx);
};