#ifndef OBJECTMGR_H
#define OBJECTMGR_H

#include "Common.h"

const uint32 OBJECTMANAGER_STARTINGOBJECTID = 1000000; //we have plenty of uint32s

class ObjectMgr
{
public:
	class ObjectNotAvailable {};
	class ClientNotAvailable {};
	class NoMoreFreeViews {};

	ObjectMgr():m_currFreeObjectId(OBJECTMANAGER_STARTINGOBJECTID) {}
	~ObjectMgr(){}

	uint32 allocatePlayer(class GameClient *requester, uint64 charUID );
	void deallocatePlayer(uint32 goId);
	class PlayerObject* getGOPtr(uint32 goId);
	uint32 getGOId(class PlayerObject* forWhichObj);
	uint16 getViewForGO(class GameClient *requester, uint32 goId);
	void clientSigningOff(class GameClient *requester);
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
};

#endif