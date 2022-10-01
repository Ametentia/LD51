struct Mode_Play {
    Game_State *state;
    Memory_Arena *arena;

    v2 camera_pos;
    rect2 camera_region;

    f32 dash_timer;
    b32 dashing;
    b32 can_jump;
    v2 dp;
    v2 player_pos;
};

function void ModePlay(Game_State *state) {
    Reset(&state->mode_arena);

    Mode_Play *play = AllocType(&state->mode_arena, Mode_Play);
    if (play) {
        play->state = state;
        play->arena = &state->mode_arena;

        play->camera_region.min = V2(-1, -1);
        play->camera_region.max = V2( 1,  1);

        // We don't have to zero init anything because it is cleared by AllocType
        //
        state->mode = GameMode_Play;
        state->play = play;
    }
}

function void LudumModePlayUpdateRender(Mode_Play *play, Input *input, Renderer_Buffer *rbuffer) {
    Game_State *state = play->state;

    v2 dir = V2(0, 0);

    Draw_Batch _batch = { 0 };
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, rbuffer);

    DrawClear(batch, V4(0.25, 0.25, 0.25, 1.0));

    f32 dt = input->delta_time;

    f32 gravity = (2 * 1.2) / (0.5 * 0.5);
    v2 ddp = V2(0, gravity);

    if (play->can_jump && JustPressed(input->keys[Key_K])) {
        play->dp.y = -Sqrt(2 * gravity * 1.2);
        play->can_jump = false;
    }

    if (IsPressed(input->keys[Key_A])) {
        ddp.x = -10;
    }

    if (IsPressed(input->keys[Key_D])) {
        ddp.x = 10;
    }

    if (IsZero(ddp.x) && !play->dashing) {
        play->dp.x *= (1.0f / (1 + (18.5 * dt)));
    }
    else if (play->dashing) {
        play->dp.x *= (1.0f / (1 + (30.5 * dt)));
        play->dp.y *= (1.0f / (1 + (30.5 * dt)));

        play->dash_timer += dt;
        if (play->dash_timer > 0.33) {
            play->dashing = false;
            play->dash_timer = 0;
        }
    }

    play->player_pos += dt * play->dp;
    play->dp += dt * ddp;

    if (play->player_pos.y > 5.0) {
        play->player_pos.y = 5.0;
        if (play->dp.y > 0) {
            play->dp.y = 0;
        }

        play->can_jump = true;
        play->dashing = false;
    }

    v3 camera_p = V3(play->player_pos.x, play->player_pos.y - 0.8, 8);
    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), camera_p);

    v2 mouse_world = Unproject(&batch->game_tx, input->mouse_clip).xy;

    v2 dash_dir = Noz(mouse_world - play->player_pos);
    if (JustPressed(input->keys[Key_J])) {
        play->dp = 100 * dash_dir;
        play->dashing = true;
    }

    DrawQuad(batch, { 0 }, mouse_world, V2(0.1, 0.1));

    for (f32 i = -0.5; i > -100; i -= 5) {
        f32 x = Max(0.09 * Abs(i), 1);
        DrawQuad(batch, { 0 }, V3(0.0, 0.0, i), V2(0.8, 0.8), 0, V4(1/x, 0, 1/x, 1));
    }

    DrawQuad(batch, { 0 }, play->player_pos - V2(0, 0.2), V2(0.4, 0.4), 0, V4(0, 1, 1, 1));
    DrawQuad(batch, { 0 }, V2(0, 7.5), V2(25, 5));
}
