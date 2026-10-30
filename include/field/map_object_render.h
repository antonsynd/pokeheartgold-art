#ifndef POKEHEARTGOLD_FIELD_MAP_OBJECT_RENDER_H
#define POKEHEARTGOLD_FIELD_MAP_OBJECT_RENDER_H

#include "billboard_internal.h"
#include "map_object.h"

// The billboard renderer of the map objects: the resource caches behind every map object's billboard and the
// queue that loads them over several frames. It lives inside MapObjectManager (MapObjectManager_GetRender).
typedef struct MapObjectRender MapObjectRender;

// What BillboardObject_SaveState keeps of a billboard while its map object is paused
typedef struct BillboardState {
    u32 visible;
    u16 unused4;
    u16 sequence;
    fx32 frame;
} BillboardState;

void MapObjectRender_ReleaseUnused(MapObjectManager *manager, MapObjectRender *render);
Billboard *MapObjectRender_GetBillboards(MapObjectRender *render);
BOOL MapObject_IsBillboardPending(LocalMapObject *object);
void MapObject_ChangeBillboardSprite(LocalMapObject *object, u32 spriteId);
// Returns 0; the callers ignore it.
int MapObject_UpdateBillboardPosition(LocalMapObject *object, BillboardObject *billboard);
void MapObjectRender_ReleaseSpriteResources(MapObjectManager *manager, u32 spriteId, LocalMapObject *skip);
int MapObject_RequestBillboard(LocalMapObject *object, BillboardObject **billboard, u32 spriteId);
void MapObject_CreateBillboard(LocalMapObject *object, BillboardObject **billboard);
void MapObject_CancelAndFreeBillboard(LocalMapObject *object, BillboardObject **billboard, u32 spriteId);
void MapObject_SetBillboardVisibleFromFlags(LocalMapObject *object, BillboardObject *billboard);
void MapObject_DeleteBillboard(LocalMapObject *object, BillboardObject **billboard);
u32 Facing_GetBillboardStandSequence(u32 facing);
u32 Facing_GetBillboardWalkSequence(u32 facing);
u32 MapObjectRender_GetFacingSequence(u32 facing);

void MapObjectRender_Init(MapObjectRender *render, MapObjectManager *manager, u32 count, u32 priority, u32 textureCount, u32 residentTextureCount, const u32 *textureIds, u32 loadsPerFrame);
void MapObjectRender_Free(MapObjectRender *render);
u8 MapObjectRender_GetSpriteModelId(u32 spriteId);
void MapObject_SetBillboardPending(LocalMapObject *object, BOOL set);
void BillboardObject_SaveState(BillboardObject *billboard, BillboardState *state);
void BillboardObject_RestoreState(BillboardObject *billboard, BillboardState *state);

#endif // POKEHEARTGOLD_FIELD_MAP_OBJECT_RENDER_H
