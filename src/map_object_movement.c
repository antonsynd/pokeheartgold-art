#include "map_object_movement.h"

#include "global.h"

#include "field_system.h"
#include "map_object.h"
#include "map_object_movement_commands_internal.h"
#include "map_object_movement_utilities_internal.h"
#include "math_util.h"
#include "player_avatar.h"
#include "player_avatar_movement.h"

// The movement data of each movement type (MapObject_InitMovementData)

typedef struct RandomLookMovementState {
    u16 state;
    s16 delay;
    u32 tableId;
} RandomLookMovementState;

typedef struct LocalMovementState {
    s16 state;
    s16 delay;
    u32 restrictToRange;
    u32 heldMovement;
    u32 directionTable;
} LocalMovementState;

typedef struct LocalMovementBounds {
    s32 minX;
    s32 minZ;
    s32 maxX;
    s32 maxZ;
} LocalMovementBounds;

typedef struct FaceDirectionMovementState {
    u32 direction;
    u32 state;
} FaceDirectionMovementState;

// order: the direction (DIR_WEST: counterclockwise, DIR_EAST: clockwise) the object turns in; the alternating spin
// flips it each time the object faces its initial direction again
typedef struct SpinMovementState {
    s8 order;
    u8 unused;
    s8 state;
    int timer;
} SpinMovementState;

typedef struct DirectionCycle {
    int value[5];
} DirectionCycle;

// Objects of type 7 and 8 turn one step through sWalkingSpinOrders before each step they walk
typedef struct WalkingSpinState {
    s8 facing;
    s8 order;
    u8 wasTurning;
} WalkingSpinState;

typedef struct BackAndForthMovementState {
    s16 state;
    s16 turn;
    WalkingSpinState spin;
} BackAndForthMovementState;

typedef struct MovementPathState {
    u8 state;
    u8 index;
    u8 limit;
    u8 axis;
    u32 tableId;
    WalkingSpinState spin;
} MovementPathState;

typedef struct MovementListEntry {
    int id;
    const int *list;
} MovementListEntry;

typedef BOOL (*SpinMovementStateFunc)(LocalMapObject *object, SpinMovementState *state);
typedef BOOL (*BackAndForthMovementStateFunc)(LocalMapObject *object, BackAndForthMovementState *state);
typedef BOOL (*PathMovementStateFunc)(LocalMapObject *object, MovementPathState *state);

enum RandomWalkState {
    RANDOM_WALK_FACE,
    RANDOM_WALK_WAIT_FACE,
    RANDOM_WALK_WAIT_DELAY,
    RANDOM_WALK_START_STEP,
    RANDOM_WALK_WAIT_STEP,
};

static BOOL SpinMovement_FaceNearbyPlayer(LocalMapObject *object, SpinMovementState *state);
static BOOL SpinMovement_WaitFace(LocalMapObject *object, SpinMovementState *state);
static BOOL SpinMovement_Wait(LocalMapObject *object, SpinMovementState *state);
static BOOL SpinMovement_Turn(LocalMapObject *object, SpinMovementState *state);
static BOOL AlternatingSpinMovement_Face(LocalMapObject *object, SpinMovementState *state);
static BOOL AlternatingSpinMovement_WaitFace(LocalMapObject *object, SpinMovementState *state);
static BOOL AlternatingSpinMovement_Wait(LocalMapObject *object, SpinMovementState *state);
static BOOL AlternatingSpinMovement_Turn(LocalMapObject *object, SpinMovementState *state);
static BOOL BackAndForthMovement_FaceStart(LocalMapObject *object, BackAndForthMovementState *state);
static BOOL BackAndForthMovement_Step(LocalMapObject *mapObject, BackAndForthMovementState *state);
static BOOL BackAndForthMovement_WaitStep(LocalMapObject *object, BackAndForthMovementState *state);
static BOOL PathMovement_Step(LocalMapObject *object, MovementPathState *state);
static BOOL PathMovement_WaitStep(LocalMapObject *object, MovementPathState *state);

static const PathMovementStateFunc sPathMovementStateFuncs[] = {
    PathMovement_Step,
    PathMovement_WaitStep,
};

static const int sDirectionTable09[] = { DIR_NORTH, DIR_SOUTH, DIR_NONE };

static const int sDirectionTable04[] = { DIR_SOUTH, DIR_EAST, DIR_NONE };

static const int sDirectionTable02[] = { DIR_NORTH, DIR_EAST, DIR_NONE };

static const int sDirectionTable12[] = { DIR_NORTH, DIR_SOUTH, DIR_NONE };

static const BackAndForthMovementStateFunc sBackAndForthMovementStateFuncs[] = {
    BackAndForthMovement_FaceStart,
    BackAndForthMovement_Step,
    BackAndForthMovement_WaitStep,
};

static const int sDirectionTable10[] = { DIR_WEST, DIR_EAST, DIR_NONE };

static const int sDirectionTable01[] = { DIR_NORTH, DIR_WEST, DIR_NONE };

static const int sDirectionTable03[] = { DIR_SOUTH, DIR_WEST, DIR_NONE };

