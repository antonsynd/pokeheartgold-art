#ifndef POKEHEARTGOLD_FRIEND_ROSTER_H
#define POKEHEARTGOLD_FRIEND_ROSTER_H

#include "constants/friend_roster.h"

#include "pm_string.h"
#include "save.h"

#define FRIEND_ROSTER_MAX 32

typedef struct FriendRoster FriendRoster;

void Save_FriendRoster_LoadFromSave(SaveData *saveData);
void Save_FriendRoster_StoreToSave(SaveData *saveData);
FriendRoster *Save_FriendRoster_Get(SaveData *saveData);
int FriendRoster_CountFriends(FriendRoster *data);
void *FriendRoster_GetUserData(FriendRoster *data);
u16 *FriendRoster_GetPlayerName(FriendRoster *data, s32 index);
u16 *FriendRoster_GetGroupName(FriendRoster *data, s32 index);
BOOL FriendRoster_IsFriendValid(FriendRoster *data, int index);
s32 FriendRoster_GetEntryField(FriendRoster *data, s32 index, s32 field);
void FriendRoster_Compact(FriendRoster *data);
void FriendRoster_UpdateDate(FriendRoster *data, s32 index);
void FriendRoster_AddWinsLossesTrades(FriendRoster *data, s32 index, s32 wins, s32 losses, s32 trades);
void FriendRoster_AddPlazaGame0Plays(FriendRoster *data, s32 index, s32 value);
void FriendRoster_AddPlazaGame1Plays(FriendRoster *data, s32 index, s32 value);
void FriendRoster_AddPlazaGame2Plays(FriendRoster *data, s32 index, s32 value);
int FriendRoster_GetUsedSlotCount(FriendRoster *data);
void FriendRoster_RemoveFriend(FriendRoster *data, int index);
void FriendRoster_SetEntryField(FriendRoster *data, int index, enum FriendRosterField field, u8 value);
void Save_FriendRoster_Init(FriendRoster *data);
u32 Save_FriendRoster_sizeof(void);
void *FriendRoster_GetDwcFriendData(FriendRoster *data, int index);
void FriendRoster_SetPlayerName(FriendRoster *data, int index, String *name);
void FriendRoster_SetGroupName(FriendRoster *data, int index, String *name);
void FriendRoster_MergeFriend(FriendRoster *data, s32 src, s32 dest);

#endif // POKEHEARTGOLD_FRIEND_ROSTER_H
