function void WriteKiz(Level *level) {
    Kiz_Header hdr = { 0 };
    hdr.magic      = KIZ_HEADER_MAGIC;
    hdr.version    = KIZ_VERSION;

    hdr.px = level->player_start_p.x;
    hdr.py = level->player_start_p.y;

    hdr.num_layers  = 0;
    hdr.num_objects = 0;

    hdr.camera_min_x = level->camera_r.min.x;
    hdr.camera_min_y = level->camera_r.min.y;

    hdr.camera_max_x = level->camera_r.max.x;
    hdr.camera_max_y = level->camera_r.max.y;

    // Count number of layers and objects
    //
    for (u32 i = 0; i < MAX_LEVEL_LAYERS; ++i) {
        Layer *layer = &level->layers[i];

        u32 layer_object_count = 0;
        for (u32 j = 0; j < MAX_LAYER_OBJECTS; ++j) {
            if (layer->objects[j].valid) {
                layer_object_count += 1;
                hdr.num_objects    += 1;
            }
        }

        if (layer_object_count > 0) { hdr.num_layers += 1; }
    }

    Scratch_Memory scratch = GetScratch();
    Memory_Arena *temp = scratch.arena;

    str8 working  = Platform->GetPath(PlatformPath_Executable);
    str8 filename = FormatStr(temp, "%.*s/data/level0.kiz", str8_unpack(working));

    Kiz_Layer  *layers  = AllocArray(temp, Kiz_Layer,  hdr.num_layers);
    Kiz_Object *objects = AllocArray(temp, Kiz_Object, hdr.num_objects);

    u32 current_layer = 0;
    u32 current_object_count = 0;

    // Collate all of the layer information
    //
    for (u32 i = 0; i < MAX_LEVEL_LAYERS; ++i) {
        Layer *level_layer = &level->layers[i];

        u32 begin_object_count = current_object_count;

        for (u32 j = 0; j < MAX_LAYER_OBJECTS; ++j) {
            Env_Object *object = &level_layer->objects[j];

            if (object->valid) {
                Kiz_Object *kiz = &objects[current_object_count];
                current_object_count += 1;

                kiz->type = object->type;

                kiz->sub_layer = j;

                kiz->side_flags   = object->sides;
                kiz->terrain_seed = object->terrain_seed;

                kiz->p     = object->p;
                kiz->scale = object->scale;
            }
        }

        if (current_object_count > begin_object_count) {
            Kiz_Layer *kiz_layer = &layers[current_layer++];

            kiz_layer->first_object = begin_object_count;
            kiz_layer->object_count = current_object_count - begin_object_count;
            kiz_layer->z_value      = level_layer->z_value;
        }
    }

    File_Handle file = Platform->OpenFile(filename, FileAccess_Write);
    if (file.errors) {
        printf("Error: failed to open file\n");
        return;
    }

    uptr offset = 0;

    Platform->WriteFile(&file, &hdr, offset, sizeof(Kiz_Header));
    offset += sizeof(Kiz_Header);

    Platform->WriteFile(&file, layers, offset, hdr.num_layers * sizeof(Kiz_Layer));
    offset += (hdr.num_layers * sizeof(Kiz_Layer));

    Platform->WriteFile(&file, objects, offset, hdr.num_objects * sizeof(Kiz_Object));

    Platform->CloseFile(&file);

    printf("There were %d layers, with a total of %d objects\n", hdr.num_layers, hdr.num_objects);
}

function void ReadKiz(str8 filename, Level *level, Mode_Play *play) {
    Scratch_Memory scratch = GetScratch();
    Memory_Arena *temp = scratch.arena;

    str8 working = Platform->GetPath(PlatformPath_Executable);
    str8 fname = FormatStr(temp, "%.*s/data/%.*s", str8_unpack(working), str8_unpack(filename));

    File_Handle file = Platform->OpenFile(fname, FileAccess_Read);
    if (file.errors) {
        printf("Error: failed to read %.*s\n", str8_unpack(fname));
        return;
    }

    Kiz_Header hdr = { 0 };

    uptr offset = 0;

    Platform->ReadFile(&file, &hdr, offset, sizeof(Kiz_Header));

    if (file.errors || (hdr.magic != KIZ_HEADER_MAGIC)) {
        printf("Error: supplied file was not a .kiz file\n");
        return;
    }

    level->front_layer = 16;
    level->back_layer  = 0;

    Kiz_Layer  *layers  = AllocArray(temp, Kiz_Layer,  hdr.num_layers);
    Kiz_Object *objects = AllocArray(temp, Kiz_Object, hdr.num_objects);

    offset += sizeof(Kiz_Header);
    Platform->ReadFile(&file, layers, offset, hdr.num_layers * sizeof(Kiz_Layer));

    offset += (hdr.num_layers * sizeof(Kiz_Layer));
    Platform->ReadFile(&file, objects, offset, hdr.num_objects * sizeof(Kiz_Object));

    level->camera_r.min = V2(hdr.camera_min_x, hdr.camera_min_y);
    level->camera_r.max = V2(hdr.camera_max_x, hdr.camera_max_y);

    for (u32 i = 0; i < hdr.num_layers; ++i) {
        Kiz_Layer *kiz_layer = &layers[i];

        u32 level_layer_index = (FRONT_LAYER - kiz_layer->z_value) >> 1;
        Layer *level_layer = &level->layers[level_layer_index];

        if (level_layer_index < level->front_layer) { level->front_layer = level_layer_index; }
        if (level_layer_index > level->back_layer)  { level->back_layer  = level_layer_index; }

        level_layer->z_value     = kiz_layer->z_value;
        level_layer->num_objects = kiz_layer->object_count;

        printf("adding layer at %d: %d objects\n", level_layer->z_value, kiz_layer->object_count);

        u32 start = kiz_layer->first_object;
        u32 end   = start + kiz_layer->object_count;

        for (u32 j = start; j < end; ++j) {
            Kiz_Object *kiz_obj = &objects[j];
            Env_Object *env_obj = &level_layer->objects[kiz_obj->sub_layer];


            env_obj->valid = true;

            env_obj->type         = kiz_obj->type;
            env_obj->sides        = kiz_obj->side_flags;
            env_obj->terrain_seed = kiz_obj->terrain_seed;

            if (env_obj->type == KIZ_TERRAIN_TYPE) {
                env_obj->flags |= EnvObjectFlag_Terrain;
                env_obj->flags |= EnvObjectFlag_Platform;
            }

            if (level_layer->z_value == FRONT_LAYER) { env_obj->flags |= EnvObjectFlag_Foreground; }

            if (env_obj->type >= KizObject_platform_01 && env_obj->type <= KizObject_platform_06) {
                env_obj->flags |= EnvObjectFlag_Platform;
            }

            if (env_obj->type == KizObject_orb || env_obj->type == KizObject_goal) {
                env_obj->flags |= EnvObjectFlag_Animated;

                Image_Handle sheet = GetObjectImage(play, env_obj->type);
                switch (env_obj->type) {
                    case KizObject_orb: {
                        Initialise(&env_obj->animation, sheet, 1, 22, 1.0f / 30.0f);
                    }
                    break;
                    case KizObject_goal: {
                        Initialise(&env_obj->animation, sheet, 3, 4, 11, 1.0f / 30.0f);
                    }
                    break;
                }
            }

            env_obj->p     = kiz_obj->p;
            env_obj->scale = kiz_obj->scale;
        }
    }

    Platform->CloseFile(&file);
}
