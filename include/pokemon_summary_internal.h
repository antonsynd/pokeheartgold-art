#ifndef POKEHEARTGOLD_POKEMON_SUMMARY_INTERNAL_H
#define POKEHEARTGOLD_POKEMON_SUMMARY_INTERNAL_H

#include "global.h"

#include "bg_cell_manager.h"
#include "bg_window.h"
#include "filesystem.h"
#include "message_format.h"
#include "message_printer.h"
#include "msgdata.h"
#include "overlay_manager.h"
#include "pokemon_summary.h"
#include "pokepic.h"
#include "sprite.h"
#include "sprite_system.h"
#include "touchscreen.h"

// Pokemon summary screen work.

// summary->sprites, in the order of the sprite templates
enum PokemonSummarySprite {
    POKEMON_SUMMARY_SPRITE_CURSOR = 9,                  // the party slot marker; the move cursor on the moves page
    POKEMON_SUMMARY_SPRITE_SWAP_CURSOR = 10,            // stays on the first move picked for a swap
    POKEMON_SUMMARY_SPRITE_TYPE_1 = 11,
    POKEMON_SUMMARY_SPRITE_TYPE_2 = 12,
    POKEMON_SUMMARY_SPRITE_MOVE_TYPES = 13,             // one per move slot
    POKEMON_SUMMARY_SPRITE_MOVE_TO_LEARN_TYPE = 17,
    POKEMON_SUMMARY_SPRITE_MOVE_CATEGORY = 18,
    POKEMON_SUMMARY_SPRITE_MON_ICON = 19,
    POKEMON_SUMMARY_SPRITE_STATUS = 20,
    POKEMON_SUMMARY_SPRITE_MARKINGS = 23,               // 6
    POKEMON_SUMMARY_SPRITE_SHEEN_SPARKLES = 29,         // 12, one per 1/12 of the sheen
    POKEMON_SUMMARY_SPRITE_CONDITION_CHANGES = 41,      // 5, cool..tough
    POKEMON_SUMMARY_SPRITE_CONDITION_COOL = 47,         // the corners of the condition chart
    POKEMON_SUMMARY_SPRITE_CONDITION_BEAUTY = 48,
    POKEMON_SUMMARY_SPRITE_CONDITION_CUTE = 49,
    POKEMON_SUMMARY_SPRITE_CONDITION_SMART = 50,
    POKEMON_SUMMARY_SPRITE_CONDITION_TOUGH = 51,
    POKEMON_SUMMARY_SPRITE_SHINY = 52,
    POKEMON_SUMMARY_SPRITE_POKERUS_CURED = 53,
    POKEMON_SUMMARY_SPRITE_RIBBONS = 54,                // 9, the current ribbon row page
    POKEMON_SUMMARY_SPRITE_RIBBON_CURSOR = 63,
    POKEMON_SUMMARY_SPRITE_RIBBON_PAGE_UP = 64,
    POKEMON_SUMMARY_SPRITE_RIBBON_PAGE_DOWN = 65,
    POKEMON_SUMMARY_SPRITE_POKERUS = 72,                // shown in place of the status icon while infected
    POKEMON_SUMMARY_SPRITE_PARTY_ICONS = 73,            // 6; a box Pokemon's icon alternates between the first two
    POKEMON_SUMMARY_SPRITE_POKEATHLON_STARS = 79,       // 5 stars for each of the 5 Pokeathlon stats
    POKEMON_SUMMARY_SPRITE_POKEATHLON_MODIFIERS = 104,  // each stat's Aprijuice modifier
    POKEMON_SUMMARY_SPRITE_SHINY_LEAVES = 109,          // 5
    POKEMON_SUMMARY_SPRITE_SHINY_LEAF_CROWN = 114,
    POKEMON_SUMMARY_SPRITE_COUNT,
};

