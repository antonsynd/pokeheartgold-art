#include "global.h"

#include "frontier/frontier.h"
#include "frontier/frontier_map.h"
#include "frontier/frontier_system.h"

#include "assert.h"
#include "heap.h"
#include "poke_overlay.h"

FS_EXTERN_OVERLAY(OVY_42);
FS_EXTERN_OVERLAY(OVY_80);
FS_EXTERN_OVERLAY(OVY_81);

struct FrontierWork {
    FrontierLaunchArgs *launchArgs;
    OverlayManager *childManager;
    void *childArgs;
    void (*childExit)(void *);
    u8 freeChildArgs;
    u8 pad_011[3];
    FrontierSystem *system;
    FrontierMap *map;
    u8 pad_01c;
    u8 mapActive;
    u8 rebuildPending;
    u8 pad_01f;
    u16 taskId;
    u8 exitRequested;
    u8 pad_023;
    FrontierMapObjectGfx objectGfx[FRONTIER_MAP_OBJECT_GFX_MAX];
    FrontierMapObject objects[FRONTIER_MAP_OBJECT_MAX];
    u8 padding_884[0x200];
    FrontierMapSpriteState spriteState;
};

static BOOL Frontier_Init(OverlayManager *manager, int *state);
static BOOL Frontier_Main(OverlayManager *manager, int *state);
static BOOL Frontier_Exit(OverlayManager *manager, int *state);
static void Frontier_CreateMap(FrontierWork *work);
static void Frontier_FreeMap(FrontierWork *work);
static void Frontier_ResetMapObjects(FrontierWork *work);
static void Frontier_LoadOverlays(void);
static void Frontier_UnloadOverlays(void);

const OverlayManagerTemplate gOverlayTemplate_Frontier = { Frontier_Init, Frontier_Main, Frontier_Exit, FS_OVERLAY_ID_NONE };

static BOOL Frontier_Init(OverlayManager *manager, int *state) {
    FrontierWork *work;
    Frontier_LoadOverlays();
    work = OverlayManager_CreateAndGetData(manager, sizeof(FrontierWork), HEAP_ID_FIELD2);
    MI_CpuFill8(work, 0, sizeof(FrontierWork));
    Frontier_ResetMapObjects(work);
    Frontier_ResetMapSpriteState(work);
    work->launchArgs = OverlayManager_GetArgs(manager);
    GF_ASSERT(work->launchArgs != NULL);
    work->system = FrontierSystem_Create(work, HEAP_ID_FIELD2, work->launchArgs->frontierMapId);
    FrontierSystem_AddTask(work->system, work->launchArgs->frontierMapId, 0);
    Frontier_CreateMap(work);
    return TRUE;
}

enum {
    FRONTIER_MAIN_INIT,
    FRONTIER_MAIN_RUN,
    FRONTIER_MAIN_EXIT,
    FRONTIER_MAIN_START_CHILD_APP,
    FRONTIER_MAIN_RUN_CHILD_APP,
    FRONTIER_MAIN_FREE_MAP,
    FRONTIER_MAIN_REBUILD_MAP,
};

static BOOL Frontier_Main(OverlayManager *manager, int *state) {
    FrontierWork *work = OverlayManager_GetData(manager);
    switch (*state) {
    case FRONTIER_MAIN_INIT:
        *state = FRONTIER_MAIN_RUN;
        break;
    case FRONTIER_MAIN_RUN:
        if (work->exitRequested == TRUE) {
            *state = FRONTIER_MAIN_EXIT;
            break;
        }
        if (!work->mapActive) {
            break;
        }
        if (work->rebuildPending == TRUE) {
            *state = FRONTIER_MAIN_FREE_MAP;
            break;
        }
        FrontierSystem_Main(work->system);
        if (work->childManager != NULL) {
            *state = FRONTIER_MAIN_START_CHILD_APP;
        }
        break;
    case FRONTIER_MAIN_EXIT:
        return TRUE;
    case FRONTIER_MAIN_START_CHILD_APP:
        FrontierMap_SaveObjects(work->map);
        Frontier_FreeMap(work);
        Frontier_UnloadOverlays();
        *state = FRONTIER_MAIN_RUN_CHILD_APP;
        break;
    case FRONTIER_MAIN_RUN_CHILD_APP:
        if (OverlayManager_Run(work->childManager) == TRUE) {
            OverlayManager_Delete(work->childManager);
            Frontier_LoadOverlays();
            if (work->childExit != NULL) {
                work->childExit(work->childArgs);
            }
            if (work->childArgs != NULL && work->freeChildArgs == TRUE) {
                Heap_Free(work->childArgs);
            }
            work->childManager = NULL;
            work->childExit = NULL;
            work->childArgs = NULL;
            Frontier_CreateMap(work);
            FrontierMap_RestoreObjects(work->map);
            *state = FRONTIER_MAIN_RUN;
        }
        break;
    case FRONTIER_MAIN_FREE_MAP:
        Frontier_FreeMap(work);
        Frontier_ResetMapObjects(work);
        *state = FRONTIER_MAIN_REBUILD_MAP;
        break;
    case FRONTIER_MAIN_REBUILD_MAP:
        Frontier_CreateMap(work);
        if (work->taskId == 0xffff) {
            FrontierSystem_ChangeMsgBank(work->system, work->launchArgs->frontierMapId, HEAP_ID_FIELD2);
        } else {
            u16 *saved = FrontierSystem_SaveVars(work->system, HEAP_ID_FIELD2);
            FrontierSystem_Delete(work->system);
            work->system = FrontierSystem_Create(work, HEAP_ID_FIELD2, work->launchArgs->frontierMapId);
            FrontierSystem_AddTask(work->system, work->launchArgs->frontierMapId, work->taskId);
            FrontierSystem_RestoreVars(work->system, saved);
        }
        work->rebuildPending = FALSE;
        *state = FRONTIER_MAIN_RUN;
        break;
    }
    return FALSE;
}

