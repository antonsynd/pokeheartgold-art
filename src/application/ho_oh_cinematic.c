#include "global.h"

#include "constants/sndseq.h"

#include "follow_mon.h"
#include "legendary_cinematic.h"
#include "legendary_cinematic_internal.h"
#include "screen_fade.h"
#include "sound_playback.h"

static int HoOhCinematic_Init(LegendaryCinematic *cinematic);
static int HoOhCinematic_Exit(LegendaryCinematic *cinematic);
static int HoOhCinematic_WaitFade(LegendaryCinematic *cinematic);
static int HoOhCinematic_PlayFirstScene(LegendaryCinematic *cinematic);
static int HoOhCinematic_LoadSecondScene(LegendaryCinematic *cinematic);
static int HoOhCinematic_PlaySecondScene(LegendaryCinematic *cinematic);

static const GXRgb sHoOhCinematicEdgeColors[] = {
    GX_RGB(0, 0, 0),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
    GX_RGB(4, 4, 4),
};

enum HoOhCinematicState {
    HO_OH_CINEMATIC_INIT,
    HO_OH_CINEMATIC_EXIT,
    HO_OH_CINEMATIC_WAIT_FADE,
    HO_OH_CINEMATIC_PLAY_FIRST_SCENE,
    HO_OH_CINEMATIC_LOAD_SECOND_SCENE,
    HO_OH_CINEMATIC_PLAY_SECOND_SCENE,
    HO_OH_CINEMATIC_DONE,
};

static int (*const sScriptCinematicSubs_HoOh[HO_OH_CINEMATIC_DONE])(LegendaryCinematic *) = {
    HoOhCinematic_Init,
    HoOhCinematic_Exit,
    HoOhCinematic_WaitFade,
    HoOhCinematic_PlayFirstScene,
    HoOhCinematic_LoadSecondScene,
    HoOhCinematic_PlaySecondScene,
};

static const LegendaryCinematicModelSpec sHoOhCinematicModels[] = {
    { .model = 0,  .anims = { 1, 2, 3, 4 },     .noMaterialColors = FALSE },
    { .model = 5,  .anims = { 6, 7, 8, 9 },     .noMaterialColors = FALSE },
    { .model = 10, .anims = { 11, 12, 13, 14 }, .noMaterialColors = FALSE },
};

static const LegendaryCinematicModelSpec sHoOhCinematicSecondSceneModels[] = {
    { .model = 15, .anims = { 16, 17, 18, MODEL_ANIM_NONE }, .noMaterialColors = FALSE },
    { .model = 19, .anims = { 20, 21, 22, MODEL_ANIM_NONE }, .noMaterialColors = FALSE },
    { .model = 23, .anims = { 24, 25, 26, MODEL_ANIM_NONE }, .noMaterialColors = FALSE },
};

static const LegendaryCinematicCamera sHoOhCinematicCamera = {
    .param.distance = FX32_CONST(1204.08251953125),
    .param.angle.x = 0xC80,
    .param.angle.y = 0xD8E0,
    .param.angle.z = 0,
    .param.perspectiveType = CAMERA_PERSPECTIVE_TYPE_PERSPECTIVE,
    .param.perspective = 0x881,
    .target.x = FX32_CONST(602.4970703125),
    .target.y = FX32_CONST(578.511474609375),
    .target.z = -FX32_CONST(604.906982421875),
    .nearClip = FX32_CONST(9),
    .farClip = FX32_CONST(1700),
};

static const LegendaryCinematicCamera sHoOhCinematicSecondSceneCamera = {
    .param.distance = FX32_CONST(442.08251953125),
    .param.angle.x = 0xECA2,
    .param.angle.y = 0x8380,
    .param.angle.z = 0,
    .param.perspectiveType = CAMERA_PERSPECTIVE_TYPE_PERSPECTIVE,
    .param.perspective = 0x381,
    .target.x = -FX32_CONST(18.497802734375),
    .target.y = -FX32_CONST(32.8759765625),
    .target.z = FX32_CONST(58.973388671875),
    .nearClip = FX32_CONST(150),
    .farClip = FX32_CONST(1700),
};

