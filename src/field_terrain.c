#include "field_terrain.h"

#include "global.h"

#include "field/bdhc.h"
#include "field/map_load_manager.h"
#include "field/map_load_manager_internal.h"
#include "field/map_prop.h"

#include "field_metatile_query_internal.h"
#include "field_system.h"
#include "gymmick_collision.h"
#include "heap.h"
#include "map_matrix.h"
#include "terrain_attributes.h"

// Attribute bits: 0-7 metatile behavior, 15 collision.

static int Terrain_AbsDiff(int lhs, int rhs);
static int FieldSystem_GetHeightStep(FieldSystem *fieldSystem, VecFx32 *playerPos, int x, int z, u8 *outKind);

static int Terrain_CalcHeight(FieldSystem *fieldSystem, int objectHeightMode, int objectHeight, fx32 x, fx32 z, u8 *outKind);
static int Terrain_GetHeight_Simple(FieldSystem *fieldSystem, int objectHeight, fx32 x, fx32 z, u8 *outKind);
static BOOL Terrain_GetAttributes_Loaded(FieldSystem *fieldSystem, int x, int z, u16 *outAttr);
static BOOL Terrain_GetAttributes_Simple(FieldSystem *fieldSystem, int x, int z, u16 *outAttr);

static BOOL MapProp_IsInRect(MapProp *mapProp, fx32 *rect, VecFx32 *offset);
static BOOL MapModelNo_IsInTableD0ToD3(u16 modelNo);

static const u16 sMapModelsD0ToD3[] = { 0xD0, 0xD1, 0xD2, 0xD3 };

static const FieldTerrainQueries sTerrainQueries_Bdhc = {
    .getHeight = Terrain_GetHeight_Bdhc,
    .getAttributes = Terrain_GetAttributes_Loaded,
};

static const u16 sMapModelsWithoutAreaMapAnimation[] = { 0xD0, 0xD2, 0xD3, 0 };

static const FieldTerrainQueries sTerrainQueries_Simple = {
    .getHeight = Terrain_GetHeight_Simple,
    .getAttributes = Terrain_GetAttributes_Simple,
};

static int Terrain_AbsDiff(int lhs, int rhs) {
    if (lhs >= rhs) {
        return lhs - rhs;
    }
    return rhs - lhs;
}

