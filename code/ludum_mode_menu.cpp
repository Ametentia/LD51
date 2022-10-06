struct Mode_Menu {
    Game_State *state;

    f32 div_percent;
    b32 start_hot;
    b32 last_start_hot;
};

function void ModePlay(Game_State *state);

function void ModeMenu(Game_State *state) {
    Reset(&state->mode_arena);

    Mode_Menu *menu = AllocType(&state->mode_arena, Mode_Menu);
    if (menu) {
        menu->state = state;

        state->mode = GameMode_Menu;
        state->menu = menu;
    }

}

function b32 InBox(rect2 b, v2 p) {
    b32 result =
        (b.min.x <= p.x) && (b.max.x >= p.x) &&
        (b.min.y <= p.y) && (b.max.y >= p.y);

    return result;
}

function void LudumModeMenuUpdateRender(Mode_Menu *menu, Input *input, Renderer_Buffer *rbuffer) {
    Game_State *state = menu->state;

    Draw_Batch _batch = { 0 };
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, rbuffer);

    f32 dt = input->delta_time;

    v4 floor_c = V4(SQUARE(7/255.0f), SQUARE(24/255.0f), SQUARE(22/255.0f), 1.0);

    DrawClear(batch, floor_c);

    SetCameraTransform(batch, 0, V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 8));

    rect3 r = GetCameraFrustum(&batch->game_tx);

    v3 mouse_world = Unproject(&batch->game_tx, input->mouse_clip);

    rect2 bounds;
    bounds.min = r.min.xy;
    bounds.max = r.max.xy;

    Image_Handle bg = GetImageByName(&state->assets, "background");
    Image_Handle title = GetImageByName(&state->assets, "title");

    Image_Handle start = GetImageByName(&state->assets, "start_game");
    Image_Handle exit  = GetImageByName(&state->assets, "exit");
    Image_Handle div   = GetImageByName(&state->assets, "divider");

    v2 dim = bounds.max - bounds.min;
    dim.y  = -dim.y;

    DrawQuad(batch, bg, 0.5f * (bounds.max + bounds.min), dim);
    DrawQuad(batch, title, V2(-1.35, -0.96), 1.8);

    f32 start_scale = 1.4f;
    v2 start_dim    = GetScaledImageDim(GetImageInfo(&state->assets, start));
    v2 start_p      = V2(1.13, 0.3);

    f32 exit_scale = 0.5f;
    v2 exit_dim    = GetScaledImageDim(GetImageInfo(&state->assets, exit));
    v2 exit_p      = V2(1.13, 0.8);

    rect2 start_box;
    start_box.min = start_p - (0.5f * start_scale * start_dim);
    start_box.max = start_p + (0.5f * start_scale * start_dim);

    rect2 exit_box;
    exit_box.min = exit_p - (0.5f * exit_scale * exit_dim);
    exit_box.max = exit_p + (0.5f * exit_scale * exit_dim);

    DrawQuad(batch, exit,  exit_p,  exit_scale);
    DrawQuad(batch, start, start_p, start_scale);

    if (InBox(start_box, mouse_world.xy)) {
        v2 div_dim = GetScaledImageDim(GetImageInfo(&state->assets, div));

        menu->div_percent += (8 * dt);
        menu->div_percent = Min(menu->div_percent, 1.67f);

        DrawQuad(batch, div, start_p - V2(0, start_dim.y), V2(menu->div_percent * div_dim.x, div_dim.y));
        DrawQuad(batch, div, start_p + V2(0, start_dim.y), V2(menu->div_percent * div_dim.x, div_dim.y));

        menu->start_hot = true;

        if (JustPressed(input->mouse_buttons[0])) {
            ModePlay(state);
        }
    }
    else if (InBox(exit_box, mouse_world.xy)) {
        if (menu->last_start_hot) { menu->div_percent = 0; }

        v2 div_dim = GetScaledImageDim(GetImageInfo(&state->assets, div));

        menu->div_percent += (6 * dt);
        menu->div_percent = Min(menu->div_percent, 0.67f);

        DrawQuad(batch, div, exit_p - V2(0, 0.34 * exit_dim.y), V2(menu->div_percent * div_dim.x, div_dim.y));
        DrawQuad(batch, div, exit_p + V2(0, 0.34 * exit_dim.y), V2(menu->div_percent * div_dim.x, div_dim.y));

        if (JustPressed(input->mouse_buttons[0])) {
            input->requested_quit = true;
        }
    }
    else {
        menu->div_percent = 0;
    }

    menu->last_start_hot = menu->start_hot;
    menu->start_hot = false;
}
