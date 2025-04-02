// ServerDisplayer.cpp: implementation of the CServerDisplayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerDisplayer.h"
#include "CustomArena.h"
#include "GameMain.h"
#include "Log.h"
#include "Protect.h"
#include "resource.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "InvasionManager.h"
#include "Message.h"

CServerDisplayer gServerDisplayer;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerDisplayer::CServerDisplayer() // OK
{
	this->EventBc = -1;

	for(int n=0;n < MAX_LOG_TEXT_LINE;n++)
	{
		memset(&this->m_log[n],0,sizeof(this->m_log[n]));
	}

	this->m_font = CreateFont(50,0,0,0,FW_THIN,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,"Times");
	this->m_font2 = CreateFont(24,0,0,0,FW_THIN,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,"Times");
	this->m_font3 = CreateFont(15,0,0,0,FW_THIN,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,"MS Sans Serif");

	this->m_brush[0] = CreateSolidBrush(RGB(100,100,100));
	this->m_brush[1] = CreateSolidBrush(RGB(110,220,130));
	this->m_brush[2] = CreateSolidBrush(RGB(240,120,10)); //-> Titulo
	this->m_brush[3] = CreateSolidBrush(RGB(50,50,50)); 
	this->m_brush[4] = CreateSolidBrush(RGB(25,25,25)); //-> Fundo

	strcpy_s(this->m_DisplayerText[0],"STANDBY MODE");
	strcpy_s(this->m_DisplayerText[1],"ACTIVE MODE");
}

CServerDisplayer::~CServerDisplayer() // OK
{
	DeleteObject(this->m_font);
	DeleteObject(this->m_brush[0]);
	DeleteObject(this->m_brush[1]);
	DeleteObject(this->m_brush[2]);
	DeleteObject(this->m_brush[3]);
	DeleteObject(this->m_brush[4]);
	DeleteObject(this->m_brush[5]);
}

void CServerDisplayer::Init(HWND hWnd) // OK
{
	PROTECT_START

	this->m_hwnd = hWnd;

	PROTECT_FINAL

	gLog.AddLog(1,"LOG");

	gLog.AddLog(gServerInfo.m_WriteChatLog,"CHAT_LOG");

	gLog.AddLog(gServerInfo.m_WriteCommandLog,"COMMAND_LOG");

	gLog.AddLog(gServerInfo.m_WriteTradeLog,"TRADE_LOG");

	gLog.AddLog(gServerInfo.m_WriteConnectLog,"CONNECT_LOG");

	gLog.AddLog(gServerInfo.m_WriteHackLog,"HACK_LOG");

	gLog.AddLog(gServerInfo.m_WriteCashShopLog,"CASH_SHOP_LOG");

	gLog.AddLog(gServerInfo.m_WriteChaosMixLog,"CHAOS_MIX_LOG");
}

void CServerDisplayer::Run() // OK
{
	this->Interface(this->m_hwnd);
	this->LogTextPaint();
	this->PaintName();
	this->SetWindowName();
	this->PaintAllInfo();
	this->PaintOnline();
	//this->PaintPremium();
	this->PaintSeason();
	this->PaintEventTime();
	this->PaintInvasionTime();
	this->PaintCustomArenaTime();
	this->LogTextPaintConnect();
	this->LogTextPaintGlobalMessage();
}

