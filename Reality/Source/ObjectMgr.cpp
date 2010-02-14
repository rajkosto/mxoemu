// *************************************************************************************************
// --------------------------------------
// Copyright (C) 2006-2010 Rajko Stojadinovic
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

#include "ObjectMgr.h"
#include "PlayerObject.h"
#include "GameClient.h"

uint32 ObjectMgr::allocatePlayer( GameClient* requester, uint64 charUID )
{
	if (requester == NULL)
		throw ClientNotAvailable();

	PlayerObject *newPlayerObj = NULL;
	try
	{
		newPlayerObj = new PlayerObject(*requester,charUID);
	}
	catch (PlayerObject::CharacterNotFound)
	{
		throw ObjectNotAvailable();
	}

	uint32 theNewObjectId = getNewObjectId();
	newPlayerObj->initGoId(theNewObjectId);
	m_objects[theNewObjectId]=objectPtr(newPlayerObj);
	return theNewObjectId;
}

void ObjectMgr::deallocatePlayer( uint32 goId )
{
	//erase from valid objects
	for (;;)
	{
		objectsMap::iterator it=m_objects.find(goId);
		if (it==m_objects.end())
			break;

		it->second.reset();
		m_objects.erase(it);
	}

	//erase from object view maps (of all clients) and release object view
	for (clientToViewMap::iterator it1=m_views.begin();it1!=m_views.end();++it1)
	{
		for (viewIdsMap::iterator it2=it1->second.begin();it2!=it1->second.end();)
		{
			uint16 theViewId = it2->first;
			uint32 theGoId = it2->second;
			if (theGoId == goId)
			{
				it1->second.erase(it2++);
			}
			else
			{
				++it2;
			}
		}
	}
}

class PlayerObject* ObjectMgr::getGOPtr( uint32 goId )
{
	objectsMap::iterator it=m_objects.find(goId);
	if (it!=m_objects.end())
		return it->second.get();

	throw ObjectNotAvailable();
}

uint32 ObjectMgr::getGOId( class PlayerObject* forWhichObj )
{
	if (forWhichObj==NULL)
		throw ObjectNotAvailable();

	for (objectsMap::iterator it=m_objects.begin();it!=m_objects.end();++it)
	{
		if (it->second.get() == forWhichObj)
			return it->first;
	}

	throw ObjectNotAvailable();
}

uint16 ObjectMgr::getViewForGO( GameClient* requester, uint32 goId )
{
	if (requester==NULL)
		throw ClientNotAvailable();

	if (m_objects.find(goId)==m_objects.end())
		throw ObjectNotAvailable();

	//first time client asked for this
	if (m_views.find(requester)==m_views.end())
		m_views[requester] = viewIdsMap();

	viewIdsMap &viewsOfClient = m_views[requester];
	for (viewIdsMap::iterator it=viewsOfClient.begin();it!=viewsOfClient.end();++it)
	{
		if (it->second == goId)
			return it->first;
	}

	//otherwise allocate new viewID, put that in the views list with the goId, and return it
	uint16 newViewId = allocateViewId(requester);
	viewsOfClient[newViewId] = goId;

	return newViewId;
}

void ObjectMgr::clientSigningOff( GameClient *requester )
{
	for (;;)
	{
		clientToViewMap::iterator it=m_views.find(requester);
		if (it==m_views.end())
			return;

		m_views.erase(it);
	}
}

uint16 ObjectMgr::allocateViewId( GameClient* requester)
{
	if (requester == NULL)
		throw ClientNotAvailable();

	//get the views map for current client
	viewIdsMap &viewsOfClient = m_views[requester];

	//go through all possible viewIds, when we find one thats not in the list, return it
	for (uint16 i=2;i<0xFFFF;i++) //we start from 2 because 1 is the object manager id, it spawns and deletes objects
	{
		if (viewsOfClient.find(i)==viewsOfClient.end())
		{
			return i;
		}
	}
	throw NoMoreFreeViews();
}