static const int sDirectionTable13[] = { DIR_WEST, DIR_EAST, DIR_NONE };

static const int sDirectionTable36[] = { DIR_WEST, DIR_NORTH, DIR_EAST, DIR_SOUTH };

static const int sDirectionTable19[] = { DIR_WEST, DIR_EAST, DIR_SOUTH, DIR_NORTH };

static const int sDirectionTable37[] = { DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_NORTH };

static const int sDirectionTable33[] = { DIR_EAST, DIR_NORTH, DIR_WEST, DIR_SOUTH };

static const int sDirectionTable25[] = { DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_NORTH };

static const int sDirectionTable22[] = { DIR_WEST, DIR_NORTH, DIR_SOUTH, DIR_EAST };

static const int sDirectionTable07[] = { DIR_NORTH, DIR_WEST, DIR_EAST, DIR_NONE };

static const int sDirectionTable24[] = { DIR_EAST, DIR_WEST, DIR_NORTH, DIR_SOUTH };

static const int sDirectionTable05[] = { DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_NONE };

static const int sDirectionTable18[] = { DIR_WEST, DIR_EAST, DIR_SOUTH, DIR_NORTH };

static const int sDirectionTable32[] = { DIR_WEST, DIR_SOUTH, DIR_EAST, DIR_NORTH };

static const int sDirectionTable26[] = { DIR_EAST, DIR_NORTH, DIR_SOUTH, DIR_WEST };

static const int sDirectionTable23[] = { DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST };

static const int sDirectionTable27[] = { DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST };

static const int sDirectionTable14[] = { DIR_NORTH, DIR_EAST, DIR_WEST, DIR_SOUTH };

static const int sDirectionTable15[] = { DIR_EAST, DIR_WEST, DIR_SOUTH, DIR_NORTH };

static const SpinMovementStateFunc sAlternatingSpinMovementStateFuncs[] = {
    AlternatingSpinMovement_Face,
    AlternatingSpinMovement_WaitFace,
    AlternatingSpinMovement_Wait,
    AlternatingSpinMovement_Turn,
};

static const int sDirectionTable29[] = { DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH };

static const int sDirectionTable30[] = { DIR_NORTH, DIR_WEST, DIR_SOUTH, DIR_EAST };

static const SpinMovementStateFunc sSpinMovementStateFuncs[] = {
    SpinMovement_FaceNearbyPlayer,
    SpinMovement_WaitFace,
    SpinMovement_Wait,
    SpinMovement_Turn,
};

static const int sDirectionTable35[] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST };

static const int sDirectionTable16[] = { DIR_SOUTH, DIR_NORTH, DIR_EAST, DIR_WEST };

static const int sDirectionTable17[] = { DIR_WEST, DIR_SOUTH, DIR_NORTH, DIR_EAST };

static const int sDirectionTable06[] = { DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_NONE };

static const int sDirectionTable08[] = { DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NONE };

static const int sDirectionTable21[] = { DIR_EAST, DIR_SOUTH, DIR_NORTH, DIR_WEST };

static const int sDirectionTable34[] = { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };

static const int sDirectionTable20[] = { DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST };

static const int sDirectionTable28[] = { DIR_WEST, DIR_EAST, DIR_NORTH, DIR_SOUTH };

static const int sDirectionTable31[] = { DIR_SOUTH, DIR_EAST, DIR_NORTH, DIR_WEST };

static const DirectionCycle sSpinClockwiseOrder = {
    { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_NONE }
};

static const DirectionCycle sAlternatingSpinCounterclockwiseOrder = {
    { DIR_NORTH, DIR_WEST, DIR_SOUTH, DIR_EAST, DIR_NONE }
};

static const DirectionCycle sAlternatingSpinClockwiseOrder = {
    { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_NONE }
};

static const int sRandomLookDelays[] = { 16, 32, 48, 64, -1 };

static const int sDirectionTable11[] = { DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NONE };

static const DirectionCycle sSpinCounterclockwiseOrder = {
    { DIR_NORTH, DIR_WEST, DIR_SOUTH, DIR_EAST, DIR_NONE }
};

static const int sDirectionTable38[] = { DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NONE };

static const int sDirectionTable00[] = { DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NONE };

static const int sWalkingSpinOrders[][DIR_MAX] = {
    { DIR_NORTH, DIR_WEST, DIR_SOUTH, DIR_EAST },
    { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST },
};

static const int sPlayerAwareMovementTypes[] = { 2, 6, 7, 8, 9, 10, 11, 12, 13, 45, 46, 18, 19, 255 };

