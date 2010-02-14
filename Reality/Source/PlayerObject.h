#ifndef PLAYEROBJECT_H
#define PLAYEROBJECT_H

#include "LocationVector.h"

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

	uint64 getExperience() const {return m_exp;}
	uint64 getInformation() const {return m_cash;}
	LocationVector getPosition() const {return m_pos;}
	uint8 getRsiData(byte* outputBuf,uint32 maxBufLen) const ;
	uint16 getCurrentHealth() const {return m_healthC;}
	uint16 getMaximumHealth() const {return m_healthM;}
	uint16 getCurrentIS() const {return m_innerStrC;}
	uint16 getMaximumIS() const {return m_innerStrM;}
	uint8 getProfession() const {return m_prof;}
	uint8 getLevel() const {return m_lvl;}
	uint8 getAlignment() const {return m_alignment;}
	bool getPvpFlag() const {return m_pvpflag;}

	vector<class MsgBaseClass*> getCurrentStatePackets();
private:
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
	LocationVector m_pos;
	shared_ptr<class RsiData> m_rsi;
	uint16 m_healthC,m_healthM,m_innerStrC,m_innerStrM;
	uint8 m_prof,m_lvl,m_alignment;
	bool m_pvpflag;
	uint32 testCount;

	bool m_spawnedInWorld;
};

#endif