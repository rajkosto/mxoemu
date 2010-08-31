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

#ifndef MXOEMU_PLAYEROBJECT_H
#define MXOEMU_PLAYEROBJECT_H

#include "LocationVector.h"
#include "MessageTypes.h"

class PlayerObject
{
public:
	class CharacterNotFound {};

	PlayerObject(class GameClient &parent,uint64 charUID);
	~PlayerObject();

	void InitializeWorld();
	void SpawnSelf();
	void PopulateWorld();

	void initGoId(uint32 theGoId);
	void HandleStateUpdate(ByteBuffer &srcData);
	void HandleCommand(ByteBuffer &srcCmd);

	string getHandle() const {return m_handle;}
	string getFirstName() const {return m_firstName;}
	string getLastName() const {return m_lastName;}
	string getBackground() const {return m_background;}
	bool setBackground(string newBackground);

	uint64 getExperience() const {return m_exp;}
	uint64 getInformation() const {return m_cash;}
	LocationVector getPosition() const {return m_pos;}
	void setPosition(const LocationVector& newPos) {m_pos = newPos;}
	uint8 getDistrict() const {return m_district;}
	void setDistrict(uint8 newDistrict) {m_district = newDistrict;}
	uint8 getRsiData(byte* outputBuf,size_t maxBufLen) const;
	uint16 getCurrentHealth() const {return m_healthC;}
	uint16 getMaximumHealth() const {return m_healthM;}
	uint16 getCurrentIS() const {return m_innerStrC;}
	uint16 getMaximumIS() const {return m_innerStrM;}
	uint32 getProfession() const {return m_prof;}
	uint8 getLevel() const {return m_lvl;}
	uint8 getAlignment() const {return m_alignment;}
	bool getPvpFlag() const {return m_pvpflag;}

	uint8 getCurrentAnimation() const {return m_currAnimation;}
	uint8 getCurrentMood() const {return m_currMood;}

	class GameClient& getClient() { return m_parent; }
	vector<msgBaseClassPtr> getCurrentStatePackets();

	void Update();
private: 
	//RPC handler type
	typedef void (PlayerObject::*RPCHandler)( ByteBuffer &srcCmd );

	//RPC handlers
	void RPC_NullHandle(ByteBuffer &srcCmd);
	void RPC_HandleReadyForSpawn(ByteBuffer &srcCmd);
	void RPC_HandleChat( ByteBuffer &srcCmd );
	void RPC_HandleWhisper( ByteBuffer &srcCmd );
	void RPC_HandleStopAnimation( ByteBuffer &srcCmd );
	void RPC_HandleStartAnimtion( ByteBuffer &srcCmd );
	void RPC_HandleChangeMood( ByteBuffer &srcCmd );
	void RPC_HandlePerformEmote( ByteBuffer &srcCmd );
	void RPC_HandleDynamicObjInteraction( ByteBuffer &srcCmd );
	void RPC_HandleStaticObjInteraction( ByteBuffer &srcCmd );
	void RPC_HandleJump( ByteBuffer &srcCmd );
	void RPC_HandleRegionLoadedNotification( ByteBuffer &srcCmd );
	void RPC_HandleReadyForWorldChange( ByteBuffer &srcCmd );
	void RPC_HandleWho( ByteBuffer &srcCmd );
	void RPC_HandleWhereAmI( ByteBuffer &srcCmd );
	void RPC_HandleGetPlayerDetails( ByteBuffer &srcCmd );
	void RPC_HandleGetBackground( ByteBuffer &srcCmd );
	void RPC_HandleSetBackground( ByteBuffer &srcCmd );
	void RPC_HandleHardlineTeleport( ByteBuffer &srcCmd );
	void RPC_HandleObjectSelected( ByteBuffer &srcCmd );
	void RPC_HandleJackoutRequest( ByteBuffer &srcCmd );
	void RPC_HandleJackoutFinished( ByteBuffer &srcCmd );

	//RPC Handler maps
	map<uint8,RPCHandler> m_RPCbyte;
	map<uint16,RPCHandler> m_RPCshort;
private:
	void loadFromDB(bool updatePos=false);
	void checkAndStore();
	void saveDataToDB();
	void setOnlineStatus( bool isOnline );

	typedef enum
	{
		EVENT_JACKOUT
	} eventType;

	typedef boost::function < void (void) > eventFunc;

	void addEvent(eventType type, eventFunc func, float activationTime);
	size_t cancelEvents(eventType type);

	struct eventStruct
	{
		eventStruct(eventType _type, eventFunc _func, float _fireTime) : type(_type), func(_func), fireTime(_fireTime) {}
		eventType type;
		eventFunc func;
		float fireTime;
	};

	list<eventStruct> m_events;

	void jackoutEvent();

	void ParseAdminCommand(string theCmd);
	void ParsePlayerCommand(string theCmd);
	void GoAhead(double distanceToGo);
	void UpdateAppearance();
	class GameClient &m_parent;
	
	//Player info
	uint64 m_characterUID;
	string m_handle;
	string m_firstName;
	string m_lastName;
	string m_background;

	uint32 m_goId;
	uint64 m_exp,m_cash;
	uint8 m_district;
	LocationVector m_pos,m_savedPos;
	shared_ptr<class RsiData> m_rsi;
	uint16 m_healthC,m_healthM,m_innerStrC,m_innerStrM;
	uint32 m_prof;
	uint8 m_lvl,m_alignment;
	bool m_pvpflag;
	uint32 testCount;

	bool m_spawnedInWorld;
	queue<msgBaseClassPtr> m_sendAfterSpawn;
	bool m_worldPopulated;

	uint32 m_lastStore;
	uint32 m_storeCntr;

	uint8 m_currAnimation;
	uint8 m_currMood;

	uint8 m_emoteCounter;

	bool m_isAdmin;
};

#endif