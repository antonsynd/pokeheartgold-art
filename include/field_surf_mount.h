#ifndef POKEHEARTGOLD_FIELD_SURF_MOUNT_H
#define POKEHEARTGOLD_FIELD_SURF_MOUNT_H

#include "map_object_manager.h"

struct FieldEffect;

struct FieldEffect *SurfMountEffect_Create(LocalMapObject *obj, u32 x, u32 y, u32 dir, BOOL bobbing);

#endif // POKEHEARTGOLD_FIELD_SURF_MOUNT_H