static const MovementListEntry sMovementDirectionTables[] = {
    { 0,  sDirectionTable00 },
    { 1,  sDirectionTable01 },
    { 2,  sDirectionTable02 },
    { 3,  sDirectionTable03 },
    { 4,  sDirectionTable04 },
    { 5,  sDirectionTable05 },
    { 6,  sDirectionTable06 },
    { 7,  sDirectionTable07 },
    { 8,  sDirectionTable08 },
    { 9,  sDirectionTable09 },
    { 10, sDirectionTable10 },
    { 11, sDirectionTable11 },
    { 12, sDirectionTable12 },
    { 13, sDirectionTable13 },
    { 14, sDirectionTable14 },
    { 15, sDirectionTable15 },
    { 16, sDirectionTable16 },
    { 17, sDirectionTable17 },
    { 18, sDirectionTable18 },
    { 19, sDirectionTable19 },
    { 20, sDirectionTable20 },
    { 21, sDirectionTable21 },
    { 22, sDirectionTable22 },
    { 23, sDirectionTable23 },
    { 24, sDirectionTable24 },
    { 25, sDirectionTable25 },
    { 26, sDirectionTable26 },
    { 27, sDirectionTable27 },
    { 28, sDirectionTable28 },
    { 29, sDirectionTable29 },
    { 30, sDirectionTable30 },
    { 31, sDirectionTable31 },
    { 32, sDirectionTable32 },
    { 33, sDirectionTable33 },
    { 34, sDirectionTable34 },
    { 35, sDirectionTable35 },
    { 36, sDirectionTable36 },
    { 37, sDirectionTable37 },
    { 38, sDirectionTable38 },
    { 39, NULL              },
};

static void RandomLookMovement_Init(LocalMapObject *object, u32 value);
static void RandomWalkMovement_Init(LocalMapObject *object, u32 heldMovement, u32 directionTable, u32 restrictToRange);
static void MapObject_GetMovementBounds(LocalMapObject *object, LocalMovementBounds *bounds);
static BOOL MapObject_IsWithinMovementBounds(LocalMapObject *object, u32 direction);
static void FaceDirectionMovement_Init(LocalMapObject *object, u32 direction);
static void SpinMovement_Init(LocalMapObject *object, u32 order);
static void PathMovement_Init(LocalMapObject *object, u32 limit, u32 axis, u32 tableId);
static int DirectionList_Count(const int *list, int value);
static int DirectionList_GetRandom(const int *list, int value);
static int GetRandomDirectionFromTable(int tableId, int value);
static const int *GetDirectionTable(int id);
static int MapObject_GetDirectionToNearbyPlayer(LocalMapObject *object);
static int MapObject_GetAllowedDirectionToNearbyPlayer(LocalMapObject *object, int tableId, int value);
static BOOL MapObject_SpinsWhileWalking(LocalMapObject *object);
static void WalkingSpin_Init(LocalMapObject *object, WalkingSpinState *state);
static void WalkingSpin_TurnBeforeStep(LocalMapObject *object, WalkingSpinState *state);
static void WalkingSpin_EndStep(LocalMapObject *object, WalkingSpinState *state);

static void RandomLookMovement_Init(LocalMapObject *object, u32 value) {
    RandomLookMovementState *state = MapObject_InitMovementData(object, sizeof(RandomLookMovementState));
    state->delay = DirectionList_GetRandom(sRandomLookDelays, -1);
    state->tableId = value;
    MapObject_SetAnimation(object, MAP_OBJECT_ANIM_STAND);
    MapObject_ClearSingleMovement(object);
}

void RandomLookMovement_InitAnyDirection(LocalMapObject *object) {
    RandomLookMovement_Init(object, 0);
}

void RandomLookMovement_InitUpLeft(LocalMapObject *object) {
    RandomLookMovement_Init(object, 1);
}

void RandomLookMovement_InitUpRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 2);
}

void RandomLookMovement_InitDownLeft(LocalMapObject *object) {
    RandomLookMovement_Init(object, 3);
}

void RandomLookMovement_InitDownRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 4);
}

void RandomLookMovement_InitUpDownLeft(LocalMapObject *object) {
    RandomLookMovement_Init(object, 5);
}

void RandomLookMovement_InitUpDownRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 6);
}

void RandomLookMovement_InitUpLeftRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 7);
}

void RandomLookMovement_InitDownLeftRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 8);
}

void RandomLookMovement_InitUpDown(LocalMapObject *object) {
    RandomLookMovement_Init(object, 9);
}

void RandomLookMovement_InitLeftRight(LocalMapObject *object) {
    RandomLookMovement_Init(object, 10);
}

void RandomLookMovement_Update(LocalMapObject *object) {
    RandomLookMovementState *state = MapObject_GetMovementData(object);
    int direction = MapObject_GetAllowedDirectionToNearbyPlayer(object, state->tableId, -1);
    if (direction != DIR_NONE) {
        MapObject_SetFacingDirection(object, direction);
    } else if (state->state == 0) {
        state->delay--;
        if (state->delay <= 0) {
            state->delay = DirectionList_GetRandom(sRandomLookDelays, -1);
            direction = GetRandomDirectionFromTable(state->tableId, -1);
            MapObject_SetFacingDirection(object, direction);
        }
    }
    MapObject_SetPreviousCoordsToCurrent(object);
}

void RandomLookMovement_Delete(LocalMapObject *object) {
}

void RandomWalkMovement_InitAnyDirection(LocalMapObject *object) {
    RandomWalkMovement_Init(object, MOVEMENT_STEP_UP, 11, 0);
}

