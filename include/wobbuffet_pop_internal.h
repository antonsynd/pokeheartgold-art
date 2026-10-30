#ifndef POKEHEARTGOLD_WOBBUFFET_POP_INTERNAL_H
#define POKEHEARTGOLD_WOBBUFFET_POP_INTERNAL_H

#include "communication_command.h"
#include "wobbuffet_pop.h"

typedef struct WobbuffetPopModelFiles {
    int files[5];
} WobbuffetPopModelFiles;

// Area the handle can be grabbed in, per balloon selection
typedef struct WobbuffetPopTouchArea {
    u8 left;
    u8 right;
    u8 top;
    u8 bottom;
} WobbuffetPopTouchArea;

extern const CameraAngle gWobbuffetPopCameraAngle;
extern BOOL (*const gWobbuffetPopHostSteps[])(WobbuffetPop *game, WobbuffetPopSequence *sequence);
extern const VecFx32 gWobbuffetPopCameraTarget;
extern const GraphicsModes gWobbuffetPopGraphicsModes;
extern const u8 gWobbuffetPopSeatOrders[][4];
extern const u8 gWobbuffetPopNameWindowIndices[];
extern const OamCharTransferParam gWobbuffetPopOamCharTransfer;
extern const SpriteResourceCountsListUnion gWobbuffetPopSpriteResourceCounts;
extern const OamManagerParam gWobbuffetPopOamManagerParam;
extern const GraphicsBanks gWobbuffetPopBanks;
extern const WobbuffetPopModelFiles gWobbuffetPopLocalModelFiles[];
extern const WobbuffetPopModelFiles gWobbuffetPopSlotModelFiles0[];
extern const WobbuffetPopModelFiles gWobbuffetPopSlotModelFiles1[];
extern const WobbuffetPopModelFiles gWobbuffetPopSlotModelFiles2[];
extern const WobbuffetPopModelFiles gWobbuffetPopSlotModelFiles3[];
extern const BgTemplate gWobbuffetPopMainBgTemplates[3];
extern const BgTemplate gWobbuffetPopSubBgTemplates[4];
extern const s32 gWobbuffetPopBalloonAirRates[];

extern const CommCommand gWobbuffetPopCommands[];
extern void (*const gWobbuffetPopEventFuncs[])(WobbuffetPop *game, WobbuffetPopEvent *event);

extern const WobbuffetPopTouchArea gWobbuffetPopBalloonTouchAreas[];
extern const ManagedSpriteTemplate gWobbuffetPopHandleSpriteTemplate;

#endif // POKEHEARTGOLD_WOBBUFFET_POP_INTERNAL_H
