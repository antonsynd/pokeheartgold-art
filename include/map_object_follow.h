#ifndef POKEHEARTGOLD_MAP_OBJECT_FOLLOW_H
#define POKEHEARTGOLD_MAP_OBJECT_FOLLOW_H

#include "map_object.h"

struct FieldEffect;

void FollowPlayerMovement_Reset(LocalMapObject *object);
LocalMapObject *MapObject_FindTrainerPartner(LocalMapObject *object);
int HiddenObjectMovement_SetCoverEffect(LocalMapObject *object, struct FieldEffect *effect);

#endif // POKEHEARTGOLD_MAP_OBJECT_FOLLOW_H