void RandomWalkMovement_InitUpDown(LocalMapObject *object) {
    RandomWalkMovement_Init(object, MOVEMENT_STEP_UP, 12, 0);
}

void RandomWalkMovement_InitLeftRight(LocalMapObject *object) {
    RandomWalkMovement_Init(object, MOVEMENT_STEP_UP, 13, 0);
}

static void RandomWalkMovement_Init(LocalMapObject *object, u32 heldMovement, u32 directionTable, u32 restrictToRange) {
    LocalMovementState *state = MapObject_InitMovementData(object, sizeof(LocalMovementState));
    state->restrictToRange = restrictToRange;
    state->heldMovement = heldMovement;
    state->directionTable = directionTable;
    MapObject_SetAnimation(object, MAP_OBJECT_ANIM_STAND);
    MapObject_ClearSingleMovement(object);
}

void RandomWalkMovement_Update(LocalMapObject *object) {
    LocalMovementState *state = MapObject_GetMovementData(object);
    u32 direction;
    switch (state->state) {
    case RANDOM_WALK_FACE:
        MapObject_ClearSingleMovement(object);
        MapObject_ClearEndMovement(object);
        direction = GetMovementForDirection(MapObject_GetFacingDirection(object), MOVEMENT_FACE_UP);
        MapObject_ForceSetHeldMovement(object, direction);
        state->state++;
        break;
    case RANDOM_WALK_WAIT_FACE:
        if (!MapObject_RunForcedMovement(object)) {
            break;
        }
        state->delay = DirectionList_GetRandom(sRandomLookDelays, -1);
        state->state++;
    case RANDOM_WALK_WAIT_DELAY:
        state->delay--;
        if (state->delay != 0) {
            break;
        }
        state->state++;
    case RANDOM_WALK_START_STEP:
        direction = GetRandomDirectionFromTable(state->directionTable, -1);
        MapObject_SetOrQueueFacing(object, direction);
        if (state->restrictToRange == 1 && !MapObject_IsWithinMovementBounds(object, direction)) {
            state->state = RANDOM_WALK_FACE;
            break;
        }
        if (MapObject_CheckCollisionInDirection(object, direction)) {
            state->state = RANDOM_WALK_FACE;
            break;
        }
        MapObject_ForceSetHeldMovement(object, GetMovementForDirection(direction, state->heldMovement));
        MapObject_SetSingleMovement(object);
        state->state++;
    case RANDOM_WALK_WAIT_STEP:
        if (MapObject_RunForcedMovement(object)) {
            MapObject_ClearSingleMovement(object);
            state->state = RANDOM_WALK_FACE;
        }
        break;
    }
}

static void MapObject_GetMovementBounds(LocalMapObject *object, LocalMovementBounds *bounds) {
    s32 x = MapObject_GetInitialX(object);
    s32 z = MapObject_GetInitialZ(object);
    s32 xRange = MapObject_GetXRange(object);
    s32 zRange = MapObject_GetYRange(object);
    switch (MapObject_GetMovement(object)) {
    case 6:
        bounds->minX = x - xRange;
        bounds->maxX = x;
        bounds->minZ = z - zRange;
        bounds->maxZ = z;
        break;
    case 7:
        bounds->minX = x;
        bounds->maxX = x + xRange;
        bounds->minZ = z - zRange;
        bounds->maxZ = z;
        break;
    case 8:
        bounds->minX = x - xRange;
        bounds->maxX = x;
        bounds->minZ = z;
        bounds->maxZ = z + zRange;
        break;
    case 9:
        bounds->minX = x;
        bounds->maxX = x + xRange;
        bounds->minZ = z;
        bounds->maxZ = z + zRange;
        break;
    case 10:
        bounds->minX = x - xRange;
        bounds->maxX = x;
        bounds->minZ = z - zRange;
        bounds->maxZ = z + zRange;
        break;
    case 11:
        bounds->minX = x;
        bounds->maxX = x + xRange;
        bounds->minZ = z - zRange;
        bounds->maxZ = z + zRange;
        break;
    case 12:
        bounds->minX = x - xRange;
        bounds->maxX = x + xRange;
        bounds->minZ = z - zRange;
        bounds->maxZ = z;
        break;
    case 13:
        bounds->minX = x - xRange;
        bounds->maxX = x + xRange;
        bounds->minZ = z;
        bounds->maxZ = z + zRange;
        break;
    default:
        GF_ASSERT(FALSE);
        break;
    }
}

static BOOL MapObject_IsWithinMovementBounds(LocalMapObject *object, u32 direction) {
    LocalMovementBounds bounds;
    s32 x;
    s32 z;
    MapObject_GetMovementBounds(object, &bounds);
    x = MapObject_GetXCoord(object) + GetDeltaXByFacingDirection(direction);
    z = MapObject_GetZCoord(object) + GetDeltaYByFacingDirection(direction);
    if (bounds.minX > x || bounds.maxX < x) {
        return FALSE;
    }
    if (bounds.minZ > z || bounds.maxZ < z) {
        return FALSE;
    }
    return TRUE;
}