static int Terrain_CalcHeight(FieldSystem *fieldSystem, int objectHeightMode, int objectHeight, fx32 objectX, fx32 objectZ, u8 *newObjectHeightSourceOut) {
    BOOL dynHeightPlateFound;
    u32 tileX, tileZ;
    u32 mapMatrixX, mapMatrixZ;
    VecFx32 objectPosition;

    int mapMatrixWidth;
    int mapMatrixWidthTiles;
    MapLoadManager *landDataMan;
    BOOL bdhcHeightValid;
    fx32 newObjectHeight = 0;

    objectPosition.x = objectX;
    objectPosition.z = objectZ;
    objectPosition.y = 0;

    landDataMan = fieldSystem->mapLoadManager;
    mapMatrixWidth = MapMatrix_GetWidth(fieldSystem->mapMatrix);
    mapMatrixWidthTiles = mapMatrixWidth * MAP_TILES_COUNT_X;

    tileX = objectX / MAP_TILE_SIZE;
    tileZ = objectZ / MAP_TILE_SIZE;

    u8 dynHeightPlateIndex;
    dynHeightPlateFound = DynamicTerrainHeightManager_FindPlate(tileX, tileZ, fieldSystem->dynamicTerrainHeightManager, &dynHeightPlateIndex);

    mapMatrixX = tileX / MAP_TILES_COUNT_X;
    mapMatrixZ = tileZ / MAP_TILES_COUNT_Z;
    u32 mapMatrixIndex = mapMatrixX + mapMatrixZ * mapMatrixWidth;

    fx32 currentMapOriginX = (mapMatrixX * MAP_TILES_COUNT_X + MAP_TILES_COUNT_X / 2) * MAP_TILE_SIZE;
    fx32 currentMapOriginZ = (mapMatrixZ * MAP_TILES_COUNT_Z + MAP_TILES_COUNT_Z / 2) * MAP_TILE_SIZE;

    objectPosition.x = objectX - currentMapOriginX;
    objectPosition.z = objectZ - currentMapOriginZ;

    u32 tileIndex = tileX + tileZ * mapMatrixWidthTiles;
    int tileMapQuadrant = TileIndex_GetQuadrant(tileIndex, mapMatrixWidthTiles);
    u32 loadedMapIndex = MapLoadManager_GetMapSlotOfMatrixIndex(mapMatrixIndex, tileMapQuadrant, landDataMan);

    if (loadedMapIndex > MAP_LOAD_NUM_SLOTS - 1) {
        bdhcHeightValid = FALSE;
    } else {
        Bdhc *bdhc = MapLoadManager_GetBdhc(landDataMan, loadedMapIndex);
        bdhcHeightValid = Bdhc_GetHeight(objectHeightMode, objectHeight, objectPosition.x, objectPosition.z, bdhc, &objectPosition.y);
    }

    u8 newObjectHeightSource;

    if (dynHeightPlateFound) {
        fx32 dynamicHeight = DynamicTerrainHeightManager_GetPlateHeight(dynHeightPlateIndex, fieldSystem->dynamicTerrainHeightManager);

        if (bdhcHeightValid) {
            if (dynamicHeight <= objectPosition.y) {
                newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_BDHC;
                newObjectHeight = objectPosition.y;
            } else {
                fx32 bdhcHeightDiff = Terrain_AbsDiff(objectPosition.y, objectHeight);
                fx32 dynamicHeightDiff = Terrain_AbsDiff(dynamicHeight, objectHeight);

                if (bdhcHeightDiff <= dynamicHeightDiff) {
                    newObjectHeight = objectPosition.y;
                    newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_BDHC;
                } else {
                    newObjectHeight = dynamicHeight;
                    newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_DYNAMIC;
                }
            }
        } else {
            newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_DYNAMIC;
            newObjectHeight = dynamicHeight;
        }
    } else {
        if (bdhcHeightValid) {
            newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_BDHC;
            newObjectHeight = objectPosition.y;
        } else {
            newObjectHeightSource = CALCULATED_HEIGHT_SOURCE_NONE;
        }
    }

    if (newObjectHeightSourceOut != NULL) {
        *newObjectHeightSourceOut = newObjectHeightSource;
    }

    return newObjectHeight;
}

int Terrain_GetHeight_Bdhc(FieldSystem *fieldSystem, int objectHeight, fx32 x, fx32 z, u8 *outKind) {
    return Terrain_CalcHeight(fieldSystem, 0, objectHeight, x, z, outKind);
}

int Terrain_CalcHeightWithMode(FieldSystem *fieldSystem, int objectHeightMode, int objectHeight, fx32 x, fx32 z, u8 *outKind) {
    return Terrain_CalcHeight(fieldSystem, objectHeightMode, objectHeight, x, z, outKind);
}

static int Terrain_GetHeight_Simple(FieldSystem *fieldSystem, int objectHeight, fx32 x, fx32 z, u8 *outKind) {
    BOOL ok = MapLoadManager_GetMapSlotAt(fieldSystem->mapLoadManager, x / MAP_TILE_SIZE, z / MAP_TILE_SIZE, NULL);
    if (outKind != NULL) {
        *outKind = ok ? CALCULATED_HEIGHT_SOURCE_BDHC : CALCULATED_HEIGHT_SOURCE_NONE;
    }
    return 0;
}

static BOOL Terrain_GetAttributes_Loaded(FieldSystem *fieldSystem, int x, int z, u16 *outAttr) {
    MapLoadManager *mapLoadManager = fieldSystem->mapLoadManager;
    u8 idx;
    u16 *attrs;
    if (!MapLoadManager_GetMapSlotAt(mapLoadManager, x, z, &idx)) {
        *outAttr = 0xFF;
        return FALSE;
    }
    attrs = MapLoadManager_GetPermissions(mapLoadManager, idx);
    *outAttr = attrs[x % 32 + z % 32 * 32];
    return TRUE;
}

