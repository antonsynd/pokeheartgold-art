#ifndef POKEHEARTGOLD_FRONTIER_H
#define POKEHEARTGOLD_FRONTIER_H

#include "bag_cursor.h"
#include "field_system.h"
#include "overlay_manager.h"

struct FrontierWork;

typedef struct FrontierLaunchArgs {
    void *frontierFsys;
    Options *options;
    SaveData *saveData;
    BagCursor *bagCursor;
    u32 unk10;
    u32 unk14;
    u32 mapId;
    void *keyPressedSave;
    u8 frontierMapId;
    FieldSystem *fieldSystem;
} FrontierLaunchArgs;

extern const OverlayManagerTemplate gOverlayTemplate_Frontier;

typedef void(FrontierAppExitFunc)(void *);

FrontierLaunchArgs *Frontier_GetLaunchArgs(struct FrontierWork *work);
void Frontier_SetData(struct FrontierWork *work, void *data);
void *Frontier_GetData(struct FrontierWork *work);
void Frontier_LaunchApplication(struct FrontierWork *work, const OverlayManagerTemplate *ovyTemp, void *args, BOOL freeArgs, FrontierAppExitFunc func);

#endif // POKEHEARTGOLD_FRONTIER_H