static void FaceDirectionMovement_Init(LocalMapObject *object, u32 direction) {
    FaceDirectionMovementState *state = MapObject_InitMovementData(object, sizeof(FaceDirectionMovementState));
    state->direction = direction;
    MapObject_SetAnimation(object, MAP_OBJECT_ANIM_STAND);
    MapObject_ClearSingleMovement(object);
    MapObject_SetPreviousCoordsToCurrent(object);
}

void FaceDirectionMovement_Update(LocalMapObject *object) {
    FaceDirectionMovementState *state = MapObject_GetMovementData(object);
    switch (state->state) {
    case 0:
        MapObject_SetFacingDirection(object, state->direction);
        state->state++;
        break;
    case 1:
        break;
    }
}

void FaceDirectionMovement_InitUp(LocalMapObject *object) {
    FaceDirectionMovement_Init(object, DIR_NORTH);
}

void FaceDirectionMovement_InitDown(LocalMapObject *object) {
    FaceDirectionMovement_Init(object, DIR_SOUTH);
}

void FaceDirectionMovement_InitLeft(LocalMapObject *object) {
    FaceDirectionMovement_Init(object, DIR_WEST);
}

void FaceDirectionMovement_InitRight(LocalMapObject *object) {
    FaceDirectionMovement_Init(object, DIR_EAST);
}

static void SpinMovement_Init(LocalMapObject *object, u32 order) {
    SpinMovementState *state = MapObject_InitMovementData(object, sizeof(SpinMovementState));
    state->order = order;
    MapObject_SetAnimation(object, MAP_OBJECT_ANIM_STAND);
    MapObject_ClearSingleMovement(object);
    MapObject_SetPreviousCoordsToCurrent(object);
}

void SpinMovement_InitCounterclockwise(LocalMapObject *object) {
    SpinMovement_Init(object, DIR_WEST);
}

void SpinMovement_InitClockwise(LocalMapObject *object) {
    SpinMovement_Init(object, DIR_EAST);
}

void SpinMovement_Update(LocalMapObject *object) {
    SpinMovementState *state = MapObject_GetMovementData(object);

    while (sSpinMovementStateFuncs[state->state](object, state) == TRUE) {
    }
}

static BOOL SpinMovement_FaceNearbyPlayer(LocalMapObject *object, SpinMovementState *state) {
    int direction = MapObject_GetAllowedDirectionToNearbyPlayer(object, 0x26, -1);
    if (direction == DIR_NONE) {
        direction = MapObject_GetFacingDirection(object);
    }
    MapObject_ForceSetHeldMovement(object, GetMovementForDirection(direction, MOVEMENT_FACE_UP));
    state->state = 1;
    return TRUE;
}

static BOOL SpinMovement_WaitFace(LocalMapObject *object, SpinMovementState *state) {
    if (!MapObject_RunForcedMovement(object)) {
        return FALSE;
    }
    state->timer = 0;
    state->state = 2;
    return TRUE;
}

static BOOL SpinMovement_Wait(LocalMapObject *object, SpinMovementState *state) {
    if (state->timer != 0 && MapObject_GetAllowedDirectionToNearbyPlayer(object, 0x26, -1) != DIR_NONE) {
        state->state = 0;
        return TRUE;
    }
    state->timer++;
    if (state->timer < 24) {
        return FALSE;
    }
    state->state = 3;
    return TRUE;
}

static BOOL SpinMovement_Turn(LocalMapObject *object, SpinMovementState *state) {
    DirectionCycle listA = sSpinCounterclockwiseOrder;
    DirectionCycle listB = sSpinClockwiseOrder;
    int i;
    DirectionCycle *list = &listA;
    u32 direction;

    if (state->order != DIR_WEST) {
        list = &listB;
    }
    direction = MapObject_GetFacingDirection(object);
    i = 0;
    if (list->value[0] != -1) {
        int *entry = list->value;
        do {
            if (direction == *entry) {
                break;
            }
            entry++;
            i++;
        } while (*entry != -1);
    }
    GF_ASSERT(list->value[i] != -1);
    i++;
    if (list->value[i] == -1) {
        i = 0;
    }
    MapObject_SetFacingDirection(object, list->value[i]);
    state->state = 0;
    return TRUE;
}

void AlternatingSpinMovement_Init(LocalMapObject *object) {
    SpinMovement_Init(object, DIR_EAST);
}

void AlternatingSpinMovement_Update(LocalMapObject *object) {
    SpinMovementState *state = MapObject_GetMovementData(object);
    while (sAlternatingSpinMovementStateFuncs[state->state](object, state) == TRUE) {}
}

static BOOL AlternatingSpinMovement_Face(LocalMapObject *object, SpinMovementState *state) {
    u32 facing = MapObject_GetFacingDirection(object);
    MapObject_ForceSetHeldMovement(object, GetMovementForDirection(facing, MOVEMENT_FACE_UP));
    state->state = 1;
    return TRUE;
}

