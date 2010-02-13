#ifndef OBJECTMGR_H
#define OBJECTMGR_H

#include "Common.h"

class ObjectMgr
{
public:
	class ObjectNotAvailable {};
	class ClientNotAvailable {};
	class NoMoreFreeViews {};

	ObjectMgr():m_currFreeObjectId(1000*1000) {}
	~ObjectMgr(){}

	uint32 allocatePlayer(class GameClient *requester, uint64 charUID );
	void deallocatePlayer(uint32 goId);
	class PlayerObject* getGOPtr(uint32 goId);
	uint32 getGOId(class PlayerObject* forWhichObj);
	uint16 getViewForGO(class GameClient *requester, uint32 goId);
	void clientSigningOff(class GameClient *requester);
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