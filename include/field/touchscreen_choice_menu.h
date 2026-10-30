#ifndef POKEHEARTGOLD_FIELD_TOUCHSCREEN_CHOICE_MENU_H
#define POKEHEARTGOLD_FIELD_TOUCHSCREEN_CHOICE_MENU_H

#include "constants/bottom_screen_app.h"

#include "bg_window.h"
#include "field_system.h"
#include "message_format.h"
#include "msgdata.h"
#include "sprite.h"
#include "sprite_resource.h"
#include "sprite_resource_header.h"
#include "sys_task.h"

#define TOUCHSCREEN_CHOICE_MENU_MAX_ITEMS 28

// The task data of the touch YES/NO buttons and the touch multichoice list (BOTTOM_SCREEN_APP_CHOICE_MENU).
typedef struct TouchscreenChoiceMenuData {
    enum TouchscreenChoiceMenuState state;
    u16 *result;
    u32 unk_08;
    struct ScriptMenu *menu; // ScrCmd_MenuExec list
    int frameCounter;
    u32 unk_14;
    BgConfig *bgConfig;
    SysTask *task;
    u16 *lastTouchMenuInput;
    FieldSystem *fieldSystem;
    Window yesWindow;
    Window noWindow;
    u32 unk_48;
    MsgData *msgData;
    MessageFormat *msgFormat;
    Window listWindows[TOUCHSCREEN_CHOICE_MENU_MAX_ITEMS];
    int itemCount;
    SpriteList *spriteList;
    G2dRenderer renderer;
    GF_2DGfxResMan *resMans[4];
    SpriteResource *resObjs[4];
    SpriteResourcesHeader header;
    Sprite *cursorSprite;
    u8 unk_38C[8];
    int selection;
    u32 unk_398;
    BOOL (*selectionCallback)(struct ScriptMenu *menu, int selection); // ScrCmd_MenuExec: MenuExec_PrintItemHelp
    struct ScriptMenu *selectionCallbackMenu;
} TouchscreenChoiceMenuData;

void TouchscreenChoiceMenu_SetCallback(SysTask *task, BOOL (*callback)(struct ScriptMenu *menu, int selection), struct ScriptMenu *menu);

#endif // POKEHEARTGOLD_FIELD_TOUCHSCREEN_CHOICE_MENU_H
