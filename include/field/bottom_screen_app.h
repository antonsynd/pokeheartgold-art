#ifndef POKEHEARTGOLD_FIELD_BOTTOM_SCREEN_APP_H
#define POKEHEARTGOLD_FIELD_BOTTOM_SCREEN_APP_H

#include "constants/bottom_screen_app.h"

#include "field_system.h"

void FieldSystem_SendBottomScreenEvent(FieldSystem *fieldSystem, int event, int unused);
void BottomScreen_TouchSave_Init(FieldSystem *fieldSystem);
void BottomScreen_TouchSave_End(FieldSystem *fieldSystem);
BOOL BottomScreen_TouchSave_IsDone(FieldSystem *fieldSystem);
void BottomScreen_Apps_Init(FieldSystem *fieldSystem);
void BottomScreen_Apps_End(FieldSystem *fieldSystem);
BOOL BottomScreen_Apps_IsDone(FieldSystem *fieldSystem);
void FieldSystem_RequestTouchscreenMenuSelectedIconAnim(FieldSystem *fieldSystem);
void FieldSystem_RequestTouchscreenMenuRegisteredItemAnim(FieldSystem *fieldSystem, int item);

// The label of the touchscreen menu's and the dowsing machine's action button: what the A button does now
// (FieldSystem_GetFacingActionType gives check / talk / check BG event).
enum TouchscreenMenuActionType {
    TOUCHSCREEN_MENU_ACTION_CHECK,
    TOUCHSCREEN_MENU_ACTION_TALK,
    TOUCHSCREEN_MENU_ACTION_CHECK_BG_EVENT,
    TOUCHSCREEN_MENU_ACTION_FISHING,
    TOUCHSCREEN_MENU_ACTION_NEXT,
};

BOOL IsSpriteIdWithoutTalkAction(int spriteId);
BOOL IsScriptIdWithoutTalkAction(int scriptId);

#endif // POKEHEARTGOLD_FIELD_BOTTOM_SCREEN_APP_H
