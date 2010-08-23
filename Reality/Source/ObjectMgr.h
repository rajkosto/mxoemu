// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

#ifndef MXOEMU_OBJECTMGR_H
#define MXOEMU_OBJECTMGR_H

#include "Common.h"
#include "MessageTypes.h"

const uint32 OBJECTMANAGER_STARTINGOBJECTID = 0x8000; //we have plenty of uint32s

class ObjectMgr
{
public:
	class ObjectNotAvailable {};
	class ClientNotAvailable {};
	class NoMoreFreeViews {};

	ObjectMgr():m_currFreeObjectId(OBJECTMANAGER_STARTINGOBJECTID) {}
	~ObjectMgr(){}

	uint32 constructPlayer(class GameClient *requester, uint64 charUID );
	void destroyObject(uint32 goId);
	class PlayerObject* getGOPtr(uint32 goId);
	uint32 getGOId(class PlayerObject* forWhichObj);
	uint16 getViewForGO(class GameClient *requester, uint32 goId);
	void releaseRelevantSet(class GameClient *requester);
	vector<uint32> getAllGOIds()
	{
		vector<uint32> tempVect;
		for (objectsMap::iterator it=m_objects.begin();it!=m_objects.end();++it)
		{
			if (it->first >= OBJECTMANAGER_STARTINGOBJECTID && it->second != NULL)
				tempVect.push_back(it->first);
		}
		return tempVect;
	}
	void OpenDoor(uint32 doorId, class GameClient *requester);
	vector<msgBaseClassPtr> GetAllOpenDoors(class GameClient *requester);

	void RandomObject( uint32 randomObjectId, GameClient* requester, double X, double Y, double Z, double ROT);
private:
	typedef shared_ptr<PlayerObject> objectPtr;
	typedef map<uint32,objectPtr> objectsMap;
	objectsMap m_objects;
	typedef map<uint16,uint32> viewIdsMap;
	typedef map<class GameClient*,viewIdsMap> clientToViewMap;
	clientToViewMap m_views;

	uint16 allocateViewId(class GameClient* requester);

	uint32 getNewObjectId()
	{
		uint32 theObjId = m_currFreeObjectId;
		m_currFreeObjectId++;
		return theObjId;
	}
	uint32 m_currFreeObjectId;

	map<uint16,uint32> m_openDoors;
};

#endif