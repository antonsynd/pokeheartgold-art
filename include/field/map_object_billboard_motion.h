#ifndef POKEHEARTGOLD_FIELD_MAP_OBJECT_BILLBOARD_MOTION_H
#define POKEHEARTGOLD_FIELD_MAP_OBJECT_BILLBOARD_MOTION_H

#include "map_object.h"

struct BillboardObject;

struct BillboardObject *MapObject_GetBillboard(LocalMapObject *object);
void FollowMonDraw_UpdatePositionOnly(LocalMapObject *object);
struct BillboardObject *MapObjectManager_GetFollowMonBillboard(MapObjectManager *manager);

void BillboardDraw_Init(LocalMapObject *object);
void BillboardDraw_Delete(LocalMapObject *object);
void BillboardDraw_Pause(LocalMapObject *object);
void BillboardDraw_Resume(LocalMapObject *object);
void FollowMonDraw_Init(LocalMapObject *object);
void FollowMonDraw_Delete(LocalMapObject *object);
void FollowMonDraw_Pause(LocalMapObject *object);
void FollowMonDraw_Resume(LocalMapObject *object);
void FollowMonDraw_ClearBob(LocalMapObject *object);
void BillboardDraw_UpdateNpc(LocalMapObject *object);
void BillboardDraw_UpdatePositionOnly(LocalMapObject *object);
void BillboardDraw_UpdateLooping(LocalMapObject *object);
void FollowMonDraw_UpdateLarge(LocalMapObject *object);
void FollowMonDraw_Update(LocalMapObject *object);
void BillboardDraw_UpdatePcWoman(LocalMapObject *object);
void BillboardDraw_UpdateTwoSequences(LocalMapObject *object);
void PlayerDraw_Init(LocalMapObject *object);
void PlayerDraw_Delete(LocalMapObject *object);
void PlayerDraw_Pause(LocalMapObject *object);
void PlayerDraw_Resume(LocalMapObject *object);
void PlayerDraw_UpdateWalking(LocalMapObject *object);
void PlayerDraw_UpdateCycling(LocalMapObject *object);
void PlayerDraw_UpdateFishing(LocalMapObject *object);
void PlayerDraw_UpdateLadder(LocalMapObject *object);
void PlayerDraw_UpdateSurfing(LocalMapObject *object);
void PlayerDraw_UpdateUseHmOrSaving(LocalMapObject *object);
void PlayerDraw_UpdateWatering(LocalMapObject *object);
void PlayerDraw_UpdatePoketch(LocalMapObject *object);
void PlayerDraw_UpdateHeal(LocalMapObject *object);
void PlayerDraw_UpdateApricornShake(LocalMapObject *object);
void ModelObjectDraw_Init(LocalMapObject *object);
void ModelObjectDraw_Update(void);
void ModelObjectDraw_Delete(LocalMapObject *object);
void ModelObjectDraw_Pause(LocalMapObject *object);
void ModelObjectDraw_Resume(LocalMapObject *object);

#endif // POKEHEARTGOLD_FIELD_MAP_OBJECT_BILLBOARD_MOTION_H