BOOL ScriptCinematic_HoOh(LegendaryCinematic *cinematic) {
    cinematic->state = sScriptCinematicSubs_HoOh[cinematic->state](cinematic);
    if (cinematic->state == HO_OH_CINEMATIC_DONE) {
        return FALSE;
    }
    LegendaryCinematic_DrawModels(cinematic);
    return TRUE;
}

static int HoOhCinematic_Init(LegendaryCinematic *cinematic) {
    u32 variant;
    LegendaryCinematic_Init3D(cinematic);
    LegendaryCinematic_SetLights(&cinematic->args->modelAttributes);
    LegendaryCinematic_SetEdgeMarking(sHoOhCinematicEdgeColors);
    LegendaryCinematic_SetCamera(cinematic, &sHoOhCinematicCamera);
    variant = cinematic->args->timeOfDay;
    LegendaryCinematic_LoadModels(cinematic, NARC_a_2_4_3, &sHoOhCinematicModels[variant], 1);
    LegendaryCinematic_FadeInFromWhite(cinematic, 6, 1, HO_OH_CINEMATIC_PLAY_FIRST_SCENE);
    return HO_OH_CINEMATIC_WAIT_FADE;
}

static int HoOhCinematic_Exit(LegendaryCinematic *cinematic) {
    LegendaryCinematic_FreeModels(cinematic);
    LegendaryCinematic_Free3D(cinematic);
    return HO_OH_CINEMATIC_DONE;
}

static int HoOhCinematic_WaitFade(LegendaryCinematic *cinematic) {
    if (IsPaletteFadeFinished() == TRUE) {
        return cinematic->nextState;
    }
    return HO_OH_CINEMATIC_WAIT_FADE;
}

static int HoOhCinematic_PlayFirstScene(LegendaryCinematic *cinematic) {
    u32 frame;
    if (!LegendaryCinematic_AreModelsAnimating(cinematic) && IsPaletteFadeFinished() == TRUE) {
        cinematic->frame = 0;
        return HO_OH_CINEMATIC_LOAD_SECOND_SCENE;
    }
    frame = cinematic->frame;
    if (frame == 0) {
        PlaySE(SEQ_SE_GS_SUZUNOONPA_APRI);
    } else if (frame == 40) {
        PlaySE(SEQ_SE_GS_HAKKOUTAI_SYUTUGEN);
    } else if (frame == 78) {
        PlayBGMOnBGMPlayerInField(SEQ_GS_E_HOUOU);
        PlaySE(SEQ_SE_GS_HAKKOUTAI_HIROGARI2);
    } else if (frame == 98) {
        PlaySE(SEQ_SE_GS_HAKKOUTAI_HIROGARI);
    } else if (frame == 129) {
        LegendaryCinematic_FadeOutToBlack(cinematic, 6, 1, HO_OH_CINEMATIC_PLAY_FIRST_SCENE);
    }
    cinematic->frame++;
    return HO_OH_CINEMATIC_PLAY_FIRST_SCENE;
}

static int HoOhCinematic_LoadSecondScene(LegendaryCinematic *cinematic) {
    u32 variant;
    LegendaryCinematic_FreeModels(cinematic);
    LegendaryCinematic_SetCamera(cinematic, &sHoOhCinematicSecondSceneCamera);
    variant = cinematic->args->timeOfDay;
    LegendaryCinematic_LoadModels(cinematic, NARC_a_2_4_3, &sHoOhCinematicSecondSceneModels[variant], 1);
    LegendaryCinematic_FadeInFromBlack(cinematic, 6, 1, HO_OH_CINEMATIC_PLAY_SECOND_SCENE);
    return HO_OH_CINEMATIC_WAIT_FADE;
}

static int HoOhCinematic_PlaySecondScene(LegendaryCinematic *cinematic) {
    if (!LegendaryCinematic_AreModelsAnimating(cinematic) && IsPaletteFadeFinished() == TRUE) {
        cinematic->frame = 0;
        return HO_OH_CINEMATIC_EXIT;
    }
    if ((cinematic->frame & 0x3F) == 0) {
        PlaySE(SEQ_SE_GS_HOUOUHABATAKI_APRI);
    }
    if (cinematic->frame == 190) {
        LegendaryCinematic_FadeOutToWhite(cinematic, 10, 1, HO_OH_CINEMATIC_EXIT);
    }
    cinematic->frame++;
    return HO_OH_CINEMATIC_PLAY_SECOND_SCENE;
}