static BOOL Terrain_GetAttributes_Simple(FieldSystem *fieldSystem, int x, int z, u16 *outAttr) {
    const u16 *attrs = TerrainAttributes_Get(x / 32 + z / 32 * MapMatrix_GetWidth(fieldSystem->mapMatrix), fieldSystem->terrainAttributes);
    *outAttr = attrs[x % 32 + z % 32 * 32];
    return TRUE;
}

SoundplateStruct *FieldSystem_GetSoundplateAt(FieldSystem *fieldSystem, int x, int z) {
    MapLoadManager *mapLoadManager = fieldSystem->mapLoadManager;
    u8 idx;
    if (MapLoadManager_GetMapSlotAt(mapLoadManager, x, z, &idx) == FALSE) {
        return NULL;
    }
    return MapLoadManager_GetSoundplates(mapLoadManager, idx);
}

void FieldSystem_SetTerrainQueries(const FieldTerrainQueries **queries, int mode) {
    if (mode == 0) {
        *queries = &sTerrainQueries_Bdhc;
    } else if (mode == 1) {
        *queries = &sTerrainQueries_Simple;
    } else {
        GF_ASSERT(FALSE);
    }
}

BOOL FieldSystem_CheckTileCollision(FieldSystem *fieldSystem, int x, int z) {
    u16 attr;
    if (fieldSystem->terrainQueries->getAttributes(fieldSystem, x, z, &attr)) {
        if ((u8)((u8)(attr >> 15) & 1) == 1) {
            return TRUE;
        }
    }
    return FALSE;
}

u8 FieldSystem_GetTileAttrBits8To14(FieldSystem *fieldSystem, int x, int z) {
    u16 attr;
    if (fieldSystem->terrainQueries->getAttributes(fieldSystem, x, z, &attr)) {
        return (u8)(attr >> 8) & 0x7F;
    }
    return 0;
}

u8 GetMetatileBehavior(FieldSystem *fieldSystem, int x, int z) {
    u16 attr;
    int behavior;
    if (fieldSystem->terrainQueries->getAttributes(fieldSystem, x, z, &attr)) {
        behavior = (u8)attr;
        return (u8)behavior;
    }
    return 0xFF;
}

int FieldSystem_GetTerrainHeight(FieldSystem *fieldSystem, int objectHeight, fx32 x, fx32 z, u8 *outKind) {
    return fieldSystem->terrainQueries->getHeight(fieldSystem, objectHeight, x, z, outKind);
}

static int FieldSystem_GetHeightStep(FieldSystem *fieldSystem, VecFx32 *playerPos, int x, int z, u8 *outKind) {
    u32 dir;
    fx32 hi, lo;
    fx32 height = FieldSystem_GetTerrainHeight(fieldSystem, playerPos->y, (x << 16) + FX32_CONST(8), (z << 16) + FX32_CONST(8), outKind);
    if (height < playerPos->y) {
        hi = playerPos->y;
        lo = height;
        dir = -1;
    } else if (height > playerPos->y) {
        hi = height;
        lo = playerPos->y;
        dir = 1;
    } else {
        return 0;
    }
    if (hi - lo >= FX32_CONST(20)) {
        GF_ASSERT(dir != 0);
    } else {
        dir = 0;
    }
    return dir;
}

BOOL FieldSystem_CheckCollisionAndHeight(FieldSystem *fieldSystem, VecFx32 *playerPos, int xInFront, int yInFront, s8 *outSlope) {
    u8 kind;
    BOOL ret;
    int slope = FieldSystem_GetHeightStep(fieldSystem, playerPos, xInFront, yInFront, &kind);
    if (outSlope != NULL) {
        *outSlope = slope;
    }
    if (slope == 0) {
        ret = FieldSystem_CheckTileCollision(fieldSystem, xInFront, yInFront);
        if (!ret && kind == CALCULATED_HEIGHT_SOURCE_DYNAMIC) {
            GetMetatileBehavior(fieldSystem, xInFront, yInFront);
        }
        return ret;
    }
    return TRUE;
}

