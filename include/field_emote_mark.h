#ifndef POKEHEARTGOLD_FIELD_EMOTE_MARK_H
#define POKEHEARTGOLD_FIELD_EMOTE_MARK_H

#include "player_avatar.h"

struct FieldEffect;

struct FieldEffect *EmoteMark_Spawn(LocalMapObject *obj, int kind, BOOL playSound);
BOOL EmoteMark_IsDone(struct FieldEffect *effect);
void EmoteMark_Hide(struct FieldEffect *effect);

#endif // POKEHEARTGOLD_FIELD_EMOTE_MARK_H