enum PokemonSummaryState {
    POKEMON_SUMMARY_STATE_START_FADE_IN,
    POKEMON_SUMMARY_STATE_WAIT_FADE_IN,
    POKEMON_SUMMARY_STATE_INPUT,
    POKEMON_SUMMARY_STATE_SLIDE_IN_MOVE_INFO,
    POKEMON_SUMMARY_STATE_SLIDE_OUT_MOVE_INFO,
    POKEMON_SUMMARY_STATE_MOVE_SELECT_INPUT,
    POKEMON_SUMMARY_STATE_MOVE_SWAP_INPUT,
    POKEMON_SUMMARY_STATE_START_MOVE_SWAP,
    POKEMON_SUMMARY_STATE_FORGET_MOVE_INPUT,
    POKEMON_SUMMARY_STATE_FORGET_MOVE_CONFIRM_INPUT,
    POKEMON_SUMMARY_STATE_OPEN_RIBBON_LIST,
    POKEMON_SUMMARY_STATE_CLOSE_RIBBON_LIST,
    POKEMON_SUMMARY_STATE_RIBBON_INPUT,
    POKEMON_SUMMARY_STATE_RIBBON_PAGE_UP,
    POKEMON_SUMMARY_STATE_RIBBON_PAGE_DOWN,
    POKEMON_SUMMARY_STATE_FLASH_BUTTON,
    POKEMON_SUMMARY_STATE_UPDATE_CONDITION,
    POKEMON_SUMMARY_STATE_SHOW_CONDITION_CHANGES,
    POKEMON_SUMMARY_STATE_WAIT_CONDITION_EXIT,
    POKEMON_SUMMARY_STATE_SHOW_POKEPIC,
    POKEMON_SUMMARY_STATE_CHANGE_MON_BY_BUTTON,
    POKEMON_SUMMARY_STATE_START_FADE_OUT,
    POKEMON_SUMMARY_STATE_WAIT_FADE_OUT,
};

// summary->page, and the page lists given to PokemonSummary_SetPages
enum PokemonSummaryPage {
    POKEMON_SUMMARY_PAGE_INFO,
    POKEMON_SUMMARY_PAGE_STATS,     // stats and moves
    POKEMON_SUMMARY_PAGE_RIBBONS,   // with the Pokeathlon performance once flag 982 is set
    POKEMON_SUMMARY_PAGE_CONDITION,
    POKEMON_SUMMARY_PAGE_COUNT,     // ends a page list
};

// The ribbons are shown a page at a time, in a grid of 3 x 3
#define POKEMON_SUMMARY_RIBBON_COLUMNS   3
#define POKEMON_SUMMARY_RIBBONS_PER_PAGE (POKEMON_SUMMARY_RIBBON_COLUMNS * 3)

// What args->party holds (args->monDataType).
enum PokemonSummaryMonDataType {
    POKEMON_SUMMARY_MON_ARRAY,
    POKEMON_SUMMARY_PARTY,
    POKEMON_SUMMARY_BOX_MON_ARRAY,
};

// args->mode
enum PokemonSummaryMode {
    POKEMON_SUMMARY_MODE_NORMAL,
    POKEMON_SUMMARY_MODE_NO_MOVE_SWAP,
    POKEMON_SUMMARY_MODE_FORGET_MOVE,
    POKEMON_SUMMARY_MODE_CONDITION,
    POKEMON_SUMMARY_MODE_UPDATE_CONDITION,
};

// summary->moveSlot after the four moves when picking a move to forget; args->selectedMoveSlot is
// POKEMON_SUMMARY_MOVE_SLOT_NEW when no move was forgotten.
enum PokemonSummaryMoveSlot {
    POKEMON_SUMMARY_MOVE_SLOT_NEW = MAX_MON_MOVES, // the move to learn
    POKEMON_SUMMARY_MOVE_SLOT_CANCEL,              // the cancel button
};

enum PokemonSummaryPokerus {
    POKEMON_SUMMARY_POKERUS_NONE,
    POKEMON_SUMMARY_POKERUS_INFECTED,
    POKEMON_SUMMARY_POKERUS_CURED,
};