u32 FieldSystem_CheckCollisionHeightAndGimmick(FieldSystem *fieldSystem, VecFx32 *playerPos, u32 x, u32 y, u32 *outSlope) {
    u8 kind;
    BOOL ret;
    int slope = FieldSystem_GetHeightStep(fieldSystem, playerPos, x, y, &kind);
    if (outSlope != NULL) {
        *(u8 *)outSlope = (u8)slope;
    }
    if (slope == 0) {
        if (!Gymmick_CheckCollision(fieldSystem, x, y, playerPos->y, &ret)) {
            ret = FieldSystem_CheckTileCollision(fieldSystem, x, y);
            if (!ret && kind == CALCULATED_HEIGHT_SOURCE_DYNAMIC) {
                GetMetatileBehavior(fieldSystem, x, y);
            }
            return ret;
        }
        return ret;
    }
    return TRUE;
}

void Terrain_MakeTileRect(int x, int z, int offsetX, int offsetZ, int width, int depth, fx32 *out) {
    int x1 = x + offsetX;
    int z1 = z + offsetZ;
    int x2 = x1 + width;
    int z2 = z1 + depth;
    GF_ASSERT(x1 >= 0 && z1 >= 0 && x2 >= 0 && z2 >= 0);
    out[0] = x1 << 16;
    out[1] = z1 << 16;
    out[2] = x2 << 16;
    out[3] = z2 << 16;
}

static BOOL MapProp_IsInRect(MapProp *mapProp, fx32 *rect, VecFx32 *offset) {
    VecFx32 pos;
    MapProp_GetTranslation(&pos, mapProp);
    pos.x = pos.x + offset->x;
    pos.z = pos.z + offset->z;
    if (rect[0] <= pos.x && pos.x <= rect[2] && rect[1] <= pos.z && pos.z <= rect[3]) {
        return TRUE;
    }
    return FALSE;
}

