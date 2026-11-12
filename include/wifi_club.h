#ifndef POKEHEARTGOLD_WIFI_CLUB_H
#define POKEHEARTGOLD_WIFI_CLUB_H

#include "task.h"

// What the player chose in the Wi-Fi Club (WifiClubArgs.choice).
enum WifiClubChoice {
    WIFI_CLUB_BATTLE_SINGLE_LV50 = 1,
    WIFI_CLUB_BATTLE_SINGLE_LV100,
    WIFI_CLUB_BATTLE_SINGLE_FREE,
    WIFI_CLUB_BATTLE_DOUBLE_LV50,
    WIFI_CLUB_BATTLE_DOUBLE_LV100,
    WIFI_CLUB_BATTLE_DOUBLE_FREE,
    WIFI_CLUB_TRADE,
    WIFI_CLUB_EXIT,
    WIFI_CLUB_WIFI_SETTINGS,
    WIFI_CLUB_CANCEL,
    WIFI_CLUB_FRONTIER = 12,
    WIFI_CLUB_SWALOT_PLOP,
    WIFI_CLUB_MIME_JR_TOP,
    WIFI_CLUB_WOBBUFFET_POP,
};

BOOL WifiClub_IsMatchingActivity(int netId);
void CallTask_WifiClub(TaskManager *taskManager);
void CallTask_WifiClubWithResult(TaskManager *taskManager, u16 *result);

#endif // POKEHEARTGOLD_WIFI_CLUB_H
