#include <stdio.h>

#include "ludum.h"
#include "ludum.cpp"

#include <platform/windows.h>
#include <platform/windows.cpp>

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    Windows_Parameters init_params = {};
    init_params.init_flags   = PlatformInit_OpenWindow | PlatformInit_EnableAudio;
    init_params.show_cmd     = SW_SHOW;
    init_params.window_dim   = V2U(1280, 720);
    init_params.window_title = WrapConst("LD51");

    if (!WindowsInitialise(&init_params)) {
        OutputDebugString("[Error] :: Failed to initialise\n");
        return 1;
    }

    // Load and initialise the renderer
    //
    Renderer_Parameters renderer_params = {};
    renderer_params.command_buffer_size = Megabytes(1);
    renderer_params.max_immediate_quads = (1 << 18);

    renderer_params.max_texture_handles = 512;
    renderer_params.texture_queue_size  = Megabytes(512);

    Renderer_Context *renderer = WindowsLoadRenderer(&renderer_params);
    if (!IsValid(renderer)) {
        OutputDebugString("[Error] :: Failed to create renderer\n");
        return 1;
    }

    // Setup input structure
    //
    Input _input = {};
    Input *input = &_input;

    // Main game state init
    //
    Memory_Allocator *alloc = Platform->GetMemoryAllocator();
    Game_State *state = AllocInline(alloc, Megabytes(128), Game_State, perm);

    LudumInit(state, &renderer->texture_queue);

    b32 running = true;
    while (running) {
        WindowsHandleInput(input);
        if (input->requested_quit) { running = false; }

        v2u window_dim = WindowsGetWindowDim();

        rect2 render_region = {};
        render_region.min = V2(0, 0);
        render_region.max = V2(window_dim);

        Renderer_Buffer *renderer_buffer = renderer->BeginFrame(renderer, window_dim, render_region);
        LudumUpdateRender(state, input, renderer_buffer);

        renderer->SubmitFrame(renderer);

        //Game_State *state = context->state;

        //Audio_Buffer audio_buffer = {};
        //WindowsGetAudioBuffer(&audio_buffer);
        // MixPlayingSounds(&state->audio_state, &state->assets,  &audio_buffer);
        //WindowsSubmitAudioBuffer(&audio_buffer);
    }

    renderer->Shutdown(renderer);

    return 0;

}
