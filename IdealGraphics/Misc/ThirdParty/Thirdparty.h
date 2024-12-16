#pragma once

#include "ThirdParty/Include/assimp/config.h"

#ifdef _DEBUG
#pragma comment(lib, "DebugLib/assimp/assimp-vc143-mtd.lib")

#else
#pragma comment(lib, "ReleaseLib/assimp/assimp-vc143-mt.lib")
#endif