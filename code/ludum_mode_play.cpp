enum Layers {
    Layer_Foreground0 = 6,
    Layer_Foreground1 = 4,
    Layer_Foreground2 = 2,
    Layer_Foreground3 = 0,

    Layer_Level = -2,

    Layer_Background3 = -4,
    Layer_Background2 = -6,
    Layer_Background1 = -8,
    Layer_Background0 = -10
};

enum Player_Flags {
    PlayerFlag_HoldAnimation = (1 << 0),
    PlayerFlag_HasJumped     = (1 << 1)
};

enum Animation_Slot {
    AnimationSlot_Idle,
    AnimationSlot_Walk,
    AnimationSlot_Jump,
    AnimationSlot_Fall,

    AnimationSlot_PreDash,
    AnimationSlot_Dash,
    AnimationSlot_PostDash,

    AnimationSlot_Count
};

#define PLAYER_LAYER 3

struct Player {
    v2 dp;
    v2 p;

    v2 dim;

    u32 flags;

    b32 on_ground;
    f32 last_jump_time;

    b32 can_dash;
    f32 dash_timer;
    v2 dash_dir;
    v2 dash_p;
    f32 dash_angle;

    u32 slot;
    Sprite_Animation animations[AnimationSlot_Count];

    f32 facing;
    v2 animation_scale;
};

struct Dust_Particle {
    Image_Handle image; // blurred or not blurred

    f32 scale;

    v2 p;
    v2 dp;

    f32 alpha_rate;
    f32 alpha;
};

enum Env_Object_Flags {
    EnvObjectFlag_Platform   = (1 << 0),
    EnvObjectFlag_Foreground = (1 << 1)
};

struct Env_Object {
    Image_Handle image;

    u32 flags;

    v2 p;
    f32 scale;
};

enum Editor_Mode {
    EditorMode_Select,
    EditorMode_Place,
};

#define MAX_LAYER_OBJECTS 64
#define MAX_LAYER_DUST    8

struct Layer {
    s32 z_value;

    u32 num_objects;
    Env_Object objects[MAX_LAYER_OBJECTS];

    u32 num_dust;
    Dust_Particle dust[MAX_LAYER_OBJECTS];

    // @Todo: add platforms etc.
};

#define MAX_LEVEL_LAYERS 8

struct Level {
    u32 front_layer;
    u32 back_layer;
    Layer layers[MAX_LEVEL_LAYERS];
};

struct Editor {
    b32 enabled;

    u32 mode;

    s32 type;

    f32 scale;
    s32 layer;

    Level *level;
    Env_Object *selected;
};

struct Mode_Play {
    Game_State *state;
    Memory_Arena *arena;

    Draw_Batch *batch;

    Player player;

    b32 active;
    f32 yaw;
    f32 pitch;
    f32 zoom;
    v3 target;

    f32 shake;
    v2  shake_offset;
    f32 shake_angle;

    v2 camera_p;
    v2 camera_dp;

    Level  level;
    Editor editor;

    u32 num_moves;
    u32 next_move;
    v2 movement_trail[256];

    Random rnd;
};

function void SimEditor(Mode_Play *play, Input *input);

#define SQUARE(x) ((x) * (x))

#define CAMERA_STIFFNESS 430
#define CAMERA_DAMPING   70

#define DASH_OFFSET 1.3
#define PLAYER_PRE_DASH_TIME (0.1f)
#define PLAYER_POST_DASH_TIME (0.6f)
#define PLAYER_DASH_TIME (0.2f)
#define PLAYER_TOTAL_DASH_TIME (PLAYER_PRE_DASH_TIME + PLAYER_DASH_TIME)

#define PLAYER_DASH_SPEED (550.3f)

#define PLAYER_MAX_JUMP_HEIGHT (1.68f)
#define PLAYER_MIN_JUMP_HEIGHT (0.9f)

#define PLAYER_MAX_DASH_DIST (3.43f)

#define PLAYER_DASH_STRAFE_SPEED (20.0f)

#define PLAYER_DASH_APEX_TIME  (0.8f)
#define PLAYER_JUMP_APEX_TIME   (0.4f)

#define PLAYER_JUMP_BUFFER_TIME (0.2f)
#define PLAYER_COYOTE_TIME      (0.2f)

#define PLAYER_DAMPING (44.5f)
#define PLAYER_DAMPING_AIR (3.1f)
#define PLAYER_DASH_DAMPING (3.0f)

#define PLAYER_MAX_SPEED_X (4.8f)
#define PLAYER_MAX_SPEED_Y (7.2f)

#define PLAYER_MOVE_SPEED       (10.0f)
#define PLAYER_AIR_STRAFE_SPEED (4.0f)

