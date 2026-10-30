#ifndef POKEHEARTGOLD_FIELD_EFFECTS_H
#define POKEHEARTGOLD_FIELD_EFFECTS_H

#include "field_types_def.h"

// The pool of running field effects. The FieldEffect and FieldEffectBasicTemplate types are in
// field/field_effect_manager.h, which includes this header.
typedef struct FieldEffectPool FieldEffectPool;

struct FieldEffect;
struct FieldEffectBasicTemplate;

FieldEffectPool *FieldEffectPool_New(enum HeapID heapID, int count);
void FieldEffectPool_Delete(FieldEffectPool *pool);
struct FieldEffect *FieldEffectPool_AddBasicEffect(FieldEffectPool *pool, const struct FieldEffectBasicTemplate *template, const VecFx32 *position, int parameter, void *userData, u32 priority);
void FieldEffect_Destroy(struct FieldEffect *effect);
void FieldEffectPool_DrawAll(FieldEffectPool *pool);
void *FieldEffect_GetUserData(struct FieldEffect *effect);
void FieldEffect_GetPosition(struct FieldEffect *effect, VecFx32 *position);
void FieldEffect_DefaultDraw(struct FieldEffect *effect, void *state);
void FieldEffect_DefaultDestroy(struct FieldEffect *effect, void *state);

#endif // POKEHEARTGOLD_FIELD_EFFECTS_H