static BOOL Frontier_Exit(OverlayManager *manager, int *state) {
    FrontierWork *work = OverlayManager_GetData(manager);
    FrontierSystem_Delete(work->system);
    Frontier_FreeMap(work);
    OverlayManager_FreeData(manager);
    Frontier_UnloadOverlays();
    return TRUE;
}

static void Frontier_CreateMap(FrontierWork *work) {
    work->map = FrontierMap_Init(work);
    work->mapActive = TRUE;
}

static void Frontier_FreeMap(FrontierWork *work) {
    FrontierMap_Free(work->map);
    work->mapActive = FALSE;
}

static void Frontier_ResetMapObjects(FrontierWork *work) {
    int i;
    for (i = 0; i < FRONTIER_MAP_OBJECT_GFX_MAX; i++) {
        work->objectGfx[i].id = FRONTIER_MAP_NO_ID;
    }
    MI_CpuFill8(work->objects, 0, sizeof(work->objects));
    for (i = 0; i < FRONTIER_MAP_OBJECT_MAX; i++) {
        work->objects[i].template.id = FRONTIER_MAP_NO_ID;
    }
}

static void Frontier_LoadOverlays(void) {
    HandleLoadOverlay(FS_OVERLAY_ID(OVY_80), OVY_LOAD_ASYNC);
    HandleLoadOverlay(FS_OVERLAY_ID(OVY_81), OVY_LOAD_ASYNC);
    HandleLoadOverlay(FS_OVERLAY_ID(OVY_42), OVY_LOAD_ASYNC);
}

static void Frontier_UnloadOverlays(void) {
    UnloadOverlayByID(FS_OVERLAY_ID(OVY_80));
    UnloadOverlayByID(FS_OVERLAY_ID(OVY_81));
    UnloadOverlayByID(FS_OVERLAY_ID(OVY_42));
}

FrontierLaunchArgs *Frontier_GetLaunchArgs(FrontierWork *work) {
    return work->launchArgs;
}

FrontierMap *Frontier_GetMap(FrontierWork *work) {
    return work->map;
}

void *Frontier_GetData(FrontierWork *work) {
    return work->launchArgs->frontierFsys;
}

void Frontier_SetData(FrontierWork *work, void *data) {
    work->launchArgs->frontierFsys = data;
}

void Frontier_LaunchApplication(FrontierWork *work, const OverlayManagerTemplate *template, void *args, BOOL freeArgs, FrontierAppExitFunc exit) {
    GF_ASSERT(work->childManager == NULL);
    work->childManager = OverlayManager_New(template, args, HEAP_ID_FIELD2);
    work->childArgs = args;
    work->freeChildArgs = freeArgs;
    work->childExit = exit;
}

void Frontier_RequestExit(FrontierWork *work) {
    work->exitRequested = TRUE;
}

void Frontier_ChangeMode(FrontierWork *work, u16 mode, u16 taskId) {
    work->launchArgs->frontierMapId = mode;
    work->rebuildPending = TRUE;
    work->taskId = taskId;
}

FrontierMapObjectGfx *Frontier_GetMapObjectGfx(FrontierWork *work) {
    return work->objectGfx;
}

FrontierMapObject *Frontier_GetMapObjects(FrontierWork *work) {
    return work->objects;
}

FrontierMapObject *Frontier_GetMapObject(FrontierWork *work, int index) {
    return &work->objects[index];
}

FrontierMapSpriteState *Frontier_GetMapSpriteState(FrontierWork *work) {
    return &work->spriteState;
}

void Frontier_ResetMapSpriteState(FrontierWork *work) {
    int i;
    MI_CpuFill8(work->spriteState.resourceIds, 0, 8);
    for (i = 0; i < FRONTIER_MAP_SPRITE_MAX; i++) {
        work->spriteState.resourceIds[i] = FRONTIER_MAP_NO_ID;
    }
}
