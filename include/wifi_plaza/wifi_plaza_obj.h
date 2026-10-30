#ifndef POKEHEARTGOLD_WIFI_PLAZA_OBJ_H
#define POKEHEARTGOLD_WIFI_PLAZA_OBJ_H

#include "global.h"

#include "wifi_plaza/wifi_plaza_map.h"

#include "overlay_42.h"

// Objects of the Wi-Fi Plaza room: a grid object (cell position, direction, queued moves) paired with a billboard
// avatar. Directions: 0 up, 1 down, 2 left, 3 right. Move commands: 1 turn, 2 walk, 3 run.

typedef struct WiFiPlazaObj WiFiPlazaObj;
typedef struct WiFiPlazaObjManager WiFiPlazaObjManager;

typedef void (*WiFiPlazaObjFunc)(WiFiPlazaObj *obj, WiFiPlazaObjManager *manager);

enum WiFiPlazaObjHandler {
    WIFI_PLAZA_OBJ_HANDLER_IDLE,
    WIFI_PLAZA_OBJ_HANDLER_PLAYER_INPUT,
    WIFI_PLAZA_OBJ_HANDLER_WANDER,
    WIFI_PLAZA_OBJ_HANDLER_RISE_OUT,
    WIFI_PLAZA_OBJ_HANDLER_DROP_IN,
    WIFI_PLAZA_OBJ_HANDLER_SLIDE_UP,
    WIFI_PLAZA_OBJ_HANDLER_SLIDE_DOWN,
    WIFI_PLAZA_OBJ_HANDLER_SLIDE_LEFT,
    WIFI_PLAZA_OBJ_HANDLER_SLIDE_RIGHT,
    WIFI_PLAZA_OBJ_HANDLER_HOP,
};

typedef struct WiFiPlazaObjTween {
    s32 value;
    s32 start;
    s32 delta;
    u32 duration;
} WiFiPlazaObjTween;

struct WiFiPlazaObj {
    Field2DObject *gridObj;
    struct WiFiPlazaPlayerObject *avatar;
    u16 step;
    u8 finished;
    u8 handler;

    union {
        u8 raw[0x18];

        struct {
            s16 timer;
            s16 direction;
        } wander;

        struct {
            WiFiPlazaObjTween tween;
            u32 frame;
        } lift;

        struct {
            WiFiPlazaObjTween tween;
            s32 frame;
            s32 start;
        } slide;

        struct {
            s32 frame;
            VecFx32 origin;
        } hop;
    } work;

    WiFiPlazaObjFunc update;
};

// Brightness of the avatars, faded over 60 frames when the room's light set changes
typedef struct WiFiPlazaObjLight {
    u8 active;
    u8 value;
    s8 start;
    s8 delta;
    s16 frame;
    s16 duration;
} WiFiPlazaObjLight;

struct WiFiPlazaObjManager {
    Field2DObjectManager *gridObjs;
    struct WiFiPlazaPlayerObjects *avatars;
    WiFiPlazaObj *objs;
    u16 count;
    u16 mode; // nonzero: own player avatar 0x61
    Field2DCommandQueue *commands;
    WiFiPlazaMap *map;
    WiFiPlazaObjLight light;
    WiFiPlazaObj *ownPlayer;
    WiFiPlazaObj *special; // object spawned on map tag 97
};

