// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#define PROCESS_NAME L"GTA5.exe"

#include "targetver.h"

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <Shellapi.h>
#include <ShlObj.h>
#include <gdiplus.h>
#include <codecvt>


#include <vector>
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>

#include "tinyxml2.h"
#include "Registry.h"


#include "Utils.h"
#include "Injector.h"
#include "FolderBrowser.h"



#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;
