#pragma once

#include "Map.h"

class cDragonMaps
{
public:
	//Dragones m�todos	
	cDragonMaps();
	virtual ~cDragonMaps();
	void Init();
	void DeleteFlyingDragons();
	void AddFlyingDragons(int mapa, int invasionTime, int index);
	void FlyingDragonsBossDieProc(int mapa);
	void FlyingDragonsCheck(int mapa, int index);
private:
	//Dragones variables de uso
	struct Maps
	{
		int Ok;
		time_t EndTime;
		int index;
	};

	Maps mapasDragones[MAX_MAP];
};

extern cDragonMaps gDragonMaps;