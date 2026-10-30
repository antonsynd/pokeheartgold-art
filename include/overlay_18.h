#ifndef POKEHEARTGOLD_OVY_18_H
#define POKEHEARTGOLD_OVY_18_H

#include "battle/battle.h"

#include "menu_input_state.h"
#include "overlay_manager.h"
#include "player_data.h"
#include "pokedex.h"
#include "pokedex_resume_state.h"

typedef struct PokedexArgs {
    Pokedex *pokedex;
    PlayerProfile *playerProfile;
    PokedexResumeState *resumeState;
    MenuInputStateMgr *menuInputStatePtr;
    int x;
    int y;
    u16 mapId;
    u16 mapMatrixId;
} PokedexArgs;

struct PokedexEntryPopup;

BOOL Pokedex_Init(OverlayManager *man, int *state);
BOOL Pokedex_Main(OverlayManager *man, int *state);
BOOL Pokedex_Exit(OverlayManager *man, int *state);

struct PokedexEntryPopup *PokedexEntryPopup_Create(PokedexEntryPopupArgs *args);
s32 PokedexEntryPopup_IsDone(struct PokedexEntryPopup *man);
void PokedexEntryPopup_Delete(struct PokedexEntryPopup *man);
Pokepic *PokedexEntryPopup_GetPokepic(struct PokedexEntryPopup *man);
void PokedexEntryPopup_HideSprites(struct PokedexEntryPopup *man);

#endif // POKEHEARTGOLD_OVY_18_H
