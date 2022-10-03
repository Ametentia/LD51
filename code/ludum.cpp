#include <base.cpp>

#include "ludum_util.cpp"
#include "ludum_mode_play.cpp"

function void LudumInit(Game_State *state, Texture_Transfer_Queue *texture_queue) {
    Memory_Allocator *alloc = Platform->GetMemoryAllocator();

    Initialise(&state->mode_arena, alloc, Gigabytes(2));
    Initialise(&state->assets, &state->perm, texture_queue);
    Initialise(&state->audio,  &state->perm);

    ModePlay(state);
}

function void LudumUpdateRender(Game_State *state, Input *input, Renderer_Buffer *rbuffer) {
    if (input->delta_time > (1.0f / 60.0f)) {
        input->delta_time = 1.0f / 60.0f;
    }

    switch (state->mode) {
        case GameMode_Play: {
            LudumModePlayUpdateRender(state->play, input, rbuffer);
        }
        break;
    }
}
