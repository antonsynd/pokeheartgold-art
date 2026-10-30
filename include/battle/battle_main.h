#ifndef POKEHEARTGOLD_BATTLE_BATTLE_MAIN_H
#define POKEHEARTGOLD_BATTLE_BATTLE_MAIN_H

#include "battle/battle.h"
#include "battle/battle_controller_packets.h"

// The battle overlay's main module.

FS_EXTERN_OVERLAY(OVY_7);
FS_EXTERN_OVERLAY(OVY_10);

typedef struct BattleBgTemplateSet {
    BgTemplate templates[3];
} BattleBgTemplateSet;

// A battler and its battler type, passed to OpponentData_New
typedef struct BattlerInitData {
    u8 battler;
    u8 battlerType;
} BattlerInitData;

extern const BattleBgTemplateSet gBattleReinitBgTemplates;
extern const BattleBgTemplateSet gBattleBgTemplates;
extern const u32 gBattleBgFillColors[23][3];
extern const u8 gDoublesBattlerTypes[4];

void BattleSystem_InitTerrainsAndBalls(BattleSystem *battleSystem);
void Battle_VBlankCallback(void *data);

// Defined in other modules that have no header declaring them yet.
void BattleController_ProcessQueue(BattleSystem *battleSystem, enum BattleControllerPacketKind direction);
void BgConfig_InitBattleMenuBackgrounds(BgConfig *bgConfig);
void LoadUserFrameGfx2WithFill(BgConfig *bgConfig, u8 bgId, u16 baseTile, u8 color, u8 type, enum HeapID heapId);
u32 GetUserFrameGfx2TilesMember(u32 frame);
u32 GetUserFrameGfx2PaletteMember(u32 frame);

#endif // POKEHEARTGOLD_BATTLE_BATTLE_MAIN_H