static BOOL AlternatingSpinMovement_WaitFace(LocalMapObject *object, SpinMovementState *state) {
    if (!MapObject_RunForcedMovement(object)) {
        return FALSE;
    }
    state->timer = 0;
    state->state = 2;
    return TRUE;
}

static BOOL AlternatingSpinMovement_Wait(LocalMapObject *object, SpinMovementState *state) {
    state->timer++;
    if (state->timer < 24) {
        return FALSE;
    }
    state->state = 3;
    return TRUE;
}

static BOOL AlternatingSpinMovement_Turn(LocalMapObject *object, SpinMovementState *state) {
    DirectionCycle orderA = sAlternatingSpinCounterclockwiseOrder;
    DirectionCycle orderB = sAlternatingSpinClockwiseOrder;
    int i;
    u32 direction;
    int *order;
    int *alternate;

    order = orderA.value;
    alternate = orderB.value;
    if (state->order != DIR_WEST) {
        order = alternate;
    }

    direction = MapObject_GetFacingDirection(object);
    i = 0;
    while (order[i] != -1) {
        if (direction == order[i]) {
            break;
        }
        i++;
    }
    GF_ASSERT(order[i] != -1);
    i++;
    if (order[i] == -1) {
        i = 0;
    }
    MapObject_SetFacingDirection(object, order[i]);

    direction = MapObject_GetFacingDirection(object);
    if (direction == MapObject_GetInitialFacingDirection(object)) {
        state->order = GetOppositeDirection(state->order);
    }
    state->state = 0;
    return TRUE;
}

void BackAndForthMovement_Init(LocalMapObject *object) {
    BackAndForthMovementState *state = MapObject_InitMovementData(object, sizeof(BackAndForthMovementState));
    if (MapObject_SpinsWhileWalking(object) == TRUE) {
        WalkingSpin_Init(object, &state->spin);
    }
}

void BackAndForthMovement_Update(LocalMapObject *object) {
    BackAndForthMovementState *state = MapObject_GetMovementData(object);
    while (sBackAndForthMovementStateFuncs[state->state](object, state) == TRUE) {}
}

static BOOL BackAndForthMovement_FaceStart(LocalMapObject *object, BackAndForthMovementState *state) {
    u32 direction = MapObject_GetInitialFacingDirection(object);
    if (state->turn == 1) {
        direction = GetOppositeDirection(direction);
    }
    MapObject_SetNextFacingDirection(object, direction);
    if (!MapObject_SpinsWhileWalking(object)) {
        MapObject_SetFacingDirection(object, direction);
    }
    state->state = 1;
    return TRUE;
}

static BOOL BackAndForthMovement_Step(LocalMapObject *mapObject, BackAndForthMovementState *state) {
    u32 direction;
    int result;
    int kind;

    if (state->turn != 0) {
        int initialX = MapObject_GetInitialX(mapObject);
        int initialZ = MapObject_GetInitialZ(mapObject);
        int x = MapObject_GetXCoord(mapObject);
        int z = MapObject_GetZCoord(mapObject);

        if (initialX == x && initialZ == z) {
            direction = GetOppositeDirection(MapObject_GetNextFacingDirection(mapObject));
            MapObject_SetNextFacingDirection(mapObject, direction);
            if (!MapObject_SpinsWhileWalking(mapObject)) {
                MapObject_SetFacingDirection(mapObject, direction);
            }
            state->turn = 0;
        }
    }
    direction = MapObject_GetNextFacingDirection(mapObject);
    result = MapObject_CheckCollisionInDirection(mapObject, direction);
    if (result & 1) {
        state->turn = 1;
        direction = GetOppositeDirection(direction);
        result = MapObject_CheckCollisionInDirection(mapObject, direction);
    }
    kind = MOVEMENT_STEP_UP;
    if (result != 0) {
        kind = 0x20;
    }
    result = GetMovementForDirection(direction, kind);
    MapObject_ForceSetHeldMovement(mapObject, result);
    if (MapObject_SpinsWhileWalking(mapObject) == TRUE) {
        WalkingSpin_TurnBeforeStep(mapObject, &state->spin);
    }
    MapObject_SetSingleMovement(mapObject);
    state->state = 2;
    return TRUE;
}

static BOOL BackAndForthMovement_WaitStep(LocalMapObject *object, BackAndForthMovementState *state) {
    if (MapObject_RunForcedMovement(object) == TRUE) {
        MapObject_ClearSingleMovement(object);
        if (MapObject_SpinsWhileWalking(object) == TRUE) {
            WalkingSpin_EndStep(object, &state->spin);
        }
        state->state = 0;
    }
    return FALSE;
}

static void PathMovement_Init(LocalMapObject *object, u32 limit, u32 axis, u32 tableId) {
    MovementPathState *state = MapObject_InitMovementData(object, sizeof(MovementPathState));
    state->limit = limit;
    state->axis = axis;
    state->tableId = tableId;
    if (MapObject_SpinsWhileWalking(object) == TRUE) {
        WalkingSpin_Init(object, &state->spin);
    }
}

void PathMovement_InitNorthEastWestSouth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 14);
}

void PathMovement_InitEastWestSouthNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 15);
}