WiFiPlazaObjManager *WiFiPlazaObj_CreateManager(u32 count, BOOL mode, WiFiPlazaMap *map, enum HeapID heapID, enum HeapID loadHeapID);
void WiFiPlazaObj_DeleteManager(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_UpdateObjsAndLight(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_UpdateManager(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_DrawAvatars(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_ClearPendingFree(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_SetLightSet(WiFiPlazaObjManager *manager, int set);
void WiFiPlazaObj_FadeToLightSet(WiFiPlazaObjManager *manager, int set);
WiFiPlazaObj *WiFiPlazaObj_CreateOwnPlayer(WiFiPlazaObjManager *manager, int id);
WiFiPlazaObj *WiFiPlazaObj_CreateOwnPlayerAt(WiFiPlazaObjManager *manager, int id, int x, int y);
WiFiPlazaObj *WiFiPlazaObj_CreatePlayer(WiFiPlazaObjManager *manager, int id, int avatar);
void WiFiPlazaObj_Delete(WiFiPlazaObj *obj);
WiFiPlazaObj *WiFiPlazaObj_FindById(WiFiPlazaObjManager *manager, int id);
WiFiPlazaObj *WiFiPlazaObj_GetOwnPlayer(WiFiPlazaObjManager *manager);
WiFiPlazaObj *WiFiPlazaObj_GetSpecial(WiFiPlazaObjManager *manager);
void WiFiPlazaObj_SetGridPos(WiFiPlazaObj *obj, Field2DPoint pos);
void WiFiPlazaObj_SetGridPosAndDir(WiFiPlazaObj *obj, Field2DPoint pos, s32 direction);
Field2DPoint WiFiPlazaObj_GetGridPos(WiFiPlazaObj *obj);
s32 WiFiPlazaObj_GetParam(WiFiPlazaObj *obj, enum Field2DObjectParam param);
void WiFiPlazaObj_QueueCommand(WiFiPlazaObjManager *manager, WiFiPlazaObj *obj, s32 command, s32 direction);
void WiFiPlazaObj_RunCommand(WiFiPlazaObjManager *manager, WiFiPlazaObj *obj, u16 value, u32 kind);
void WiFiPlazaObj_SetHandler(WiFiPlazaObjManager *manager, WiFiPlazaObj *obj, int handler);
u8 WiFiPlazaObj_IsFinished(WiFiPlazaObj *obj);
u8 WiFiPlazaObj_GetHandler(WiFiPlazaObj *obj);
WiFiPlazaObj *WiFiPlazaObj_FindFacing(WiFiPlazaObjManager *manager, WiFiPlazaObj *obj);
BOOL WiFiPlazaObj_IsCulled(WiFiPlazaObj *obj);
WiFiPlazaObj *WiFiPlazaObj_FindAt(WiFiPlazaObjManager *manager, s32 x, s32 y);
BOOL WiFiPlazaObj_IsOnCell(WiFiPlazaObj *obj, s32 x, s32 y);
BOOL WiFiPlazaObj_FindFreeNeighbor(WiFiPlazaObjManager *manager, WiFiPlazaObj *obj, s32 *direction, Field2DPoint *position);
void WiFiPlazaObj_SetAvatarMode(WiFiPlazaObj *obj, int enable);
void WiFiPlazaObj_SetPositionFromGrid(WiFiPlazaObj *obj, const Field2DPoint *gridPos);
void WiFiPlazaObj_SetPosition(WiFiPlazaObj *obj, VecFx32 *position);
void WiFiPlazaObj_GetPosition(WiFiPlazaObj *obj, VecFx32 *position);
void WiFiPlazaObj_SetDirection(WiFiPlazaObj *obj, u32 direction);
void WiFiPlazaObj_SetPoseEffect(WiFiPlazaObj *obj, u32 enable);
void WiFiPlazaObj_SetSpinEffect(WiFiPlazaObj *obj, u32 enable);
void WiFiPlazaObj_SetPoseLoopEffect(WiFiPlazaObj *obj, u32 enable);
void WiFiPlazaObj_SetLightFlags(WiFiPlazaObj *obj, u32 value);
void WiFiPlazaObj_SetVisible(WiFiPlazaObj *obj, u32 value);
BOOL WiFiPlazaObj_IsVisible(WiFiPlazaObj *obj);
BOOL WiFiPlazaObj_UpdateLight(WiFiPlazaObjLight *light, struct WiFiPlazaPlayerObjects *avatars);

// Avatar manager
struct WiFiPlazaPlayerObjects *WiFiPlazaPlayerObjects_New(u32 count, BOOL mode, enum HeapID heapID, enum HeapID loadHeapID);
void WiFiPlazaPlayerObjects_Delete(struct WiFiPlazaPlayerObjects *objects);
void WiFiPlazaPlayerObjects_Update(struct WiFiPlazaPlayerObjects *objects);
void WiFiPlazaPlayerObjects_Draw(struct WiFiPlazaPlayerObjects *objects);
void WiFiPlazaPlayerObjects_ClearPendingFree(struct WiFiPlazaPlayerObjects *objects);
void WiFiPlazaPlayerObjects_SetModelAlpha(struct WiFiPlazaPlayerObjects *objects, int alpha);
struct WiFiPlazaPlayerObject *WiFiPlazaPlayerObjects_Add(struct WiFiPlazaPlayerObjects *objects, Field2DObject *player);
void WiFiPlazaPlayerObject_Delete(struct WiFiPlazaPlayerObject *object);
void WiFiPlazaPlayerObject_SetMode(struct WiFiPlazaPlayerObject *object, u32 mode);
void WiFiPlazaPlayerObject_SetLightFlags(struct WiFiPlazaPlayerObject *object, u32 lightFlags);
u32 WiFiPlazaPlayerObject_IsCulled(struct WiFiPlazaPlayerObject *object);
void WiFiPlazaPlayerObject_SetGridPosition(struct WiFiPlazaPlayerObject *object, const Field2DPoint *gridPos);
void WiFiPlazaPlayerObject_SetPosition(struct WiFiPlazaPlayerObject *object, const VecFx32 *position);
void WiFiPlazaPlayerObject_GetPosition(struct WiFiPlazaPlayerObject *object, VecFx32 *position);
void WiFiPlazaPlayerObject_SetStandingSequence(struct WiFiPlazaPlayerObject *object, u32 sequence);
void WiFiPlazaPlayerObject_StartEffect(struct WiFiPlazaPlayerObject *object, u32 effect);
void WiFiPlazaPlayerObject_StopEffect(struct WiFiPlazaPlayerObject *object);
void WiFiPlazaPlayerObject_SetEffectStep(struct WiFiPlazaPlayerObject *object, u8 step);
void WiFiPlazaPlayerObject_SetVisible(struct WiFiPlazaPlayerObject *object, int visible);
u32 WiFiPlazaPlayerObject_IsVisible(struct WiFiPlazaPlayerObject *object);

#endif // POKEHEARTGOLD_WIFI_PLAZA_OBJ_H