void CServerDisplayer::SetWindowName() // OK
{
	char buff[256];

	wsprintf(buff,"[%s] %s (Online: %d)",GAMESERVER_VERSION,gServerInfo.m_ServerName, gObjTotalUser);

	SetWindowText(this->m_hwnd,buff);

	HWND hWndStatusBar = GetDlgItem(this->m_hwnd, IDC_STATUSBAR);

	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	RECT rect2;

	GetClientRect(hWndStatusBar,&rect2);

	MoveWindow(hWndStatusBar,0,rect.bottom-rect2.bottom+rect2.top,rect.right,rect2.bottom-rect2.top,true);

	int iStatusWidths[] = {190,270,360,450,580, -1};

	char text[256];

	SendMessage(hWndStatusBar, SB_SETPARTS, 6, (LPARAM)iStatusWidths);

	wsprintf(text, "GameServer %s - Update %d ", GAMESERVER_NAME,GAMESERVER_CLIENTE_UPDATE);

	SendMessage(hWndStatusBar, SB_SETTEXT, 0,(LPARAM)text);

	wsprintf(text, "OffStore: %d", gObjOffStore);

	SendMessage(hWndStatusBar, SB_SETTEXT, 1,(LPARAM)text);

	wsprintf(text, "OffAttack: %d", gObjOffAttack);

	SendMessage(hWndStatusBar, SB_SETTEXT, 2,(LPARAM)text);

	wsprintf(text, "BotsBuffer: %d", gObjTotalBot);

	SendMessage(hWndStatusBar, SB_SETTEXT, 3,(LPARAM)text);

	wsprintf(text, "Monsters: %d/%d", gObjTotalMonster,MAX_OBJECT_MONSTER+10);

	SendMessage(hWndStatusBar, SB_SETTEXT, 4,(LPARAM)text);

	SendMessage(hWndStatusBar, SB_SETTEXT, 5,(LPARAM)NULL);

	ShowWindow(hWndStatusBar, SW_SHOW);
}