void PathMovement_InitSouthNorthEastWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 16);
}

void PathMovement_InitWestSouthNorthEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 17);
}

void PathMovement_InitWestEastSouthNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 18);
}

void PathMovement_InitWestEastSouthNorth2(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 19);
}

void PathMovement_InitSouthNorthWestEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 20);
}

void PathMovement_InitEastSouthNorthWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 21);
}

void PathMovement_InitWestNorthSouthEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 22);
}

void PathMovement_InitNorthSouthEastWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 23);
}

void PathMovement_InitEastWestNorthSouth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 24);
}

void PathMovement_InitSouthEastWestNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 25);
}

void PathMovement_InitEastNorthSouthWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 26);
}

void PathMovement_InitNorthSouthWestEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 27);
}

void PathMovement_InitWestEastNorthSouth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 28);
}

void PathMovement_InitSouthWestEastNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 29);
}

void PathMovement_Update(LocalMapObject *object) {
    MovementPathState *state = MapObject_GetMovementData(object);
    while (sPathMovementStateFuncs[state->state](object, state) == TRUE) {}
}

static BOOL PathMovement_Step(LocalMapObject *object, MovementPathState *state) {
    const int *list;
    u32 direction;
    u32 collision;
    int movementKind;

    if (state->index == state->limit) {
        if (state->axis == 0) {
            if (MapObject_GetInitialX(object) == MapObject_GetXCoord(object)) {
                state->index++;
            }
        } else {
            if (MapObject_GetInitialZ(object) == MapObject_GetZCoord(object)) {
                state->index++;
            }
        }
    }
    if (state->index == 3) {
        int initialX = MapObject_GetInitialX(object);
        int initialZ = MapObject_GetInitialZ(object);
        int x = MapObject_GetXCoord(object);
        int z = MapObject_GetZCoord(object);
        if (initialX == x && initialZ == z) {
            state->index = 0;
        }
    }
    list = GetDirectionTable(state->tableId);
    direction = list[state->index];
    MapObject_SetNextFacingDirection(object, direction);
    if (!MapObject_SpinsWhileWalking(object)) {
        MapObject_SetFacingDirection(object, direction);
    }
    collision = MapObject_CheckCollisionInDirection(object, direction);
    if (collision & 1) {
        state->index++;
        direction = list[state->index];
        MapObject_SetNextFacingDirection(object, direction);
        if (!MapObject_SpinsWhileWalking(object)) {
            MapObject_SetFacingDirection(object, direction);
        }
        collision = MapObject_CheckCollisionInDirection(object, direction);
    }
    movementKind = MOVEMENT_STEP_UP;
    if (collision != 0) {
        movementKind = 32;
    }
    MapObject_ForceSetHeldMovement(object, GetMovementForDirection(direction, movementKind));
    if (MapObject_SpinsWhileWalking(object) == TRUE) {
        WalkingSpin_TurnBeforeStep(object, &state->spin);
    }
    MapObject_SetSingleMovement(object);
    state->state = 1;
    return TRUE;
}

static BOOL PathMovement_WaitStep(LocalMapObject *object, MovementPathState *state) {
    if (MapObject_RunForcedMovement(object) == TRUE) {
        MapObject_ClearSingleMovement(object);
        if (MapObject_SpinsWhileWalking(object) == TRUE) {
            WalkingSpin_EndStep(object, &state->spin);
        }
        state->state = 0;
    }
    return FALSE;
}

void PathMovement_InitNorthWestSouthEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 30);
}

void PathMovement_InitSouthEastNorthWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 31);
}

void PathMovement_InitWestSouthEastNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 32);
}

void PathMovement_InitEastNorthWestSouth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 33);
}

void PathMovement_InitNorthEastSouthWest(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 34);
}

void PathMovement_InitSouthWestNorthEast(LocalMapObject *object) {
    PathMovement_Init(object, 2, 1, 35);
}

void PathMovement_InitWestNorthEastSouth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 36);
}

void PathMovement_InitEastSouthWestNorth(LocalMapObject *object) {
    PathMovement_Init(object, 2, 0, 37);
}

static int DirectionList_Count(const int *list, int value) {
    int i = 0;
    while (list[i] != value) {
        i++;
    }
    GF_ASSERT(i != 0);
    return i;
}

static int DirectionList_GetRandom(const int *list, int value) {
    u16 random = LCRandom();
    int count = DirectionList_Count(list, value);
    return list[random % count];
}

static int GetRandomDirectionFromTable(int tableId, int value) {
    const int *list = GetDirectionTable(tableId);
    u16 random = LCRandom();
    int count = DirectionList_Count(list, value);
    return list[random % count];
}

static const int *GetDirectionTable(int id) {
    const MovementListEntry *entry = sMovementDirectionTables;
    int current = 0;

    while (current != 0x27) {
        if (current == id) {
            return entry->list;
        }
        entry++;
        current = entry->id;
    }
    GF_ASSERT(FALSE);
    return NULL;
}

