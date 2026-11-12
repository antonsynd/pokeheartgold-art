#ifndef POKEHEARTGOLD_POKEATHLON_POKEATHLON_PENNANT_CAPTURE_H
#define POKEHEARTGOLD_POKEATHLON_POKEATHLON_PENNANT_CAPTURE_H

#include "global.h"

#include "pokeathlon/pokeathlon.h"
#include "pokeathlon/pokeathlon_course_types_def.h"

#include "bg_window.h"
#include "geometry_math.h"
#include "message_format.h"
#include "msgdata.h"
#include "pm_string.h"
#include "sprite.h"

// Pennant Capture: the Pokemon pick up the pennants that appear around a walled field (at most 9 carried) and
// score by carrying them into the goal area at the bottom. Running into a Pokemon takes up to two of its pennants.

#define PENNANT_CAPTURE_OBSTACLES   20
#define PENNANT_CAPTURE_FIELD_ITEMS 10
#define PENNANT_CAPTURE_PENNANTS    5  // on the field at once
#define PENNANT_CAPTURE_SPOTS       20 // where the pennants appear, in turn
#define PENNANT_CAPTURE_MAX_CARRIED 9  // pennants a participant can carry
#define PENNANT_CAPTURE_CPU_CELLS   48 // the 6x8 cells around a CPU Pokemon it picks a target from

// The movement parameters of one of the three Pokemon (one per team member).
typedef struct PokeathlonPennantCaptureMoveParams {
    f32 accel;      // with accelScale: how fast a CPU Pokemon speeds up toward its target
    f32 accelScale;
    f32 knockbackScale; // share of the own speed passed on to a Pokemon it knocks down
    s32 knockdownTime;  // base of the frames a knocked-down Pokemon stays down
    u8 maxSpeed;
    u8 unk11[3];
} PokeathlonPennantCaptureMoveParams;

// One participant's team on the field (the Pokemon currently running is `member`).
typedef struct PokeathlonPennantCapturePlayer {
    PokeathlonCourseMonObject *monObjects[POKEATHLON_TEAM_SIZE]; // one course object per team member
    PokeathlonPennantCaptureMoveParams moveParams[POKEATHLON_TEAM_SIZE];
    PokeathlonCourseSpritePoolSprite *carrySprite;     // shown above the Pokemon while active
    PokeathlonCourseSpritePoolSprite *pointSprites[2]; // "+points" effects, used alternately
    PokeathlonCourseSpritePoolSprite *pennantSprite;   // pennant count change effect
    VecFx32 position;                                  // field position (fx32)
    VecFx32 velocity;
    s16 drawOffsetY;
    u8 flashTimer;
    u8 shownPennants;
    s32 wallSide; // side of the obstacle hit last (0 = none)
    u16 nextPointSprite;
    u16 holdTime;      // frames the touch was held (human players)
    VecFx32 cpuFrom;   // CPU: where the current path starts; human: where the touch drag started
    VecFx32 cpuTarget; // CPU: where it leads; human: where the touch drag is now
    u8 contacts[POKEATHLON_PARTICIPANTS]; // touching the other participants' Pokemon
    u32 participant;
    u8 swapPending; // set once a deposit finished
    u8 swapPhase;
    u8 swapState; // team member swap shown on the top screen
    u8 swapStep;
    u8 swapTimer;
    u8 unkA1;
    u8 knockdownTimer;
    u8 active;
    u8 freezeTimer;  // frames without movement after a collision
    u8 cpuHasTarget; // CPU: a path is chosen; human: a flick is pending
    u8 knockedDown;
    u8 pointsToAdd; // deposited pennants still to be counted
    u8 depositDelay;
    u8 depositSteps;
    u8 pennants; // carried pennants (at most 9)
    u8 depositing;
    u16 score;
    u16 shownScore; // score as last received from the host
    u8 direction;   // 1-4, see PokeathlonPennantCapture_GetDirection
    u8 member;      // team member on the field
    u8 unkB2;
    u8 touchingPennant[PENNANT_CAPTURE_PENNANTS];
} PokeathlonPennantCapturePlayer;

// An obstacle block (32 x 32 units).
typedef struct PokeathlonPennantCaptureObstacle {
    BOOL placed; // FALSE ends the list
    PokeathlonCourseSpritePoolSprite *sprite;
    u16 x;
    u16 y;
} PokeathlonPennantCaptureObstacle;

