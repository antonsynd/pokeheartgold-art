#ifndef POKEHEARTGOLD_BILLBOARD_LISTS_H
#define POKEHEARTGOLD_BILLBOARD_LISTS_H

#include "constants/heap.h"

#include "billboard_internal.h"

void BillboardLists_Create(int count, enum HeapID heapID);
void BillboardLists_Delete(void);
void BillboardLists_Draw(void);
void Billboard_ClearPendingFree(Billboard *billboard);
void BillboardObject_SetVisible(BillboardObject *object, u8 visible);
NNSG3dResMdl *BillboardObject_GetModel(BillboardObject *object);

#endif // POKEHEARTGOLD_BILLBOARD_LISTS_H