static int MapObject_GetDirectionToNearbyPlayer(LocalMapObject *object) {
    PlayerAvatar *avatar;
    u32 type = MapObject_GetType(object);
    int i, movement, entry, x, z, distance, objectX, objectZ, maxZ, minX, minZ, maxX;
    u32 y;
    if (type != 1 && type != 2) {
        return DIR_NONE;
    }
    avatar = FieldSystem_GetPlayerAvatar(MapObject_GetFieldSystem(object));
    if (!PlayerAvatar_IsRunning(avatar)) {
        return DIR_NONE;
    }
    i = 0;
    movement = MapObject_GetMovement(object);
    do {
        entry = sPlayerAwareMovementTypes[i++];
    } while (entry != movement && entry != 255);
    if (movement != entry) {
        return DIR_NONE;
    }
    y = MapObject_GetPositionVectorYCoordUInt(PlayerAvatar_GetMapObject(avatar));
    if (y != MapObject_GetPositionVectorYCoordUInt(object)) {
        return DIR_NONE;
    }
    x = PlayerAvatar_GetXCoord(avatar);
    z = PlayerAvatar_GetZCoord(avatar);
    distance = MapObject_GetParam(object, 0);
    objectX = MapObject_GetXCoord(object);
    objectZ = MapObject_GetZCoord(object);
    minX = objectX - distance;
    maxX = objectX + distance;
    minZ = objectZ - distance;
    maxZ = objectZ + distance;
    if (minZ <= z && maxZ >= z && minX <= x && maxX >= x) {
        return GetDirectionTowards(objectX, objectZ, x, z);
    }
    return DIR_NONE;
}

static int MapObject_GetAllowedDirectionToNearbyPlayer(LocalMapObject *object, int tableId, int value) {
    const int *list;
    int count;
    int i;
    int xDirection;
    int zDirection;
    int objectX;
    int objectZ;
    PlayerAvatar *avatar;
    int playerX;
    int playerZ;
    u32 direction;

    list = GetDirectionTable(tableId);
    count = DirectionList_Count(list, value);
    if (count == 1) {
        return DIR_NONE;
    }
    direction = MapObject_GetDirectionToNearbyPlayer(object);
    if (direction == DIR_NONE) {
        return direction;
    }
    i = 0;
    do {
        if (list[i] == direction) {
            return direction;
        }
        i++;
    } while (i < count);
    xDirection = DIR_NONE;
    zDirection = DIR_NONE;
    objectX = MapObject_GetXCoord(object);
    objectZ = MapObject_GetZCoord(object);
    avatar = FieldSystem_GetPlayerAvatar(MapObject_GetFieldSystem(object));
    playerX = PlayerAvatar_GetXCoord(avatar);
    playerZ = PlayerAvatar_GetZCoord(avatar);
    if (objectX > playerX) {
        xDirection = DIR_WEST;
    } else if (objectX < playerX) {
        xDirection = DIR_EAST;
    }
    if (objectZ > playerZ) {
        zDirection = DIR_NORTH;
    } else if (objectZ < playerZ) {
        zDirection = DIR_SOUTH;
    }
    i = 0;
    if (xDirection == DIR_NONE) {
        do {
            if (list[i] == zDirection) {
                return zDirection;
            }
            i++;
        } while (i < count);
    } else if (zDirection == DIR_NONE) {
        do {
            if (list[i] == xDirection) {
                return xDirection;
            }
            i++;
        } while (i < count);
    } else {
        do {
            if (list[i] == xDirection) {
                return xDirection;
            }
            if (list[i] == zDirection) {
                return zDirection;
            }
            i++;
        } while (i < count);
    }
    return DIR_NONE;
}

static BOOL MapObject_SpinsWhileWalking(LocalMapObject *object) {
    u32 type = MapObject_GetType(object) - 7;
    if (type <= 1) {
        return TRUE;
    }
    return FALSE;
}

static void WalkingSpin_Init(LocalMapObject *object, WalkingSpinState *state) {
    if (MapObject_GetType(object) == 7) {
        state->order = 0;
    } else {
        state->order = 1;
    }
}

static void WalkingSpin_TurnBeforeStep(LocalMapObject *object, WalkingSpinState *state) {
    u32 facing = MapObject_GetFacingDirection(object);
    int i = 0;
    while (i < DIR_MAX && facing != sWalkingSpinOrders[state->order][i]) {
        i++;
    }
    GF_ASSERT(i < DIR_MAX);
    state->facing = facing;
    i = (i + 1) % DIR_MAX;
    facing = sWalkingSpinOrders[state->order][i];
    if (MapObject_GetFlagsBitsMask(object, MAPOBJECTFLAG_UNK7)) {
        state->wasTurning = 1;
    } else {
        state->wasTurning = 0;
    }
    MapObject_SetFacingDirection(object, facing);
    MapObject_SetFlagsBits(object, MAPOBJECTFLAG_UNK7);
}

static void WalkingSpin_EndStep(LocalMapObject *object, WalkingSpinState *state) {
    if (state->wasTurning == 0) {
        MapObject_ClearFlagsBits(object, MAPOBJECTFLAG_UNK7);
    }
}
