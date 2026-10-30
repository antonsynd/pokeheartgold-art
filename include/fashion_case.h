#ifndef POKEHEARTGOLD_FASHION_CASE_H
#define POKEHEARTGOLD_FASHION_CASE_H

#include "mail_message.h"

typedef struct SaveFashionDataSub {
    u32 magic;
    u32 personality;
    u32 otId;
    u16 species;
    u16 nickname[11];
    u16 secondText[8];
    s8 z;
    u8 x;
    u8 y;
    u8 form;
    u8 secondTextGender;
    u8 padding_39[3];
    u32 itemMask;
    MailMessage mailMessage;
    u8 items[10][4];
    u8 backdrop;
    u8 language;
    u8 padding_72[2];
} SaveFashionDataSub; // size=0x74

typedef struct SaveFashionDataSub3FC {
    u8 filler_00[0x98];
} SaveFashionDataSub3FC; // size=0x98

typedef struct FashionCase {
    u32 accessories[40 / sizeof(u32)];
    u32 wallpaperOrder[(18 + sizeof(u32) - 1) / sizeof(u32)];
    u8 padding_3C[4];
} FashionCase;

typedef struct SaveFashionData {      // todo: rename to SaveFashionData
    SaveFashionDataSub entries[11];   // 000
    SaveFashionDataSub3FC largeEntries[5]; // 4FC
    FashionCase fashionCase;          // 7F4
} SaveFashionData;                    // size=0x834

SaveFashionData *Save_FashionData_Get(SaveData *saveData);
FashionCase *Save_FashionData_GetFashionCase(SaveFashionData *unk);
u32 FashionCase_CountAccessories(FashionCase *fashionCase);
u32 FashionCase_CountWallpapers(FashionCase *fashionCase);
BOOL Save_FashionData_IsEntrySaved(SaveFashionData *fashionData, int index);
BOOL Save_FashionData_IsLargeEntrySaved(SaveFashionData *fashionData, int index);
SaveFashionDataSub *Save_FashionData_GetEntry(SaveFashionData *fashionData, int index);
void FashionEntry_SetMailMessageWord(SaveFashionDataSub *fashionDataSub, int value);
void FashionCase_GiveFashionItem(FashionCase *fashionCase, int id, int quantity);
int FashionCase_CanGiveAccessory(FashionCase *fashionCase, int id, int quantity);
u32 FashionCase_GetAccessoryCount(FashionCase *fashionCase, int id);
void FashionCase_GiveContestBackground(FashionCase *fashionCase, int id);
BOOL FashionCase_HasWallpaper(FashionCase *fashionCase, int index);
void FashionCase_TakeAccessory(FashionCase *fashionCase, int id, int quantity);

#endif // POKEHEARTGOLD_FASHION_CASE_H