BOOL FieldSystem_FindMapPropInRect(FieldSystem *fieldSystem, int buildModel, fx32 *rect, MapProp **outProp) {
    u8 i;
    u8 j;
    MapPropManager *mgr;
    VecFx32 pos;
    MapProp *prop;
    int idx;
    int model;
    for (i = 0; i < 4; i++) {
        MapLoadManager_GetMapPropManager(i, fieldSystem->mapLoadManager, &mgr);
        if (mgr == NULL) {
            continue;
        }
        idx = MapLoadManager_GetMapMatrixIndex(fieldSystem->mapLoadManager, i);
        Terrain_GetChunkOrigin(idx, MapMatrix_GetWidth(fieldSystem->mapMatrix), &pos);
        for (j = 0; j < 32; j++) {
            prop = MapPropManager_GetMapPropByIndex(mgr, j);
            if (MapProp_IsInRect(prop, rect, &pos)) {
                model = MapProp_GetBuildModel(prop);
                if (model == buildModel) {
                    if (outProp != NULL) {
                        *outProp = prop;
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

BOOL FieldSystem_FindMapPropOfModelsInRect(FieldSystem *fieldSystem, int *models, u32 count, fx32 *rect, MapProp **outProp, int *outModel) {
    u8 i;
    u8 j;
    u8 k;
    MapPropManager *mgr;
    VecFx32 pos;
    MapProp *prop;
    int idx;
    int model;
    for (i = 0; i < 4; i++) {
        MapLoadManager_GetMapPropManager(i, fieldSystem->mapLoadManager, &mgr);
        if (mgr == NULL) {
            continue;
        }
        idx = MapLoadManager_GetMapMatrixIndex(fieldSystem->mapLoadManager, i);
        Terrain_GetChunkOrigin(idx, MapMatrix_GetWidth(fieldSystem->mapMatrix), &pos);
        for (j = 0; j < 32; j++) {
            prop = MapPropManager_GetMapPropByIndex(mgr, j);
            if (MapProp_IsInRect(prop, rect, &pos)) {
                model = MapProp_GetBuildModel(prop);
                for (k = 0; k < count; k++) {
                    if (model == models[k]) {
                        if (outProp != NULL) {
                            *outProp = prop;
                        }
                        if (outModel != NULL) {
                            *outModel = model;
                        }
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

BOOL FieldSystem_FindMapProp(FieldSystem *fieldSystem, int buildModel, MapProp **outProp, int *outIdx) {
    u8 i;
    u8 j;
    MapPropManager *mgr;
    MapProp *prop;
    int model;
    for (i = 0; i < 4; i++) {
        MapLoadManager_GetMapPropManager(i, fieldSystem->mapLoadManager, &mgr);
        if (mgr == NULL) {
            continue;
        }
        for (j = 0; j < 32; j++) {
            prop = MapPropManager_GetMapPropByIndex(mgr, j);
            model = MapProp_GetBuildModel(prop);
            if (model == buildModel) {
                if (outProp != NULL) {
                    *outProp = prop;
                }
                if (outIdx != NULL) {
                    *outIdx = MapLoadManager_GetMapMatrixIndex(fieldSystem->mapLoadManager, i);
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL FieldSystem_FindMapPropOfModels(FieldSystem *fieldSystem, int *models, u32 count, MapProp **outProp, int *outModel) {
    u8 i;
    u8 j;
    u8 k;
    MapPropManager *mgr;
    MapProp *prop;
    int model;
    for (i = 0; i < 4; i++) {
        MapLoadManager_GetMapPropManager(i, fieldSystem->mapLoadManager, &mgr);
        if (mgr == NULL) {
            continue;
        }
        for (j = 0; j < 32; j++) {
            prop = MapPropManager_GetMapPropByIndex(mgr, j);
            model = MapProp_GetBuildModel(prop);
            for (k = 0; k < count; k++) {
                if (model == models[k]) {
                    if (outProp != NULL) {
                        *outProp = prop;
                    }
                    if (outModel != NULL) {
                        *outModel = model;
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

int *FieldSystem_CollectMapPropModelsInRect(FieldSystem *fieldSystem, enum HeapID heapId, int capacity, fx32 *rect, u32 fill) {
    u8 outer;
    int *array = Heap_AllocAtEnd(heapId, capacity * sizeof(int));
    int i;
    u8 count;

    for (i = 0; i < capacity; i++) {
        array[i] = fill;
    }
    count = 0;
    for (outer = 0; outer < 4; outer++) {
        MapPropManager *manager;
        MapLoadManager_GetMapPropManager(outer, fieldSystem->mapLoadManager, &manager);
        if (manager != NULL) {
            int index = MapLoadManager_GetMapMatrixIndex(fieldSystem->mapLoadManager, outer);
            VecFx32 offset;
            Terrain_GetChunkOrigin(index, MapMatrix_GetWidth(fieldSystem->mapMatrix), &offset);
            u8 inner;
            for (inner = 0; inner < 32; inner++) {
                MapProp *prop = MapPropManager_GetMapPropByIndex(manager, inner);
                if (MapProp_IsInRect(prop, rect, &offset)) {
                    int model = MapProp_GetBuildModel(prop);
                    if (model != 0) {
                        if ((u32)count >= (u32)capacity) {
                            GF_ASSERT(FALSE);
                            return array;
                        }
                        array[count++] = model;
                    }
                }
            }
        }
    }
    return array;
}

void Terrain_GetChunkOrigin(int idx, int width, VecFx32 *pos) {
    pos->x = FX32_CONST(256);
    pos->z = FX32_CONST(256);
    pos->x = pos->x + ((u16)(idx % width) << 21);
    pos->z = pos->z + ((u16)(idx / width) << 21);
}

static BOOL MapModelNo_IsInTableD0ToD3(u16 modelNo) {
    int i;
    const u16 *entry = sMapModelsD0ToD3;
    for (i = 0; i < 4; i++) {
        if (modelNo == *entry) {
            return TRUE;
        }
        entry++;
    }
    return FALSE;
}

BOOL MapModelNo_HasNoAreaMapAnimation(u16 modelNo) {
    int i;
    const u16 *entry;
    if (!MapModelNo_IsInTableD0ToD3(modelNo)) {
        return FALSE;
    }
    entry = sMapModelsWithoutAreaMapAnimation;
    for (i = 0; i < 3; i++) {
        if (modelNo == *entry) {
            return TRUE;
        }
        entry++;
    }
    return FALSE;
}