// One of the ten two-sprite field objects; the CPU players head for them.
typedef struct PokeathlonPennantCaptureFieldItem {
    BOOL placed; // FALSE ends the list
    PokeathlonCourseSpritePoolSprite *sprite;
    PokeathlonCourseSpritePoolSprite *shadowSprite;
    u16 x;
    u16 y;
} PokeathlonPennantCaptureFieldItem;

typedef struct PokeathlonPennantCapturePennant {
    u32 active;
    PokeathlonCourseSpritePoolSprite *sprite;
    PokeathlonCourseSpritePoolSprite *poleSprite;
    u32 spot; // index into PokeathlonPennantCapturePennants.spots
    u16 x;
    u16 y;
} PokeathlonPennantCapturePennant;

typedef struct PokeathlonPennantCapturePoint {
    u16 x;
    u16 y;
} PokeathlonPennantCapturePoint;

// The five pennants on the field and the 20 spots they appear at, in turn.
typedef struct PokeathlonPennantCapturePennants {
    PokeathlonPennantCapturePennant pennants[PENNANT_CAPTURE_PENNANTS];
    PokeathlonPennantCapturePoint spots[PENNANT_CAPTURE_SPOTS];
    u16 nextSpot;
    u16 count;
} PokeathlonPennantCapturePennants;

// The event's work (PokeathlonCourse_GetWork).
typedef struct PokeathlonPennantCapture {
    BgConfig *bgConfig;
    Window scoreWindow;
    enum HeapID heapId;
    void *bgCharsFile;            // the file bgChars points into
    NNSG2dCharacterData *bgChars; // the animated BG0 tiles (6 frames)
    PokeathlonCourseSpritePool *spritePool;
    PokeathlonPennantCapturePlayer players[POKEATHLON_PARTICIPANTS];
    PokeathlonCourseTouch touches[POKEATHLON_PARTICIPANTS]; // host: the participants' touch state
    BOOL dragging[POKEATHLON_PARTICIPANTS];                         // the touch started on the own Pokemon
    PokeathlonCourseSpriteResources *monResources;
    PokeathlonCourseMonObjects *monObjects;
    PokeathlonCourseSpritePoolSprite *pickupEffects[4]; // SEQ_SE_GS_THLON06, used in turn
    PokeathlonCourseSpritePoolSprite *dropEffects[4];   // SEQ_SE_GS_SAA, used in turn
    struct PokeathlonPennantCaptureCpu *cpu;
    struct PokeathlonPennantCaptureDisplay *display;
    u32 layoutChoice; // host: the field layout it picked (0-19), sent to everyone as layoutFileId
    u32 layoutFileId; // field layout member in NARC 227
    PokeathlonPennantCaptureObstacle obstacles[PENNANT_CAPTURE_OBSTACLES];
    PokeathlonPennantCaptureFieldItem fieldItems[PENNANT_CAPTURE_FIELD_ITEMS];
    u16 timeLeft; // frames until the event ends
    u8 finished;
    u8 unk50F;
    u8 heldTooLong; // stronger friction while set
    u8 bgAnimFrame;
    u8 bgAnimTimer;
    u8 shownPennantSpots[PENNANT_CAPTURE_PENNANTS];
    u8 nextPickupEffect;
    u8 hitReported;
    u8 nextDropEffect;
    u8 flicked; // a flick was made this frame
    u16 hitY;
    u16 hitX;
    u16 flickY; // field position of the last flick
    u16 flickX;
    u32 pickupEffectX[4];
    u32 pickupEffectY[4];
    u32 dropEffectX[4];
    u32 dropEffectY[4];
    PokeathlonPennantCapturePennants pennants;
    PokeathlonPennantCapturePoint obstaclePositions[PENNANT_CAPTURE_OBSTACLES];
    PokeathlonPennantCapturePoint fieldItemPositions[PENNANT_CAPTURE_FIELD_ITEMS];
    u8 unk694[0x28];
    u8 pennantsFull[POKEATHLON_PARTICIPANTS]; // touched a pennant while carrying 9
} PokeathlonPennantCapture;

// The frame state the host sends to all participants.
// words[0-3]: per participant x (bits 0-8), y (9-17), score (18-27), pennants (28-31).
// words[4]: drop effect x, y, bit 18 new drop effect, bits 19-22 depositing participants,
//           bits 23-30 knocked-down contacts, bit 31 finished.
// words[5]: pickup effect x, y, bit 18 new pickup effect, bits 19-22 "pennants full" per participant.
// spots[0-4]: bits 0-4 spot of pennant i; for i < 4 also member (bits 5-6), direction - 1 (7-8),
//             knocked down (9) and points to add (11-15) of participant i.
typedef struct PokeathlonPennantCaptureFrame {
    u32 words[6];
    u16 spots[PENNANT_CAPTURE_PENNANTS];
    u16 timeLeft;
} PokeathlonPennantCaptureFrame;

