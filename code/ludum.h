#if !defined(LUDUM_H_)
#define LUDUM_H_

#include <base.h>

enum Game_Mode : u32 {
    GameMode_Menu = 0,
    GameMode_Play,
};

struct Mode_Play;

struct Game_State {
    Memory_Arena perm;
    Memory_Arena mode_arena;

    Audio_State   audio;
    Asset_Manager assets;

    Game_Mode mode;
    union {
        Mode_Play *play;
    };
};

#endif  // LUDUM_H_