function b32 IsDash(u32 slot) {
    b32 result = (slot >= AnimationSlot_PreDash && slot <= AnimationSlot_PostDash);
    return result;
}

function v2 GetUnormalisedDashDirection(Input *input) {
    v2 result = V2(0, 0);
    if (IsPressed(input->keys[Key_W])) {
        result.y = -1;
    }
    else if (IsPressed(input->keys[Key_S])) {
        result.y = 1;
    }

    if (IsPressed(input->keys[Key_A])) {
        result.x = -1;
    }
    else if (IsPressed(input->keys[Key_D])) {
        result.x = 1;
    }

    return result;
}

function f32 GetDashAngle(Input *input) {
    f32 result = 0;

    b32 w = IsPressed(input->keys[Key_W]);
    b32 s = IsPressed(input->keys[Key_S]);
    b32 a = IsPressed(input->keys[Key_A]);
    b32 d = IsPressed(input->keys[Key_D]);

    if (w) {
        if      (a) { result = (5 * Pi32) / 4; }
        else if (d) { result = (7 * Pi32) / 4; }
        else        { result = (3 * Pi32) / 2; }
    }
    else if (s) {
        if (a)      { result = (3 * Pi32) / 4; }
        else if (d) { result = (    Pi32) / 4; }
        else        { result = (    Pi32) / 2; }
    }
    else if (a) {
        result = Pi32;
    }

    // Don't need to check d on its down because its 0 anyway
    //
    return result;
}

function void ModePlay(Game_State *state) {
    Reset(&state->mode_arena);

    Mode_Play *play = AllocType(&state->mode_arena, Mode_Play);
    if (play) {
        play->state = state;
        play->arena = &state->mode_arena;

        Player *player = &play->player;

        Image_Handle walk_sheet = GetImageByName(&state->assets, "run_cycle_230x260");
        Image_Handle idle_sheet = GetImageByName(&state->assets, "idle");
        Image_Handle jump_sheet = GetImageByName(&state->assets, "jump");
        Image_Handle fall_sheet = GetImageByName(&state->assets, "fall");

        Image_Handle pre_dash_sheet  = GetImageByName(&state->assets, "pre_dash");
        Image_Handle dash_sheet      = GetImageByName(&state->assets, "dash");

        Image_Handle post_dash_sheet = GetImageByName(&state->assets, "post_dash");

        Initialise(&player->animations[AnimationSlot_Idle],    idle_sheet,     3, 3, 8, 1.0f / 30.0f);
        Initialise(&player->animations[AnimationSlot_Walk],    walk_sheet,     3, 3, 8, 1.0f / 30.0f);
        Initialise(&player->animations[AnimationSlot_Jump],    jump_sheet,     2, 3,    1.0f / 30.0f);

        Initialise(&player->animations[AnimationSlot_PreDash],  pre_dash_sheet,  1, 17,    1.0f / 30.0f);
        Initialise(&player->animations[AnimationSlot_Dash],     dash_sheet,      4, 1,     1.0f / 30.0f);
        Initialise(&player->animations[AnimationSlot_PostDash], post_dash_sheet, 1, 17, 9, 1.0f / 30.0f);

        // Not really an animation but we
        Initialise(&player->animations[AnimationSlot_Fall], fall_sheet, 1, 1, 1.0f / 30.0f);

        player->dim = V2(0.78, 1);

        player->slot   = AnimationSlot_Idle;
        player->facing = 1;

        play->shake_offset = V2(0.1, 0.1);
        play->shake_angle  = 0; // @Todo: I don't know if this look better Pi32 / 256.0f;

        play->rnd = RandomSeed(3920483094823);

        play->editor.mode  = EditorMode_Place;
        play->editor.scale = 1;
        play->editor.layer = 0;
        play->editor.level = &play->level;

        // We don't have to zero init anything because it is cleared by AllocType
        //
        state->mode = GameMode_Play;
        state->play = play;
    }
}