// Link data as seen by the frame update: own block, the other participants' blocks, the frame.
typedef struct PokeathlonPennantCaptureLinkData {
    u8 own[0x50];
    u8 others[POKEATHLON_PARTICIPANTS][0x28];
    u8 frame[0x28];
} PokeathlonPennantCaptureLinkData;

// A participant's team member on the top screen.
typedef struct PokeathlonPennantCaptureDisplayMon {
    PokeathlonCourseSpritePoolSprite *icon;
    PokeathlonCourseSpritePoolSprite *carryMark;
    PokeathlonCourseSpritePoolSprite *frame;
    u8 active;
    u8 unk0D[0xF];
} PokeathlonPennantCaptureDisplayMon;

typedef struct PokeathlonPennantCaptureCounter {
    u16 x;
    u16 y;
} PokeathlonPennantCaptureCounter;

// The top screen (score, the participants' Pokemon, the timer).
typedef struct PokeathlonPennantCaptureDisplay {
    enum HeapID heapId;
    PokeathlonCourseData *course;
    BgConfig *bgConfig;
    Window window;
    MsgData *msgData;
    MessageFormat *msgFormat;
    void *scoreScreenFile;
    NNSG2dScreenData *scoreScreen;
    u8 unk02C[0xC];

    union {
        PokeathlonPennantCaptureDisplayMon mons[POKEATHLON_PARTICIPANTS];

        struct {
            u8 filler[0x64];
            void *iconFiles[POKEATHLON_MONS];
        } raw;
    } u;

    NNSG2dCharacterData *iconChars[POKEATHLON_MONS];
    u8 iconPalettes[POKEATHLON_MONS];
    PokeathlonCourseMonPic monPics[POKEATHLON_TEAM_SIZE];
    u32 picCharOffsets[2];
    u32 picPlttOffsets[2];
    PokeathlonCourseSpritePoolSprite *picSprites[2];
    PokeathlonCourseSpritePoolSprite *goalSprite;
    PokeathlonCourseSpritePoolSprite *statusSprites[4][4];
    PokeathlonCourseSpritePoolSprite *pointSprites[4][2];
    int picState;
    u16 picSlot;
    u16 nextPointSprite;
    u8 shownMember[POKEATHLON_PARTICIPANTS];
    String *names[POKEATHLON_TEAM_SIZE];
    Sprite *timerDigits[2];
    PokeathlonPennantCaptureCounter counters[3];
} PokeathlonPennantCaptureDisplay;

// The per-level move parameter tables (tenths), indexed by the Pokemon's performance levels.
typedef struct PokeathlonPennantCaptureMoveTables {
    int knockbackScale[5];
    int accel[5];
    int accelScale[5];
    int knockdownTime[5];
    int maxSpeed[5];
} PokeathlonPennantCaptureMoveTables;

// An obstacle block grown by a radius: its corners, then its four edges moved outward.
typedef struct PokeathlonPennantCaptureBounds {
    Point2D corners[4];
    Point2D edges[4][2];
} PokeathlonPennantCaptureBounds;

// A participant's Pokemon as listed for the top screen.
typedef struct PokeathlonPennantCaptureIconMon {
    u16 species;
    u16 form;
} PokeathlonPennantCaptureIconMon;

// A team member as passed to PokeathlonPennantCaptureDisplay_LoadMonPics.
typedef struct PokeathlonPennantCapturePicMon {
    u16 species;
    u16 form;
    u16 unused4;
    u8 shiny;
    u8 gender;
    u32 unused8;
    u32 personality;
} PokeathlonPennantCapturePicMon;

// What the CPU sees of a participant: the Pokemon and its position on the touch screen of the host.
typedef struct PokeathlonPennantCaptureCpuPlayer {
    PokeathlonPennantCapturePlayer *player;
    u16 x;
    u16 y;
} PokeathlonPennantCaptureCpuPlayer;

typedef struct PokeathlonPennantCaptureCpuObject {
    BOOL active;
    int x;
    int y;
} PokeathlonPennantCaptureCpuObject;