void CServerDisplayer::PaintAllInfo() //
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	long Medida = rect.right - 490;

	Medida = Medida / 3;

	rect.right = rect.right - 490 - Medida - Medida;
	//--
	rect.top = rect.top + 55;

	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font2);
	
	SetTextColor(hdc, RGB(250, 250, 250));

	if(gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		FillRect(hdc, &rect,this->m_brush[0]);
		DrawText(hdc, GAMESERVER_STATUS, sizeof(GAMESERVER_STATUS), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		FillRect(hdc, &rect,this->m_brush[1]);
		DrawText(hdc, GAMESERVER_STATUS_MODE, sizeof(GAMESERVER_STATUS_MODE), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	SelectObject(hdc,OldFont);

	SetBkMode(hdc,OldBkMode);

	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::PaintOnline() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	long Medida = rect.right - 480;
	long Media = Medida / 2;
	Medida = Medida / 3;

	rect.right = rect.right - 480 - Medida;

	rect.left = Medida + 2;
	//--
	rect.top = rect.top + 55;

	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	SetTextColor(hdc, RGB(250, 250, 250));

	FillRect(hdc, &rect, this->m_brush[1]);

	char text[25];

	if(gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		FillRect(hdc, &rect,this->m_brush[0]);
		DrawText(hdc, GAMESERVER_SEASON, sizeof(GAMESERVER_SEASON), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	else
	{
		FillRect(hdc, &rect,this->m_brush[1]);
		DrawText(hdc, GAMESERVER_SEASON, sizeof(GAMESERVER_SEASON), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
/*
#if(ISPREMIUN == 0)
	if (gServerInfo.m_ServerMaxUserNumber <= 150)
	{
		wsprintf(text, "ONLINE: %d/%d", gObjTotalUser, gServerInfo.m_ServerMaxUserNumber);
	}
	else
	{
		wsprintf(text, "ONLINE: %d/%d", gObjTotalUser, 150);
	}
#else
	wsprintf(text, "ONLINE: %d/%d", gObjTotalUser, gServerInfo.m_ServerMaxUserNumber);
#endif

	if (gObjTotalUser > 0)
	{
		TextOut(hdc, Media - 50, rect.top + 2, text, strlen(text));
	}
	else
	{
		TextOut(hdc, Media - 50, rect.top + 2, text, strlen(text));
	}*/

	SelectObject(hdc, OldFont);

	SetBkMode(hdc, OldBkMode);

	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintSeason() // OK Season6
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	long Medida = rect.right - 475;

	Medida = Medida / 3;

	rect.right = rect.right - 475;

	rect.left = Medida + Medida + 2;
	//--
	rect.top = rect.top + 55;

	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	SetTextColor(hdc, RGB(250, 250, 250));

	if(gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		FillRect(hdc, &rect,this->m_brush[0]);
		DrawText(hdc, GAMESERVER_SEASON_PREMIUM, sizeof(GAMESERVER_SEASON_PREMIUM), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		FillRect(hdc, &rect,this->m_brush[1]);
		DrawText(hdc, GAMESERVER_SEASON_PREMIUM, sizeof(GAMESERVER_SEASON_PREMIUM), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintPremium() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.left= rect.right-180;
	rect.right	= rect.right-5;
	rect.top = rect.bottom-72;
	rect.bottom = rect.bottom-22;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font2);

	#if(PROTECT_STATE==1)

	SetTextColor(hdc,RGB(250,250,250));
	FillRect(hdc,&rect,this->m_brush[1]);
	TextOut(hdc,rect.right-165,rect.top+14,"PREMIUM",7);

	#else

	SetTextColor(hdc,RGB(200,200,200));
	FillRect(hdc,&rect,this->m_brush[0]);
	TextOut(hdc,rect.right-165,rect.top+14,"FREE VERSION",12);

	#endif

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::PaintName() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.top = 0;
	rect.bottom = 50;


	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font);
	
#if(GAMESERVER_TYPE==0)
	{
		SetTextColor(hdc,RGB(255,255,255));
		FillRect(hdc,&rect,this->m_brush[2]);
		//TextOut(hdc,(rect.right/2)-sizeof(GAMESERVER_CLIENT),0,GAMESERVER_CLIENT,sizeof(GAMESERVER_CLIENT));

		//std::string strOut = GAMESERVER_CLIENT;
		DrawText(hdc, GAMESERVER_CLIENT, sizeof(GAMESERVER_CLIENT), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
#endif

#if(GAMESERVER_TYPE==1)
	{
		SetTextColor(hdc,RGB(255,255,255));
		FillRect(hdc,&rect,this->m_brush[2]);
		//TextOut(hdc,(rect.right/2)-sizeof(GAMESERVER_CLIENT),0,GAMESERVER_CLIENT,sizeof(GAMESERVER_CLIENT));

		//std::string strOut = GAMESERVER_CLIENT;
		DrawText(hdc, GAMESERVER_CS_CLIENT, sizeof(GAMESERVER_CS_CLIENT), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
#endif


	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);

}


void CServerDisplayer::PaintEventTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right-462;
	int posX2 = rect.right-370;

	rect.left	= rect.right-470;
	rect.right	= rect.right-320;
	rect.top	= 55;
	rect.bottom = 305;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[3]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+8,rect.top+3,"EVENTS",7);

	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Blood Castle");

	if (this->EventBc == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventBc == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventBc;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;
		wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
	}
	
	TextOut(hdc,posX1,76,text1,strlen(text1));
	if (this->EventBc == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventBc == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventBc < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,76,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Devil Square");

	if (this->EventDs == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventDs == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventDs;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;
		wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
	}

	TextOut(hdc,posX1,91,text1,strlen(text1));
	if (this->EventDs == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventDs == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventDs < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}

	TextOut(hdc,posX2,91,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Chaos Castle");

	if (this->EventCc == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCc == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCc;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;
		wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
	}

	TextOut(hdc,posX1,106,text1,strlen(text1));
	if (this->EventCc == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCc == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCc < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,106,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Illusion Temple");

	if (this->EventIt == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventIt == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventIt;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}
	
	TextOut(hdc,posX1,121,text1,strlen(text1));
	if (this->EventIt == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventIt == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventIt < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,121,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Online Lottery");

	if (this->EventCustomLottery == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCustomLottery == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCustomLottery;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,151,text1,strlen(text1));
	if (this->EventCustomLottery == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCustomLottery == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCustomLottery < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,151,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Custom Quiz");

	if (this->EventCustomQuiz == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCustomQuiz == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCustomQuiz;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;
		wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,196,text1,strlen(text1));
	if (this->EventCustomQuiz == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCustomQuiz == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCustomQuiz < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,196,text2,strlen(text2));

	//--

	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Custom Bonus");

	if (this->EventCustomBonus == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCustomBonus == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCustomBonus;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,166,text1,strlen(text1));
	if (this->EventCustomBonus == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCustomBonus == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCustomBonus < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,166,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Custom Drop");

	if (this->EventDrop == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventDrop == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventDrop;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,181,text1,strlen(text1));
	if (this->EventDrop == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventDrop == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventDrop < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,181,text2,strlen(text2));

	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "King of Mu");

	if (this->EventKing == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventKing == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventKing;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,211,text1,strlen(text1));
	if (this->EventKing == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventKing == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventKing < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,211,text2,strlen(text2));

	//--
	
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Team VS Team");

	if (this->EventTvT == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventTvT == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventTvT;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,226,text1,strlen(text1));
	if (this->EventTvT == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventTvT == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventTvT < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,226,text2,strlen(text2));
	
/*
	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Selupam");

	if (this->EventIt == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventIt == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventIt;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}
	
	TextOut(hdc,posX1,226,text1,strlen(text1));
	if (this->EventIt == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventIt == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventIt < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,226,text2,strlen(text2));
	*/

	//--

#if(GAMESERVER_UPDATE>=601)
	//--
	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Moss Merchant");

	if (this->EventMoss == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventMoss == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventMoss;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,136,text1,strlen(text1));
	if (this->EventMoss == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventMoss == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventMoss < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2,136,text2,strlen(text2));
	#endif

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
	
#endif
}
void CServerDisplayer::PaintInvasionTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right-150;
	int posX2 = rect.right-60;

	rect.left	= rect.right-155;
	rect.right	= rect.right-5;
	rect.top	= 55;
	rect.bottom = 455;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[3]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[24];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+5,rect.top+3,"INVASION MANAGER",16);

	for (int n=0; n < 25; n++)
	{
		SetTextColor(hdc,RGB(250,220,10));

		//gInvasionManager.m_InvasionInfo[0].AlarmTime = 1;

		if (n==0) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2000)); }
		if (n==1) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2001)); }
		if (n==2) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2002)); }
		if (n==3) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2003)); }
		if (n==4) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2004)); }
		if (n==5) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2005)); }
		if (n==6) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2006)); }
		if (n==7) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2007)); }
		if (n==8) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2008)); }
		if (n==9) { wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2009)); }
		if (n==10){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2010)); }
		if (n==11){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2011)); }
		if (n==12){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2012)); }
		if (n==13){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2013)); }
		if (n==14){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2014)); }
		if (n==15){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2015)); }
		if (n==16){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2016)); }
		if (n==17){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2017)); }
		if (n==18){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2018)); }
		if (n==19){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2019)); }
		if (n==20){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2020)); }
		if (n==21){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2021)); }
		if (n==22){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2022)); }
		if (n==23){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2023)); }
		if (n==24){ wsprintf(text1,"%d - %s",n, gMessage.GetMessage(2024)); }

		if (this->EventInvasion[n] == -1)
		{
			wsprintf(text2, "Disabled");
		}
		else if (this->EventInvasion[n] == 0)
		{
			wsprintf(text2, "Online");
		}
		else
		{
			totalseconds	= this->EventInvasion[n];
			hours			= totalseconds/3600;
			minutes			= (totalseconds/60) % 60;  
			seconds			= totalseconds % 60;

			if (hours > 23)
			{
				days = hours/24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
	
		TextOut(hdc,posX1,rect.top+20+(15*n),text1,strlen(text1));
		if (this->EventInvasion[n] == -1)
		{
			SetTextColor(hdc,RGB(250,20,10));
		}
		else if (this->EventInvasion[n] == 0)
		{
			SetTextColor(hdc,RGB(10,190,10));
		}
		else if (this->EventInvasion[n] < 300)
		{
			SetTextColor(hdc,RGB(10,190,10));
		}
		else
		{
			SetTextColor(hdc,RGB(250,250,250));
		}
		TextOut(hdc,posX2+5,rect.top+20+(15*n),text2,strlen(text2));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
#endif
}

void CServerDisplayer::PaintCustomArenaTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right-310;
	int posX2 = rect.right-200;

	rect.left	= rect.right-315;
	rect.right	= rect.right-160;
	rect.top	= 55;
	rect.bottom = 305;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[3]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+5,rect.top+3,"CUSTOM ARENA",13);

	for (int n=0; n < 15; n++)
	{

		SetTextColor(hdc,RGB(250,220,10));

		if (strcmp("",gCustomArena.GetArenaName(n)) == 0)
		{
			wsprintf(text1,"%d - Empty",n);
		}
		else
		{
			wsprintf(text1,"%d - %s",n,gCustomArena.GetArenaName(n));
		}

		if (this->EventCustomArena[n] == -1)
		{
			wsprintf(text2, "Disabled");
		}
		else if (this->EventCustomArena[n] == 0)
		{
			wsprintf(text2, "Online");
		}
		else
		{
			totalseconds	= this->EventCustomArena[n];
			hours			= totalseconds/3600;
			minutes			= (totalseconds/60) % 60;  
			seconds			= totalseconds % 60;

			if (hours > 23)
			{
				days = hours/24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
	
		TextOut(hdc,posX1,rect.top+20+(15*n),text1,strlen(text1));
		if (this->EventCustomArena[n] == -1)
		{
			SetTextColor(hdc,RGB(250,20,10));
		}
		else if (this->EventCustomArena[n] == 0)
		{
			SetTextColor(hdc,RGB(10,190,10));
		}
		else if (this->EventCustomArena[n] < 300)
		{
			SetTextColor(hdc,RGB(10,190,10));
		}
		else
		{
			SetTextColor(hdc,RGB(250,250,250));
		}
		TextOut(hdc,posX2-10,rect.top+20+(15*n),text2,strlen(text2));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);

#else

	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	/*
	int posX1 = rect.right-462;
	int posX2 = rect.right-370;

	rect.left	= rect.right-470;
	rect.right	= rect.right-320;
	rect.top	= 55;
	rect.bottom = 305;
	*/

	int posX1 = rect.right-150;
	int posX2 = rect.right-60;

	rect.left	= rect.right-155;
	rect.right	= rect.right-5;
	rect.top = 55;
	rect.bottom = 305;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[3]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	char text3[30];
	char text4[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+5,rect.top+2,"EVENTS",7);

	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Loren Deep");

	if (this->EventCastleDeep == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCastleDeep == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCastleDeep;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,255,text1,strlen(text1));
	if (this->EventCastleDeep == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCastleDeep == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCastleDeep < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2-5,255,text2,strlen(text2));

	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Crywolf");

	if (this->EventCryWolf == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCryWolf == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds	= this->EventCryWolf;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc,posX1,271,text1,strlen(text1));
	if (this->EventCryWolf == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else if (this->EventCryWolf == 0)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else if (this->EventCryWolf < 300)
	{
		SetTextColor(hdc,RGB(10,190,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2-5,271,text2,strlen(text2));

	SetTextColor(hdc,RGB(250,220,10));
	wsprintf(text1, "Castle Siege");

	if (this->EventCs == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCs == 0)
	{
		wsprintf(text2, "Adjust Date");
		wsprintf(text3, " ");
		wsprintf(text4, " ");
	}
	else
	{
		totalseconds	= this->EventCs;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text4, "- Next Stage: %d day(s)+", days);
		}
		else
		{
			wsprintf(text4, "- Next Stage: %02d:%02d:%02d", hours, minutes, seconds);
		}

		if(this->EventCsState == -1)
			wsprintf(text3, "- Stage %d: None", this->EventCsState);
		if(this->EventCsState == 0)
			wsprintf(text3, "- Stage %d: Idle 1", this->EventCsState);
		if(this->EventCsState == 1)
			wsprintf(text3, "- Stage %d: Guild Register", this->EventCsState);
		if(this->EventCsState == 2)
			wsprintf(text3, "- Stage %d: Idle 2", this->EventCsState);
		if(this->EventCsState == 3)
			wsprintf(text3, "- Stage %d: Mark Register", this->EventCsState);
		if(this->EventCsState == 4)
			wsprintf(text3, "- Stage %d: Idle 3", this->EventCsState);
		if(this->EventCsState == 5)
			wsprintf(text3, "- Stage %d: Notify", this->EventCsState);
		if(this->EventCsState == 6)
			wsprintf(text3, "- Stage %d: Ready Siege", this->EventCsState);
		if(this->EventCsState == 7)
			wsprintf(text3, "- Stage %d: Started Siege", this->EventCsState);
		if(this->EventCsState == 8)
			wsprintf(text3, "- Stage %d: Ended Siege", this->EventCsState);
		if(this->EventCsState == 9)
			wsprintf(text3, "- Stage %d: End All", this->EventCsState);
			
		if (this->EventCs)
			wsprintf(text2, "Stage %d", this->EventCsState);
	}

	TextOut(hdc,posX1,287,text1,strlen(text1));
	if (this->EventCs == -1)
	{
		SetTextColor(hdc,RGB(250,20,10));
	}
	else
	{
		SetTextColor(hdc,RGB(250,250,250));
	}
	TextOut(hdc,posX2-5,287,text2,strlen(text2));

	TextOut(hdc,posX1-5,303,text3,strlen(text3));

	TextOut(hdc,posX1-5,319,text4,strlen(text4));

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);

#endif

}

void CServerDisplayer::LogTextPaint() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.top = 200;

	HDC hdc = GetDC(this->m_hwnd);

	//FillRect(hdc,&rect,(HBRUSH)GetStockObject(0));

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	int line = MAX_LOG_TEXT_LINE;

	int count = (((this->m_count-1)>=0)?(this->m_count-1):(MAX_LOG_TEXT_LINE-1));

	for(int n=0;n < MAX_LOG_TEXT_LINE;n++)
	{
		switch(this->m_log[count].color)
		{
			case LOG_BLACK:
				SetTextColor(hdc,RGB(250,250,250));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_RED:
				SetTextColor(hdc,RGB(240,20,10));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_GREEN:
				SetTextColor(hdc,RGB(10,240,20));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BLUE:
				SetTextColor(hdc,RGB(10,120,240));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BOT:
				SetTextColor(hdc,RGB(250,20,70));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_USER:
				SetTextColor(hdc,RGB(250,150,50));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_EVENT:
				SetTextColor(hdc,RGB(10,100,200));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_ALERT:
				SetTextColor(hdc,RGB(150,10,80));
				SetBkMode(hdc,TRANSPARENT);
				break;
		}

		int size = strlen(this->m_log[count].text);

		if(size > 1)
		{
			TextOut(hdc,0,(94+(line*15)),this->m_log[count].text,size);
			line--;
		}

		count = (((--count)>=0)?count:(MAX_LOG_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::LogTextPaintConnect() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);
	/*
	int posX1 = rect.right-310;
	int posX2 = rect.right-200;

	rect.left	= rect.right-315;
	rect.right	= rect.right-160;
	rect.top	= 55;
	rect.bottom = 310;
	*/

	rect.left= rect.right-470;
	rect.right	= rect.right-160;
	rect.top = 310;
	rect.bottom = 455;

	/*rect.left= rect.right-470;
	rect.right	= rect.right-185;
	rect.top = rect.bottom-237;
	rect.bottom = rect.bottom-22;*/

	HDC hdc = GetDC(this->m_hwnd);

	//FillRect(hdc,&rect,(HBRUSH)GetStockObject(0));

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	FillRect(hdc,&rect,this->m_brush[3]);

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+7,rect.top+2,"CONNECTION LOG",15);

	int line = MAX_LOGCONNECT_TEXT_LINE;

	int count = (((this->m_countConnect-1)>=0)?(this->m_countConnect-1):(MAX_LOGCONNECT_TEXT_LINE-1));

	for(int n=0;n < MAX_LOGCONNECT_TEXT_LINE;n++)
	{
		switch(this->m_logConnect[count].color)
		{
			case LOG_BLACK:
				SetTextColor(hdc,RGB(250,250,250));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_RED:
				SetTextColor(hdc,RGB(240,20,10));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_GREEN:
				SetTextColor(hdc,RGB(10,240,20));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BLUE:
				SetTextColor(hdc,RGB(10,120,240));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BOT:
				SetTextColor(hdc,RGB(250,10,60));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_USER:
				SetTextColor(hdc,RGB(250,150,40));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_EVENT:
				SetTextColor(hdc,RGB(10,100,200));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_ALERT:
				SetTextColor(hdc,RGB(150,20,70));
				SetBkMode(hdc,TRANSPARENT);
				break;
		}

		int size = strlen(this->m_logConnect[count].text);

		if(size > 1)
		{
			TextOut(hdc,rect.left+7,(rect.top+5+(line*15)),this->m_logConnect[count].text,size);
			line--;
		}

		count = (((--count)>=0)?count:(MAX_LOGCONNECT_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
#endif

#if(GAMESERVER_TYPE==1)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);
	/*
	int posX1 = rect.right-310;
	int posX2 = rect.right-200;

	rect.left	= rect.right-315;
	rect.right	= rect.right-160;
	rect.top	= 55;
	rect.bottom = 310;
	*/
	/*
	int posX1 = rect.right-462;
	int posX2 = rect.right-370;

	rect.left	= rect.right-470;
	rect.right	= rect.right-320;
	rect.top	= 55;
	rect.bottom = 305;
	*/

	rect.left= rect.right-470;
	rect.right	= rect.right-160;
	rect.top = 55;
	rect.bottom = 305;

	/*rect.left= rect.right-470;
	rect.right	= rect.right-185;
	rect.top = rect.bottom-237;
	rect.bottom = rect.bottom-22;*/

	HDC hdc = GetDC(this->m_hwnd);

	//FillRect(hdc,&rect,(HBRUSH)GetStockObject(0));

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	FillRect(hdc,&rect,this->m_brush[3]);

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+7,rect.top+2,"CONNECTION LOG",15);

	int line = MAX_LOGCONNECT_TEXT_LINE;

	int count = (((this->m_countConnect-1)>=0)?(this->m_countConnect-1):(MAX_LOGCONNECT_TEXT_LINE-1));

	for(int n=0;n < MAX_LOGCONNECT_TEXT_LINE;n++)
	{
		switch(this->m_logConnect[count].color)
		{
			case LOG_BLACK:
				SetTextColor(hdc,RGB(250,250,250));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_RED:
				SetTextColor(hdc,RGB(240,20,10));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_GREEN:
				SetTextColor(hdc,RGB(10,240,20));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BLUE:
				SetTextColor(hdc,RGB(10,120,240));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_BOT:
				SetTextColor(hdc,RGB(250,10,60));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_USER:
				SetTextColor(hdc,RGB(250,150,40));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_EVENT:
				SetTextColor(hdc,RGB(10,100,200));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_ALERT:
				SetTextColor(hdc,RGB(150,20,70));
				SetBkMode(hdc,TRANSPARENT);
				break;
		}

		int size = strlen(this->m_logConnect[count].text);

		if(size > 1)
		{
			TextOut(hdc,rect.left+7,(rect.top+5+(line*15)),this->m_logConnect[count].text,size);
			line--;
		}

		count = (((--count)>=0)?count:(MAX_LOGCONNECT_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
#endif
}

void CServerDisplayer::LogTextPaintGlobalMessage() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.left= rect.right-470;
	rect.right	= rect.right-5;
	rect.top = rect.bottom-230;
	rect.bottom = rect.bottom-25;

	HDC hdc = GetDC(this->m_hwnd);

	//FillRect(hdc,&rect,(HBRUSH)GetStockObject(0));

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	FillRect(hdc,&rect,this->m_brush[3]);

	SetTextColor(hdc,RGB(240,120,10));
	TextOut(hdc,rect.left+7,rect.top+2,"GLOBAL MESSAGE",15);

	int line = MAX_LOGGLOBAL_TEXT_LINE;

	int count = (((this->m_countGlobal-1)>=0)?(this->m_countGlobal-1):(MAX_LOGGLOBAL_TEXT_LINE-1));

	for(int n=0;n < MAX_LOGGLOBAL_TEXT_LINE;n++)
	{

		SetTextColor(hdc,RGB(20,180,40));

		int size = strlen(this->m_logGlobal[count].text);

		if(size > 1)
		{
			TextOut(hdc,rect.left+7,(rect.top+5+(line*15)),this->m_logGlobal[count].text,size);
			line--;
		}

		count = (((--count)>=0)?count:(MAX_LOGGLOBAL_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::LogAddText(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOG_TEXT_SIZE)?(MAX_LOG_TEXT_SIZE-1):size);

	memset(&this->m_log[this->m_count].text,0,sizeof(this->m_log[this->m_count].text));

	memcpy(&this->m_log[this->m_count].text,text,size);

	this->m_log[this->m_count].color = color;

	this->m_count = (((++this->m_count)>=MAX_LOG_TEXT_LINE)?0:this->m_count);

	PROTECT_FINAL

	gLog.Output(LOG_GENERAL,"%s",&text[9]);
}

void CServerDisplayer::LogAddTextConnect(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOGCONNECT_TEXT_SIZE)?(MAX_LOGCONNECT_TEXT_SIZE-1):size);

	memset(&this->m_logConnect[this->m_countConnect].text,0,sizeof(this->m_logConnect[this->m_countConnect].text));

	memcpy(&this->m_logConnect[this->m_countConnect].text,text,size);

	this->m_logConnect[this->m_countConnect].color = color;

	this->m_countConnect = (((++this->m_countConnect)>=MAX_LOGCONNECT_TEXT_LINE)?0:this->m_countConnect);

	PROTECT_FINAL

	gLog.Output(LOG_GENERAL,"%s",&text[9]);
}

void CServerDisplayer::LogAddTextGlobal(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOGGLOBAL_TEXT_SIZE)?(MAX_LOGGLOBAL_TEXT_SIZE-1):size);

	memset(&this->m_logGlobal[this->m_countGlobal].text,0,sizeof(this->m_logGlobal[this->m_countGlobal].text));

	memcpy(&this->m_logGlobal[this->m_countGlobal].text,text,size);

	this->m_logGlobal[this->m_countGlobal].color = color;

	this->m_countGlobal = (((++this->m_countGlobal)>=MAX_LOGGLOBAL_TEXT_LINE)?0:this->m_countGlobal);

	PROTECT_FINAL
}

void CServerDisplayer::Interface(HWND hWnd)
{
    HDC hDC = GetDC(this->m_hwnd);
    RECT rect;
    // ----
    GetClientRect(this->m_hwnd, &rect);
    FillRect(hDC, &rect, this->m_brush[4]);
    SetBkMode(hDC, TRANSPARENT);
    ReleaseDC(this->m_hwnd, hDC);
}