function void UpdateDebugCamera(Mode_Play *play, Input *input, Draw_Batch *batch) {
    m4x4 view = YRotation(play->yaw) * XRotation(play->pitch);

    v3 x_axis = GetColumn(view, 0);
    v3 y_axis = GetColumn(view, 1);
    v3 z_axis = GetColumn(view, 2);

    if (IsPressed(input->keys[Key_Alt])) {
        if (IsPressed(input->mouse_buttons[1])) {
            v3 pan = input->mouse_delta.x * x_axis - input->mouse_delta.y * y_axis;
            play->target -= (0.5f * play->zoom) * pan;
        }
    }
    else if (IsPressed(input->mouse_buttons[0])) {
        play->yaw   -= input->mouse_delta.x;
        play->pitch -= input->mouse_delta.y;

        play->pitch = Clamp(play->pitch, -Pi32, Pi32);

        if (play->yaw >= Tau32) {
            play->yaw -= Tau32;
        }
    }

    play->zoom -= input->mouse_delta.z;

    Draw_Transform norm_tx = batch->game_tx;

    m4x4 out = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, play->zoom,
        0, 0, 0, 1
    };

    m4x4 r = YRotation(play->yaw) * XRotation(play->pitch) * out;

    x_axis = GetColumn(r, 0);
    y_axis = GetColumn(r, 1);
    z_axis = GetColumn(r, 2);
    v3 p   = GetColumn(r, 3);

    SetCameraTransform(batch, 0, x_axis, y_axis, z_axis, p + play->target);

    f32 step = 2;
    for (f32 z = 40.f; z > 0; z -= step) {
        rect3 normal_bounds = GetCameraFrustum(&norm_tx, z);

        v4 c = V4(1, 0, 0, 1);
        if (z == 26) {
            c = V4(0, 0, 1, 1);
        }

        DrawQuadOutline(batch, 0.5f * (normal_bounds.max + normal_bounds.min),
                normal_bounds.max.xy - normal_bounds.min.xy, 0, c);
    }

    rect3 normal_bounds = GetCameraFrustum(&norm_tx);
    DrawQuadOutline(batch, 0.5f * (normal_bounds.max + normal_bounds.min),
                normal_bounds.max.xy - normal_bounds.min.xy, 0, V4(0, 1, 0, 1));
}