// The CPU of the participants without a player (one decision per CPU every `interval` frames).
typedef struct PokeathlonPennantCaptureCpu {
    PokeathlonPennantCaptureCpuPlayer players[POKEATHLON_PARTICIPANTS];
    PokeathlonPennantCaptureCpuObject obstacles[PENNANT_CAPTURE_OBSTACLES];
    PokeathlonPennantCaptureCpuObject fieldItems[PENNANT_CAPTURE_FIELD_ITEMS];
    PokeathlonPennantCapturePennant *pennants;
    u32 unk18C;
    PokeathlonCourseCpuCell cells[PENNANT_CAPTURE_CPU_CELLS]; // 6 x 8 cells (32 x 32) around the Pokemon
    BOOL thinking;
    u8 cpuCount;
    u8 cursor;
    u8 timer;
    u8 interval;
    u8 firstCpu;
    u8 humanCount;
} PokeathlonPennantCaptureCpu;

// Resource ids of sprite sets (gPokeathlonPennantCaptureSpriteIds).
typedef struct PokeathlonPennantCaptureSpriteIds {
    u32 ids[2];
} PokeathlonPennantCaptureSpriteIds;

// The sides of an obstacle a path enters through (edge and corner hit codes; 9-12 = from inside).
typedef struct PokeathlonPennantCaptureHitCodes {
    int sides[4];
} PokeathlonPennantCaptureHitCodes;

BOOL PokeathlonPennantCapture_Init(PokeathlonCourseData *course);
BOOL PokeathlonPennantCapture_Draw(PokeathlonCourseData *course);
BOOL PokeathlonPennantCapture_ShowResults(PokeathlonCourseData *course);
BOOL PokeathlonPennantCapture_Exit(PokeathlonCourseData *course);

void PokeathlonPennantCapture_UpdatePlayers(PokeathlonCourseData *course);
void PokeathlonPennantCapture_ApplyFrame(PokeathlonCourseData *course, int viewer, PokeathlonPennantCapturePlayer *players, PokeathlonPennantCaptureLinkData *link);
void PokeathlonPennantCapture_UpdateObstacleSprites(PokeathlonPennantCapture *work, int x, int y);
void PokeathlonPennantCapture_UpdateFieldItemSprites(PokeathlonPennantCapture *work, int x, int y);
void PokeathlonPennantCapture_UpdatePennantSprites(PokeathlonPennantCapture *work, PokeathlonPennantCaptureFrame *frame, int x, int y);
void PokeathlonPennantCapture_SetMoveParams(PokeathlonCourseData *course, const PokeathlonPennantCaptureMoveTables *tables, int participant, u8 member, PokeathlonPennantCapturePlayer *player);
float PokeathlonPennantCapture_GetMaxSpeed(PokeathlonPennantCapturePlayer *player, int member);
void PokeathlonPennantCapture_AddPoint2D(const Point2D *a, const Point2D *b, Point2D *out);
void PokeathlonPennantCapture_UpdateFieldDrawPriorities(PokeathlonPennantCapture *work);
void PokeathlonPennantCapture_InitScoreWindow(PokeathlonPennantCapture *work);
void PokeathlonPennantCapture_ResetPennantSpots(PokeathlonPennantCapture *work);
void PokeathlonPennantCapture_RefillPennants(PokeathlonPennantCapture *work);
int PokeathlonPennantCapture_GetDirection(VecFx32 *vector);
void PokeathlonPennantCapture_AnimateBg(PokeathlonPennantCapture *work);
void PokeathlonPennantCapture_UpdateOwnScore(PokeathlonCourseData *course);