// The shown Pokemon, read once from the Pokemon or box Pokemon (PokemonSummary_ReadMon).
typedef struct PokemonSummaryMon {
    String *speciesName;
    String *nickname;
    String *otName;
    u16 species;
    u16 heldItem;
    u8 type1;
    u8 type2;
    u8 level : 7;
    u8 noPrintGender : 1;
    u8 gender : 2;
    u8 pokeball : 6;
    u32 otId;
    u32 exp;
    u32 curLevelExp;
    u32 nextLevelExp;
    u16 hp;
    u16 maxHp;
    u16 attack;
    u16 defense;
    u16 spAttack;
    u16 spDefense;
    u16 speed;
    u8 ability;
    u8 nature;
    u16 moves[4];
    u8 pp[4];
    u8 maxPP[4];
    u8 otGender;
    u8 contestStats[6]; // cool, beauty, cute, smart, tough, sheen
    u8 favoriteFlavor;  // 5 = none
    u16 markings;
    u16 form;
    u32 status : 28;
    u32 isEgg : 1;
    u32 shiny : 1;
    u32 pokerus : 2;
    u32 ribbons[4]; // one bit per ribbon
    u8 shinyLeaves[6];
} PokemonSummaryMon;

typedef struct ConditionChartVertex {
    s16 x, y, z;
} ConditionChartVertex;

// One of the four quads of the condition chart.
typedef struct ConditionChartQuad {
    ConditionChartVertex vertices[4];
} ConditionChartQuad;

typedef struct PokemonSummary {
    BgConfig *bgConfig;
    Window windows[34];
    Window *pageWindows; // the current page's windows
    u32 numPageWindows;
    PokemonSummaryArgs *args;
    PokemonSummaryMon mon;
    struct Camera *camera;
    PokepicManager *pokepicManager;
    PokepicAnimScript pokepicAnimScript[10];
    struct PokepicAnimManager *pokepicAnimManager;
    Pokepic *pokepic;
    u32 pokepicAnimReverseX;
    ConditionChartQuad conditionChartFrom[4];
    ConditionChartQuad conditionChartStep[4]; // per frame, from conditionChartFrom to conditionChartTo
    ConditionChartQuad conditionChartTo[4];
    u32 conditionChartDone;
    SpriteSystem *spriteSystem;
    SpriteManager *spriteManager;
    Sprite *sprites[POKEMON_SUMMARY_SPRITE_COUNT];
    u8 unk5D0[0x79C - 0x5D0];
    MessagePrinter *numberPrinter;
    MsgData *msgData;
    MsgData *ribbonMsgData;
    MessageFormat *msgFormat;
    String *buffer;
    String *playerName;
    MsgData *moveNames;
    NARC *pokepicAnimNarc;
    s8 page;
    u8 moveSlot : 4; // the two move slots being swapped
    u8 swapMoveSlot : 4;
    u8 subState; // step of the slide and flash animations; one bit per changed condition stat (bits 0-4) after an update
    u8 unk7BF_0 : 4;
    u8 exitRequested : 4; // 1: leave the screen
    u8 sparkleState;      // sheen sparkles
    u8 numSparkles;
    u8 unk7C2;
    u8 unk7C3;
    u8 ribbonCursor;
    u8 ribbonRow;
    u8 ribbonCount;
    u8 ribbonId;
    u8 flashX; // the touched button being flashed (PokemonSummary_FlashButton)
    u8 flashY;
    u8 flashWidth;
    u8 flashHeight;
    u8 flashPalette : 4;
    u8 restorePalette : 4;
    u8 flashBgId;
    u8 flashTimer;
    u8 flashNextState;
    BgCellManager *cellManager;
    u32 boxIconSprite; // which of the first two party icon sprites shows the box Pokemon's icon next
} PokemonSummary;

