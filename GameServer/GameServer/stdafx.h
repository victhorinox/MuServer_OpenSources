#pragma once

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP

#define GAMESERVER_CLIENT "LIFE MU TEAM - GAMESERVER"
#define GAMESERVER_CS_CLIENT "LIFE MU TEAM - GAMESERVERCS"

#define GAMESERVER_CLIENTE_UPDATE 27
#define GAMESERVER_CLIENTE_PREMIUM 0

#if(GAMESERVER_TYPE==0)
#define GAMESERVER_VERSION "GS"
#else
#define GAMESERVER_VERSION "GSCS"
#endif

#ifndef GAMESERVER_TYPE
#define GAMESERVER_TYPE 0
#endif

#ifndef GAMESERVER_EXTRA
#define GAMESERVER_EXTRA 1
#endif

#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

#if(GAMESERVER_UPDATE==401)
#define GAMESERVER_SEASON "SEASON 4"
#endif

#if(GAMESERVER_UPDATE==603)
#define GAMESERVER_SEASON "SEASON 6"
#endif

#if(GAMESERVER_UPDATE==803)
#define GAMESERVER_SEASON "SEASON 8"
#endif


#define GAMESERVER_SEASON_PREMIUM "PREMIUM"

#ifndef GAMESERVER_LANGUAGE
#define GAMESERVER_LANGUAGE 1
#endif

#ifndef PROTECT_STATE
#define PROTECT_STATE 1
#endif

#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 1
#endif

#if(PROTECT_STATE==0)
#define GAMESERVER_NAME "Free"
#else
#define GAMESERVER_NAME "Premium"
#endif

#define CUSTOM_PET		1
#define BOT_BUFFER		1
#define	ALLBOTSSTRUC	1

#define MOVE_ITEM		1

#define OFFLINE_MODE			1
#define ADD_OFFMODE_PICKUP		0

// System Include
#include <windows.h>
#include <winsock2.h>
#include <mswSock.h>
#include <commctrl.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <Rpc.h>
#include <algorithm>
#include <string>
#include <atltime.h>
#include <dbghelp.h>
#include <Psapi.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")

#if(GAMESERVER_UPDATE>=701)
#if(NDEBUG==0)
#pragma comment(lib,"..\\..\\..\\Util\\cryptopp\\Debug\\cryptlib.lib")
#else
#pragma comment(lib,"..\\..\\..\\Util\\cryptopp\\Release\\cryptlib.lib")
#endif
#pragma comment(lib,"..\\..\\..\\Util\\mapm\\mapm.lib")
#endif

typedef char chr;

typedef float flt;

typedef short shrt;

typedef unsigned __int64 QWORD;

#define GAMESERVER_STATUS "STANDBY MODE"

#define GAMESERVER_STATUS_MODE "ACTIVE MODE"