PokeathlonPennantCaptureDisplay *PokeathlonPennantCaptureDisplay_New(enum HeapID heapId, BgConfig *bgConfig, PokeathlonCourseData *course);
void PokeathlonPennantCaptureDisplay_Delete(PokeathlonPennantCaptureDisplay *display);
void PokeathlonPennantCaptureDisplay_LoadBgGfx(PokeathlonPennantCaptureDisplay *display);
void PokeathlonPennantCaptureDisplay_LoadSpriteResources(PokeathlonPennantCaptureDisplay *display, PokeathlonCourseSpritePool *pool);
void PokeathlonPennantCaptureDisplay_CreateSprites(PokeathlonPennantCaptureDisplay *display, PokeathlonCourseSpritePool *pool, PokeathlonCourseSpriteResources *resources);
void PokeathlonPennantCaptureDisplay_LoadIcons(PokeathlonPennantCaptureDisplay *display, const PokeathlonPennantCaptureIconMon *mons);
void PokeathlonPennantCaptureDisplay_LoadMonPics(PokeathlonPennantCaptureDisplay *display, const PokeathlonPennantCapturePicMon *mons);
void PokeathlonPennantCaptureDisplay_LoadMonPicToVram(PokeathlonPennantCaptureDisplay *display, u32 slot, u32 member);
void PokeathlonPennantCaptureDisplay_SetScore(PokeathlonPennantCaptureDisplay *display, int score);
BOOL PokeathlonPennantCaptureDisplay_SlideMonPic(PokeathlonPennantCaptureDisplay *display, int member);
BOOL PokeathlonPennantCaptureDisplay_StepSwapWait(PokeathlonPennantCaptureDisplay *display, u32 participant);
void PokeathlonPennantCaptureDisplay_UpdateMon(PokeathlonPennantCaptureDisplay *display, u8 participant, u16 member, BOOL hidden, int active);
void PokeathlonPennantCaptureDisplay_SetTimer(PokeathlonPennantCaptureDisplay *display, int frames);
void PokeathlonPennantCaptureDisplay_BringToFront(PokeathlonPennantCaptureDisplay *display, int chosen);
void PokeathlonPennantCaptureDisplay_SetPennants(PokeathlonPennantCaptureDisplay *display, int participant, u32 pennants);
void PokeathlonPennantCaptureDisplay_ShowPoint(PokeathlonPennantCaptureDisplay *display, int participant);

void PokeathlonPennantCaptureDisplay_GetPicVramOffsets(PokeathlonCourseSpritePool *pool, PokeathlonPennantCaptureDisplay *display);
void PokeathlonPennantCaptureDisplay_InitWindow(PokeathlonPennantCaptureDisplay *display);
void PokeathlonPennantCaptureDisplay_ShowStatus(PokeathlonCourseSpritePoolSprite **sprites, BOOL show);
PokeathlonPennantCaptureCpu *PokeathlonPennantCaptureCpu_New(enum HeapID heapId, int cpuCount, int mode);
void PokeathlonPennantCaptureCpu_SetPlayer(PokeathlonPennantCaptureCpu *cpu, u8 participant, PokeathlonPennantCapturePlayer *player);
void PokeathlonPennantCaptureCpu_SetPennants(PokeathlonPennantCaptureCpu *cpu, PokeathlonPennantCapturePennant *pennants);
void PokeathlonPennantCaptureCpu_SetObstacles(PokeathlonPennantCaptureCpu *cpu, const u8 *spots);
void PokeathlonPennantCaptureCpu_Delete(PokeathlonPennantCaptureCpu *cpu);
int PokeathlonPennantCaptureCpu_Update(PokeathlonPennantCaptureCpu *cpu);

extern const VecFx32 gPokeathlonPennantCaptureInitialPosition;

// Checks of the CPU on the target of one of its field cells.
BOOL PokeathlonPennantCaptureCpu_IsNearCellTarget(PokeathlonCourseCpuCell *cell, s32 x, s32 y);
BOOL PokeathlonPennantCaptureCpu_IsCellTargetOnField(PokeathlonCourseCpuCell *cell);

// The Pokemon picture sprites of the display.
void PokeathlonPennantCaptureDisplay_CreatePicSprites(PokeathlonCourseSpritePool *pool, PokeathlonPennantCaptureDisplay *display);

extern const GraphicsBanks gPennantCaptureGraphicsBanks;
extern const PokeathlonCourseSpritePoolTemplate gPennantCaptureSpritePoolTemplate;
extern const GraphicsModes gPennantCaptureGraphicsModes;
extern const BgTemplate gPennantCaptureBgTemplate_Sub0;
extern const BgTemplate gPennantCaptureBgTemplate_Sub1;
extern const BgTemplate gPennantCaptureBgTemplate_Sub2;
extern const BgTemplate gPennantCaptureBgTemplate_Main0;
extern const BgTemplate gPennantCaptureBgTemplate_Main3;
extern const u8 gPennantCaptureTimerDigitX[];
extern const u8 gPennantCaptureTimerDigitAnims[];
extern const WindowTemplate gPennantCaptureScoreWindowTemplate;
extern const VecFx32 gPennantCaptureReferenceVector;
extern const PokeathlonPennantCaptureHitCodes gPennantCaptureEdgeHits;   // edge hits
extern const PokeathlonPennantCaptureHitCodes gPennantCaptureCornerHits; // corner hits

#endif // POKEHEARTGOLD_POKEATHLON_POKEATHLON_PENNANT_CAPTURE_H