void PokemonSummary_Init3D(PokemonSummary *summary);
void PokemonSummary_AddWindows(PokemonSummary *summary);
void PokemonSummary_AddPageWindows(PokemonSummary *summary);
void PokemonSummary_RemovePageWindows(PokemonSummary *summary);
void PokemonSummary_RemoveWindows(PokemonSummary *summary);
void PokemonSummary_PrintNickname(PokemonSummary *summary);
void PokemonSummary_PrintLevel(PokemonSummary *summary);
void PokemonSummary_PrintHeldItem(PokemonSummary *summary);
void PokemonSummary_PrintPage(PokemonSummary *summary);
void PokemonSummary_PrintRibbonInfo(PokemonSummary *summary);
void PokemonSummary_PrintMoveInfo(PokemonSummary *summary, u16 move);
void PokemonSummary_ClearMoveInfo(PokemonSummary *summary);
void PokemonSummary_PrintMoveToLearn(PokemonSummary *summary);
void PokemonSummary_ScheduleMainBg1Transfer(PokemonSummary *summary);
void PokemonSummary_PrintSwappedMoves(PokemonSummary *summary);
void PokemonSummary_PrintCantForgetHMMessage(PokemonSummary *summary);
void PokemonSummary_ShowForgetLabel(PokemonSummary *summary, BOOL show);
void PokemonSummary_ShowSwitchLabel(PokemonSummary *summary, BOOL show);
void PokemonSummary_PrintConditionChangeMessage(PokemonSummary *summary, int result);

int PokemonSummary_FlashButton(PokemonSummary *summary);
void PokemonSummary_ReadMon(PokemonSummary *summary, Pokemon *mon, PokemonSummaryMon *out);

void *PokemonSummary_GetShownMon(PokemonSummary *summary); // Pokemon, or BoxPokemon for POKEMON_SUMMARY_BOX_MON_ARRAY

void PokemonSummary_DrawPokepic(PokemonSummary *summary);
void PokemonSummary_FreePokepicAndCamera(PokemonSummary *summary);
void PokemonSummary_CreateCamera(PokemonSummary *summary);
void PokemonSummary_InitConditionChart(PokemonSummary *summary);
void PokemonSummary_CreatePokepic(PokemonSummary *summary);
void PokemonSummary_StartPokepicAnim(PokemonSummary *summary);
void PokemonSummary_ReloadPokepic(PokemonSummary *summary);
u8 PokemonSummary_CheckTouchedButton(PokemonSummary *summary);

BOOL PokemonSummary_IsPokeathlonUnlocked(SaveData *saveData);
BOOL PokemonSummary_Init(OverlayManager *manager, int *state);
BOOL PokemonSummary_Main(OverlayManager *manager, int *state);
BOOL PokemonSummary_Exit(OverlayManager *manager, int *state);

int PokemonSummary_GetPokeathlonStarStates(u8 *states, int maxStars, int baseStars, int color, int modifier);

extern const GraphicsBanks gPokemonSummaryGraphicsBanks;
extern const GraphicsModes gPokemonSummaryGraphicsModes;
extern const BgTemplate gPokemonSummaryBgTemplate1;
extern const BgTemplate gPokemonSummaryBgTemplate2;
extern const BgTemplate gPokemonSummaryBgTemplate3;
extern const BgTemplate gPokemonSummaryBgTemplate4;
extern const BgTemplate gPokemonSummaryBgTemplate5;
extern const BgTemplate gPokemonSummaryBgTemplate6;

extern const TouchscreenHitbox gPokemonSummaryCancelButtonHitbox[];
extern const TouchscreenHitbox gPokemonSummarySwapButtonHitbox[];
extern const TouchscreenHitbox gPokemonSummaryConfirmButtonHitbox[];
extern const TouchscreenHitbox gPokemonSummaryMonButtonHitboxes[];
extern const TouchscreenHitbox gPokemonSummaryMoveHitboxes[];
extern const u32 gPokemonSummaryPageScreens[][3];
extern const TouchscreenHitbox gPokemonSummaryRibbonHitboxes[];

#endif // POKEHEARTGOLD_POKEMON_SUMMARY_INTERNAL_H
