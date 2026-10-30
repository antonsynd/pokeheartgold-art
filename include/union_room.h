#ifndef POKEHEARTGOLD_UNION_ROOM_H
#define POKEHEARTGOLD_UNION_ROOM_H

#include "script.h"

void Colosseum_ShowOpponentTrainerCard(FieldSystem *fieldSystem);
struct UnionRoom *UnionRoom_Start(FieldSystem *fieldSystem);
void UnionRoom_Stop(FieldSystem *fieldSystem);
int UnionRoom_GetPlayerActivityType(struct UnionRoom *fieldSystem_unk80, int objId);
int UnionRoom_RequestJoinPlayer(struct UnionRoom *fieldSystem_unk80, int objId, int kind);
u32 UnionRoom_GetJoinStatus(struct UnionRoom *fieldSystem_unk80);
int UnionRoom_GetTalkChoice(struct UnionRoom *fieldSystem_unk80);
void UnionRoom_SendTalkResponse(struct UnionRoom *fieldSystem_unk80, int mode, int value);
int UnionRoom_GetTalkReply(struct UnionRoom *fieldSystem_unk80);
int UnionRoom_TrySendTalkCancel(struct UnionRoom *fieldSystem_unk80, int ret);
void UnionRoom_RecvUnusedProfile(void);
void UnionRoom_RecvTalkReply(int netId, int size, const u8 *data, FieldSystem *fieldSystem);
void UnionRoom_RecvUnused(void);
void UnionRoom_RecvRestartSearch(int netId, int size, void *data, FieldSystem *fieldSystem);
void Colosseum_RecvMixBattleMons(int netId, int size, void *data, FieldSystem *fieldSystem);
u32 Colosseum_GetMixBattleMonsSize(void);
void Colosseum_GetMixBattleMonsBuffer(void);
void Colosseum_RecvMixBattleChoice(int netId, int size, const u8 *data, FieldSystem *fieldSystem);

#endif // POKEHEARTGOLD_UNION_ROOM_H
