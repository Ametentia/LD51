#if !defined(LUDUM_H_)
#define LUDUM_H_

#define LUDAM_INTERNAL 1
#include <base.h>

#include "ludum_kiz.h"

enum Game_Mode : u32 {
    GameMode_Menu = 0,
    GameMode_Play,
};

struct Mode_Menu;
struct Mode_Play;

struct Game_State {
    Memory_Arena perm;
    Memory_Arena mode_arena;

    Audio_State   audio;
    Asset_Manager assets;

    Game_Mode mode;
    union {
        Mode_Play *play;
        Mode_Menu *menu;
    };
};

#endif  // LUDUM_H_
