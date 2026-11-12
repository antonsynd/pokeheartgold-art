#ifndef POKEHEARTGOLD_FRONTIER_SYSTEM_H
#define POKEHEARTGOLD_FRONTIER_SYSTEM_H

#include "frontier/frontier_map.h"

#include "message_format.h"
#include "msgdata.h"
#include "pm_string.h"

struct FrontierScriptContext;

// Runs the frontier map's scripts: up to 8 script contexts (tasks) at a time, which share the map's script data
// (narc 182 member) and message bank unless a task is started for another map.
typedef struct FrontierSystem {
    FrontierWork *frontierWork; // the frontier app (Frontier_GetLaunchArgs / Frontier_GetData)
    struct FrontierScriptContext *tasks[8];
    u16 vars[8]; // script variables 0x8008-0x800F
    enum HeapID heapId;
    u8 tasksPaused;
    u8 syncAborted; // set while FrtCmd_LinkSyncOrAbort waits when the partner stops the link
    u8 filler_3A[2];
    MsgData *msgData;
    u8 *scripts;
    MessageFormat *msgFormat;
    String *msgBuffer;
    String *msgTemplate;
    u8 textPrinterId;
    s8 taskCount;
    u16 mapId;
    u8 filler_54[5];
    u8 movementCount; // map object movements still running
    u8 messageWindowAdded;
    u8 filler_5B[5];
    void *menu;
    Window messageWindow;
    struct ListMenu2D *yesNoMenu;
    u8 filler_78[4];
    struct WaitingIcon *waitingIcon;
    u8 filler_80[0x28];
    Window *castlePointsWindows[2]; // Battle Castle points (BattleCastle_ShowPointsWindows)
    u16 recordSaveState;            // battle record save (FrtCmd_SaveBattleRecord)
    u16 recordWriteState;
    u16 recordType;
    u16 recordAmount;
    u16 *recordResult;
} FrontierSystem;

FrontierSystem *FrontierSystem_Create(FrontierWork *frontierWork, enum HeapID heapId, u32 mapId);
BOOL FrontierSystem_Main(FrontierSystem *system);
void FrontierSystem_Delete(FrontierSystem *system);
void FrontierSystem_AddTask(FrontierSystem *system, u16 mapId, u32 scriptIndex);
void FrontierSystem_ChangeMsgBank(FrontierSystem *system, u16 mapId, enum HeapID heapId);
u16 *FrontierSystem_SaveVars(FrontierSystem *system, enum HeapID heapId);
void FrontierSystem_RestoreVars(FrontierSystem *system, u16 *saved);
u16 *FrontierSystem_GetVarPtr(FrontierSystem *system, u32 index);
FrontierMap *FrontierSystem_GetFrontierMap(FrontierSystem *system);

#endif