function void UpdatePlayer(Mode_Play *play, Input *input) {
    Player *player = &play->player;

    f32 dt   = input->delta_time;
    f32 time = input->time;

    f32 gravity = (2 * PLAYER_MAX_JUMP_HEIGHT) / (PLAYER_JUMP_APEX_TIME * PLAYER_JUMP_APEX_TIME);
    v2  ddp  = V2(0, gravity);

    switch (player->slot) {
        case AnimationSlot_Jump: {
            if (!(player->flags & PlayerFlag_HasJumped) && IsFinished(&player->animations[player->slot])) {
                player->dp.y = -Sqrt(2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
                player->on_ground = false;

                printf("APPLYING JUMP VEL\n");

                player->flags |= PlayerFlag_HoldAnimation;
                player->flags |= PlayerFlag_HasJumped;
            }

            if ((player->flags & PlayerFlag_HasJumped) && player->dp.y > 0) {
                player->slot = AnimationSlot_Fall;

                player->flags &= ~PlayerFlag_HoldAnimation;
                player->flags &= ~PlayerFlag_HasJumped;
            }
        }
        break;
        case AnimationSlot_PreDash: {
            if (IsFinished(&player->animations[player->slot])) {
                printf("------- BEGIN DASH ------- \n");
                Reset(&player->animations[player->slot]);
                player->slot = AnimationSlot_Dash;
                player->animation_scale = V2(4, 4);

                player->dash_timer = 0;

                play->shake = 0.9;
                return;
            }
            else {
                UpdateAnimation(&player->animations[player->slot], dt);
                return;
            }
        }
        break;
        case AnimationSlot_Dash: {
            if (IsFinished(&player->animations[player->slot])) {

                Reset(&player->animations[player->slot]);
                player->slot = AnimationSlot_PostDash;
                player->animation_scale = V2(1, 1);
            }
            else {
                if (player->on_ground && player->dash_dir.y > 0) {
                    player->slot = AnimationSlot_PostDash;
                    player->animation_scale = V2(1, 1);
                }
                else {
                    if (player->dash_dir.y < 0) { player->on_ground = false; }
                    //ddp = player->dash_dir * PLAYER_DASH_SPEED;

                     // PLAYER_DASH_SPEED;
                    // UpdateAnimation(&player->animations[player->slot], dt);
                }
            }
        }
        break;
        case AnimationSlot_PostDash: {
            if (IsFinished(&player->animations[player->slot])) {
                Reset(&player->animations[player->slot]);

                if (player->on_ground) {
                    player->slot = AnimationSlot_Idle;
                }
                else /*if (player->dp.y > 0)*/ { // @Todo: Not sure if this should stay like this
                    player->slot = AnimationSlot_Fall;
                }

                ddp.x = player->on_ground ? -PLAYER_MOVE_SPEED : -PLAYER_AIR_STRAFE_SPEED;

                printf("----------- END DASH -------\n\n");
            }
        }
        break;
    }

    player->dash_timer += dt;

#if 0
    if (player->can_dash && JustPressed(input->mouse_buttons[2])) {
        player->can_dash   = false;
        player->dash_timer = 0;

        player->dash_dir = V2(0, 1); // @Todo: Get proper dir

        result = true;
        return result;
    }
#endif

    if (JustPressed(input->keys[Key_Space])) { player->last_jump_time = time; }

    if (IsPressed(input->keys[Key_A])) {

        ddp.x += -PLAYER_MOVE_SPEED; // : -PLAYER_AIR_STRAFE_SPEED;
        player->facing = -1;
    }

    if (IsPressed(input->keys[Key_D])) {
        ddp.x += PLAYER_MOVE_SPEED; // : PLAYER_AIR_STRAFE_SPEED;
        player->facing = 1;
    }

    if (player->on_ground) {
        if (IsZero(ddp.x) && (player->dash_timer > 0.32)) {
            f32 damping = PLAYER_DAMPING;
            player->dp.x *= (1.0f / (1 + (damping * dt)));

            if (player->on_ground && player->slot != AnimationSlot_Jump && !IsDash(player->slot)) {
                player->slot = AnimationSlot_Idle;
                player->animation_scale = V2(1, 1);
            }
        }
        else if (!IsDash(player->slot) && (player->slot != AnimationSlot_Jump)) {
            player->slot = AnimationSlot_Walk;
            player->animation_scale = V2(1.27, 1.27);
        }
    }
#if 0
    else {
        f32 damping = PLAYER_DASH_DAMPING;
        player->dp.x *= (1.0f / (1 + (damping * dt)));
    }
#endif

    if (player->slot != AnimationSlot_Jump) {
        if ((time - player->last_jump_time) <= PLAYER_JUMP_BUFFER_TIME) {
            if (player->on_ground) { // @Todo: coyote
                player->slot = AnimationSlot_Jump;
                player->animation_scale = V2(1, 1);

                Reset(&player->animations[AnimationSlot_Jump]);
            }
        }
    }

    if (!(player->flags & PlayerFlag_HoldAnimation)) {
        UpdateAnimation(&player->animations[player->slot], dt);
    }

#define MAX_DASH_SPEED 45
    if (player->dash_timer < 0.13) {
        player->dp = player->dash_dir * MAX_DASH_SPEED;
    }

    player->p  += (player->dp * dt);
    player->dp += (ddp * dt);

    if (player->can_dash && JustPressed(input->keys[Key_J])) {
        player->flags &= ~PlayerFlag_HasJumped;
        player->flags &= ~PlayerFlag_HoldAnimation;

        player->dash_dir   = GetUnormalisedDashDirection(input);
        if (player->dash_dir.x == 0 && player->dash_dir.y == 0) {
            player->dash_dir   = V2(player->facing, 0);
            player->dash_angle = (player->facing < 0) ? Pi32 : 0;
        }
        else {
            player->dash_dir   = Noz(player->dash_dir);
            player->dash_angle = GetDashAngle(input);
        }

        player->animation_scale = V2(1.27, 1.27);
        player->slot = AnimationSlot_PreDash;
        return;
    }

    // This is to do with the jump but after the jump has happend so we want normal stuff to apply
    //
    if ((!IsPressed(input->keys[Key_Space]) && (player->dp.y < 0)) || (player->dash_timer < 0.13f)) {
        f32 initial_dp_sq = (2 * gravity * PLAYER_MAX_JUMP_HEIGHT);
        f32 limit_dp_sq   = (2 * gravity * (PLAYER_MAX_JUMP_HEIGHT - PLAYER_MIN_JUMP_HEIGHT));

        f32 term_dp = -Sqrt(initial_dp_sq - limit_dp_sq);
        if (player->dp.y < term_dp) {
            player->dp.y = term_dp;
        }
    }

    // Limit player speed
    //
    if (Abs(player->dp.x) > PLAYER_MAX_SPEED_X) { player->dp.x *= (PLAYER_MAX_SPEED_X / Abs(player->dp.x)); }
    if (Abs(player->dp.x) > PLAYER_MAX_SPEED_Y) { player->dp.y *= (PLAYER_MAX_SPEED_Y / Abs(player->dp.y)); }

    if (player->p.y > 4.98) {
        player->p.y  = 4.98;
        player->dp.y = 0;

        player->on_ground = true;
        player->can_dash  = true;
    }

    play->movement_trail[play->next_move] = player->p;
    play->next_move += 1;

    if (play->num_moves < ArraySize(play->movement_trail)) {
        play->num_moves += 1;
    }

    if (play->next_move >= ArraySize(play->movement_trail)) { play->next_move = 0; }
}

function void SpawnDust(Mode_Play *play, Level *level, Random *rng, u32 count) {
    u32 layer_index = RandomU32(rng, 0, level->back_layer);

    Layer *layer = &level->layers[layer_index];

    rect3 bounds = GetCameraFrustum(&play->batch->game_tx, 24 - layer->z_value);

    for (u32 i = 0; i < count; ++i) {
        Dust_Particle *p = &layer->dust[layer->num_dust];
        if (p->alpha > 0) { continue; }

        layer->num_dust += 1;
        if (layer->num_dust >= MAX_LAYER_DUST) {
            layer->num_dust = 0;
        }

        p->image = GetImageByName(&play->state->assets, "dust_blurred");

        p->scale = Max(0.22f + (layer->z_value * 0.13), 0.05);

        p->p.x = RandomF32(rng, bounds.min.x, bounds.max.x);
        p->p.y = RandomF32(rng, bounds.min.y, bounds.max.y);

        p->dp.x = 0.05f * RandomBilateral(rng);
        p->dp.y = 0.05f * RandomBilateral(rng);

        p->alpha_rate = -0.1f - RandomUnilateral(rng);
        p->alpha      = 0.5f * RandomUnilateral(rng);
    }

}

function void LevelUpdate(Mode_Play *play, Level *level, f32 dt) {
    Random *rng = &play->rnd;

    u32 v = RandomU32(rng, 0, 50);
    if (v > 20) { SpawnDust(play, &play->level, rng, 1); }

    for (s32 i = level->back_layer; i > 0; --i) {
        Layer *layer = &level->layers[i];

        for (u32 j = 0; j < MAX_LAYER_DUST; ++j) {
            Dust_Particle *p = &layer->dust[j];

            p->p += (dt * p->dp);

            p->alpha -= p->alpha_rate * dt;
            if (p->alpha >= 1) {
                p->alpha_rate = -p->alpha_rate;
                p->alpha = 1;
            }
        }
    }

    Player *player = &play->player;
    Layer  *layer = &level->layers[PLAYER_LAYER];

    rect2 player_box = GetBoundingBox(player->p, player->dim);

    for (u32 i = 0; i < layer->num_objects; ++i) {
        Env_Object *obj = &layer->objects[i];

        if (obj->flags & EnvObjectFlag_Platform) {
            Amt_Image *info = GetImageInfo(&play->state->assets, obj->image);

            v2 dim = obj->scale * GetScaledImageDim(info);
            rect2 platform = GetBoundingBox(obj->p, dim);

            //DrawQuadOutline(play->batch, V3(obj->p, layer->z_value), dim, 0, V4(0, 1, 1, 1), 0.03);

            v2 overlap;
            overlap.x = Min(player_box.max.x, platform.max.x) - Max(player_box.min.x, platform.min.x);
            overlap.y = Min(player_box.max.y, platform.max.y) - Max(player_box.min.y, platform.min.y);

            if (overlap.x > 0 && overlap.y > 0) {
                v2 dir = player->p - obj->p;

                if (overlap.x < overlap.y) {
                    f32 sign = Sign(dir.x);

                    player->dp.x = 0;
                    player->p.x  += (overlap.x * sign);
                }
                else {
                    f32 sign = Sign(dir.y);
                    player->p.y += (overlap.y * sign);

                    if (sign < 0) {
                        player->on_ground = true;
                        player->can_dash  = true;

                        if (player->dp.y > 0) {
                            player->dp.y = 0;
                        }
                    }
                    else {
                        player->dp.y = -0.33f * player->dp.y;
                    }
                }

                player_box.min = player->p - (0.5f * player->dim);
                player_box.max = player->p + (0.5f * player->dim);
            }
        }
    }
}

function void LudumModePlayUpdateRender(Mode_Play *play, Input *input, Renderer_Buffer *rbuffer) {
    Game_State *state = play->state;

    Draw_Batch _batch = { 0 };
    Draw_Batch *batch = &_batch;

    Initialise(batch, &state->assets, rbuffer);

    f32 dt = input->delta_time;

    play->batch = batch;

    v4 floor_c = V4(SQUARE(7/255.0f), SQUARE(24/255.0f), SQUARE(22/255.0f), 1.0);

    //DrawClear(batch, V4(0.005, 0.005, 0.005, 1.0));
    DrawClear(batch, floor_c);

    if (!play->editor.enabled) {
        UpdatePlayer(play, input);
    }

    //play->player.p.y - 2.9

    play->shake -= input->delta_time;
    play->shake  = Max(play->shake, 0);

    v2  shake_offset = play->shake_offset * SQUARE(play->shake) * RandomBilateral(&play->rnd);
    f32 shake_angle  = play->shake_angle  * SQUARE(play->shake) * RandomBilateral(&play->rnd);

    m4x4 zrot = ZRotation(shake_angle);

    v3 x_axis = GetColumn(zrot, 0);
    v3 y_axis = GetColumn(zrot, 1);
    v3 z_axis = GetColumn(zrot, 2);

    v2 target_p =  play->player.p; // - V2(0, 2.9);

    v2 ddp = CAMERA_STIFFNESS * (target_p - play->camera_p) - CAMERA_DAMPING * play->camera_dp;
    play->camera_p  += (0.5f * ddp * dt * dt) + (play->camera_dp * dt);
    play->camera_dp += (ddp * dt);

    v2 camera_p = play->camera_p;
    camera_p += shake_offset;

    SetCameraTransform(batch, 0, x_axis, y_axis, z_axis, V3(camera_p, 24));

    LevelUpdate(play, &play->level, dt);

    Draw_Transform game_tx = batch->game_tx;

    if (JustPressed(input->keys[Key_F9])) {
        play->active = !play->active;
    }

    if (JustPressed(input->keys[Key_F5])) { play->editor.enabled = !play->editor.enabled; }

    if (play->active) { UpdateDebugCamera(play, input, batch); }
    if (play->editor.enabled) {
        SimEditor(play, input);
    }

#if 0
    v3 world_mouse = Unproject(&batch->game_tx, input->mouse_clip);
    DrawQuad(batch, { 0 }, world_mouse, V2(0.1, 0.1), 0, V4(1, 1, 0, 1));
#endif

#if 0
    v3 rdir = Noz(world_mouse - batch->game_tx.p);
    v3 orig = world_mouse;
    b32 hit = false;
#endif

    for (f32 i = -0.5; i > -100; i -= 5) {
        f32 x = Max(0.09 * Abs(i), 1);
        DrawQuad(batch, { 0 }, V3(0.0, 0.0, i), V2(0.8, 0.8), 0, V4(1/x, 0, 1/x, 1));
    }

    //DrawQuad(batch, { 0 }, V2(0, 7.5), V2(25, 5));

    // 7      24     22
    // 0.195  0.342  0.328
    //
    // 0.0004 0.0055 0.0046

    vert3 q[4];
    q[0].p  = V3(-12.5f, 5.0, -2.0);
    q[0].uv = V2(0, 0);
    q[0].c  = ABGRPack(floor_c);

    q[1].p  = V3(-12.5f, 10.0, -2.0);
    q[1].uv = V2(0, 1);
    q[1].c  = 0xFF000000;

    q[2].p  = V3( 12.5f, 10.0, -2.0);
    q[2].uv = V2(1, 1);
    q[2].c  = 0xFF000000;

    q[3].p  = V3( 12.5f, 5.0, -2.0);
    q[3].uv = V2(1, 0);
    q[3].c  = ABGRPack(floor_c);

    DrawQuad(batch, { 0 }, q[0], q[1], q[2], q[3]);

    //DrawQuad(batch, { 0 }, V2(0, 7.5), V2(25, 5), 0, );

    Random rnd = RandomSeed(32903290);

#if 0
    f32 x = -12.5f;
    for (; x <= 12.5f; ) {
        u32 idx = RandomU32(&rnd, 0, ArraySize(stones));

        Amt_Image *info = GetImageInfo(&state->assets, stones[idx]);
        v2 scaled = 3 * GetScaledImageDim(info);

        DrawQuad(batch, stones[idx], V3(x, 4.95 + (0.5f * scaled.y), -2), 3.0);
        x += (scaled.x - 0.05);
    }
#endif

    Player *player = &play->player;

#if 0
    Image_Handle bg[] = {
        GetImageByName(&state->assets, "large_rock"),
        GetImageByName(&state->assets, "small_rock"),
    };

    f32 pz = -15;
    for (u32 i = 0; i < 10; ++i) {
        f32 px = RandomF32(&rnd, -8, 8);

        u32 slot = RandomU32(&rnd, 0, ArraySize(bg));

        f32 scale = 4.0f;
        if (slot == 1) { scale = 1.4f; }

        DrawQuad(batch, bg[slot], V3(px, 3.5, pz), scale, 0, V4(0, 0, 0, 1));
        pz += 1;
    }
#endif

    f32 facing = player->facing;
    f32 angle  = 0;
    v2  offset = V2(0, 0);

    if (player->slot == AnimationSlot_Dash) {
        angle  = player->dash_angle;
        facing = 1;

        if (player->animations[player->slot].current_frame == 0) {
            offset += player->dash_dir * DASH_OFFSET;
        }
        else {
            offset -= player->dash_dir * DASH_OFFSET;
        }
    }

    b32 drawn_player = false;

    Level *level = &play->level;
    for (s32 i = level->back_layer; i > 0; --i) {
        Layer *layer = &level->layers[i];

        for (u32 j = 0; j < layer->num_objects; ++j) {
            Env_Object *obj = &layer->objects[j];

            v4 c = V4(1, 1, 1, 1);
            if (obj->flags & EnvObjectFlag_Foreground) {
                c = V4(0, 0, 0, 1);
            }

            DrawQuad(batch, obj->image, V3(obj->p, layer->z_value), obj->scale, 0, c);
        }

        for (u32 j = 0; j < MAX_LAYER_DUST; ++j) {
            Dust_Particle *dust = &layer->dust[j];
            if (dust->alpha > 0) {
                DrawQuad(batch, dust->image, V3(dust->p, layer->z_value), dust->scale, 0, V4(1, 1, 1, dust->alpha));
            }

        }

        if (layer->z_value < -2) {
            // Fullscreen quad for fog effect, this is dumb as it messes with the background colour
            // should've done it in shader but that would require base changes and thats annoying
            //
            rect3 camera_bounds = GetCameraFrustum(&game_tx, 24 + Abs(layer->z_value));

            v3 center = 0.5f * (camera_bounds.max + camera_bounds.min);
            v2 dim    = camera_bounds.max.xy - camera_bounds.min.xy;


            floor_c.a = 0.4f;
            DrawQuad(batch, { 0 }, center, dim, 0, floor_c);
        }
    }

#if 0
    for (u32 i = 0; i < play->num_env_objects; ++i) {
        Env_Object *obj = &play->env[i];

                if (obj->layer > last_layer && obj->layer < -4) {
                    last_layer = obj->layer;
        }

        // @Hack: to draw the player in the right place for alpha blending, we really ought to have
        // sorting directly in the renderer
        //
        if (!drawn_player && obj->layer == -2) {
            DrawAnimation(batch, &player->animations[player->slot], V3(player->p + offset, -2.0),
                    V2(facing, 1) * player->animation_scale, angle);

            drawn_player = true;
        }

        Amt_Image *info = GetImageInfo(&state->assets, obj->image);
        v2 dim = obj->scale *  GetScaledImageDim(info);

        if (play->editor.enabled) {
            DrawQuadOutline(batch, V3(obj->p, obj->layer), dim, 0, V4(1, 0, 0, 1));
        }


        f32 alpha = 1;
        if (obj->layer < -4) {
            alpha = 1.0f / cast(f32) Abs(4 + obj->layer);
        }

         //, 0, V4(1, 1, 1, alpha));
    }
#endif

#if 0
    for (u32 i = 0; i < play->num_moves; ++i) {
        DrawQuad(batch, { 0 }, play->movement_trail[i], V2(0.1, 0.1), 0, V4(1, 0, 0, 1));
    }
#endif

    if (!drawn_player) {
        v4 c = player->on_ground ? V4(0, 1, 0, 1) : V4(1, 1, 1, 1);
        DrawAnimation(batch, &player->animations[player->slot], V3(player->p + offset, -2.0),
                V2(facing, 1) * player->animation_scale, angle, c);
    }


    //DrawQuadOutline(batch, player->p - V2(0, 0.70f), player->dim, 0, player->on_ground ? V4(0, 1, 0, 1) : V4(0, 1, 1, 1));

#if 0
    for (u32 i = 0; i < ArraySize(fg); ++i) {
        x = RandomF32(&rnd, -8, 8);
        DrawQuad(batch, fg[i], V3(x, 4.6, 6), 1.2);
    }

    for (u32 i = 0; i < ArraySize(fg); ++i) {
        x = RandomF32(&rnd, -8, 8);
        DrawQuad(batch, fg[i], V3(x, 3.7, 12), 1.2);
    }
#endif
}

// Editor
//

#if 0
function void SortEnvObjects(Mode_Play *play) {
    // @Speed: VERY simple insertion sort bc I can't be botherd but it slow
    //
    s32 i = 1;
    while (i < (s32) play->num_env_objects) {
        s32 j = i;
        while (j > 0 && play->env[j - 1].layer > play->env[j].layer) {
            Swap(play->env[j], play->env[j - 1]);
            j = j - 1;
        }

        i += 1;
    }
}
#endif

#define FRONT_LAYER  4
#define BACK_LAYER  -10

#define NUM_PLATFORM_TYPES 6

function void SimEditor(Mode_Play *play, Input *input) {
    Game_State *state = play->state;
    Draw_Batch *batch = play->batch;

    Editor *editor = &play->editor;
    Level  *level  = editor->level;

    u32 layer_index = (FRONT_LAYER - editor->layer) >> 1;
    Layer *layer = &level->layers[layer_index];

    layer->z_value = editor->layer;

    v3 mouse_world = Unproject(&batch->game_tx, input->mouse_clip);
    Image_Handle env_images[] = {
        GetImageByName(&state->assets, "platform_01"),
        GetImageByName(&state->assets, "platform_02"),
        GetImageByName(&state->assets, "platform_03"),
        GetImageByName(&state->assets, "platform_04"),
        GetImageByName(&state->assets, "platform_05"),
        GetImageByName(&state->assets, "platform_06"),

        GetImageByName(&state->assets, "foreground_01"),
        GetImageByName(&state->assets, "foreground_02"),
        GetImageByName(&state->assets, "foreground_03"),
        GetImageByName(&state->assets, "foreground_04"),
        GetImageByName(&state->assets, "foreground_05"),
        GetImageByName(&state->assets, "foreground_06"),
        GetImageByName(&state->assets, "foreground_07"),
        GetImageByName(&state->assets, "foreground_08"),
        GetImageByName(&state->assets, "foreground_09"),
        GetImageByName(&state->assets, "foreground_10"),
        GetImageByName(&state->assets, "foreground_11"),

        GetImageByName(&state->assets, "bush_blue"),
        GetImageByName(&state->assets, "bush_blue_blurred"),
        GetImageByName(&state->assets, "bush_green"),
        GetImageByName(&state->assets, "bush_green_blurred"),
        GetImageByName(&state->assets, "bush_red"),
        GetImageByName(&state->assets, "bush_red_blurred"),
        GetImageByName(&state->assets, "bush_small_blue"),
        GetImageByName(&state->assets, "bush_small_blue_blurred"),
        GetImageByName(&state->assets, "bush_small_green"),
        GetImageByName(&state->assets, "bush_small_green_blurred"),
        GetImageByName(&state->assets, "bush_small_red"),
        GetImageByName(&state->assets, "bush_small_red_blurred"),
        GetImageByName(&state->assets, "mask_bush"),
        GetImageByName(&state->assets, "mask"),
        GetImageByName(&state->assets, "mask_blurred"),
        GetImageByName(&state->assets, "gourd"),
        GetImageByName(&state->assets, "gourd_blurred"),
        GetImageByName(&state->assets, "gourd_bush"),
        GetImageByName(&state->assets, "bg_tree"),

        GetImageByName(&state->assets, "large_rock"),
        GetImageByName(&state->assets, "small_rock"),
        GetImageByName(&state->assets, "shiitake"),
        GetImageByName(&state->assets, "shiitake-blurred"),
    };

    if (JustPressed(input->keys[Key_LBracket])) {
        editor->type -= 1;
        if (editor->type < 0) { editor->type = ArraySize(env_images) - 1; }
    }

    if (JustPressed(input->keys[Key_RBracket])) {
        editor->type += 1;
        if (editor->type >= ArraySize(env_images)) { editor->type = 0; }
    }

    if (JustPressed(input->keys[Key_Up]))   {
        editor->layer -= 2;
        if (editor->layer < BACK_LAYER) {
            editor->layer = BACK_LAYER;
        }

        printf("Info: layer is %d\n", editor->layer);
    }

    if (JustPressed(input->keys[Key_Down])) {
        editor->layer += 2;
        if (editor->layer > FRONT_LAYER) {
            editor->layer = FRONT_LAYER;
        }

        printf("Info: layer is %d\n", editor->layer);
    }

    if (JustPressed(input->keys[Key_Left]))  { editor->scale -= 0.25; }
    if (JustPressed(input->keys[Key_Right])) { editor->scale += 0.25; }

    if (IsPressed(input->keys[Key_Ctrl]) && JustPressed(input->keys[Key_Z])) {
        if (layer->num_objects >= 1) {
            layer->num_objects -= 1;
        }
    }

    if (editor->mode == EditorMode_Place && JustPressed(input->mouse_buttons[0])) {
        if (layer->num_objects < MAX_LAYER_OBJECTS) {
            Env_Object *obj = &layer->objects[layer->num_objects];
            layer->num_objects += 1;

            obj->image = env_images[editor->type];
            obj->p     = mouse_world.xy;
            obj->scale = editor->scale;

            if (layer->z_value == FRONT_LAYER) {
                obj->flags |= EnvObjectFlag_Foreground;
            }

            if (editor->type < NUM_PLATFORM_TYPES) {
                obj->flags |= EnvObjectFlag_Platform;
            }

            if (layer_index > level->back_layer) { level->back_layer = layer_index; }
        }
    }

    Amt_Image *info = GetImageInfo(&state->assets, env_images[editor->type]);
    v2 dim = editor->scale * GetScaledImageDim(info);

    DrawQuad(batch, env_images[editor->type], V3(mouse_world.xy, editor->layer), dim, 0, V4(1, 1, 1, 0.6));
}
