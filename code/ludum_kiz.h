#if !defined(LUDUM_KIZ_H_)
#define LUDUM_KIZ_H_

#define KIZ_HEADER_MAGIC (((u32) 'L' << 24) | ((u32) 'Z' << 16) | ((u32) 'I' << 8) | ((u32) 'K' << 0))
#define KIZ_VERSION 1

struct Level;
struct Mode_Play;

#define FRONT_LAYER  4
#define BACK_LAYER  -10

#define KIZ_TERRAIN_TYPE (u32) -1

#define KIZ_OBJECT_LIST \
    KIZ_OBJECT_NAME(bg_tree), \
    KIZ_OBJECT_NAME(bush_01), \
    KIZ_OBJECT_NAME(bush_01_blurred), \
    KIZ_OBJECT_NAME(bush_02), \
    KIZ_OBJECT_NAME(bush_02_blurred), \
    KIZ_OBJECT_NAME(bush_03), \
    KIZ_OBJECT_NAME(bush_03_blurred), \
    KIZ_OBJECT_NAME(bush_04), \
    KIZ_OBJECT_NAME(bush_04_blurred), \
    KIZ_OBJECT_NAME(bush_05), \
    KIZ_OBJECT_NAME(bush_05_blurred), \
    KIZ_OBJECT_NAME(bush_06), \
    KIZ_OBJECT_NAME(bush_06_blurred), \
    KIZ_OBJECT_NAME(bush_07), \
    KIZ_OBJECT_NAME(bush_07_blurred), \
    KIZ_OBJECT_NAME(bush_blue), \
    KIZ_OBJECT_NAME(bush_blue_blurred), \
    KIZ_OBJECT_NAME(bush_small_blue), \
    KIZ_OBJECT_NAME(bush_small_blue_blurred), \
    KIZ_OBJECT_NAME(bush_green), \
    KIZ_OBJECT_NAME(bush_green_blurred), \
    KIZ_OBJECT_NAME(bush_small_green), \
    KIZ_OBJECT_NAME(bush_small_green_blurred), \
    KIZ_OBJECT_NAME(bush_red), \
    KIZ_OBJECT_NAME(bush_red_blurred), \
    KIZ_OBJECT_NAME(bush_small_red), \
    KIZ_OBJECT_NAME(bush_small_red_blurred), \
    KIZ_OBJECT_NAME(gourd), \
    KIZ_OBJECT_NAME(gourd_blurred), \
    KIZ_OBJECT_NAME(gourd_bush), \
    KIZ_OBJECT_NAME(mask), \
    KIZ_OBJECT_NAME(mask_blurred), \
    KIZ_OBJECT_NAME(mask_bush), \
    KIZ_OBJECT_NAME(large_rock), \
    KIZ_OBJECT_NAME(small_rock), \
    KIZ_OBJECT_NAME(shiitake), \
    KIZ_OBJECT_NAME(shiitake_blurred), \
    KIZ_OBJECT_NAME(foreground_01), \
    KIZ_OBJECT_NAME(foreground_02), \
    KIZ_OBJECT_NAME(foreground_03), \
    KIZ_OBJECT_NAME(foreground_04), \
    KIZ_OBJECT_NAME(foreground_05), \
    KIZ_OBJECT_NAME(foreground_06), \
    KIZ_OBJECT_NAME(foreground_07), \
    KIZ_OBJECT_NAME(foreground_08), \
    KIZ_OBJECT_NAME(foreground_09), \
    KIZ_OBJECT_NAME(foreground_10), \
    KIZ_OBJECT_NAME(foreground_11), \
    KIZ_OBJECT_NAME(platform_01), \
    KIZ_OBJECT_NAME(platform_02), \
    KIZ_OBJECT_NAME(platform_03), \
    KIZ_OBJECT_NAME(platform_04), \
    KIZ_OBJECT_NAME(platform_05), \
    KIZ_OBJECT_NAME(platform_06), \
    KIZ_OBJECT_NAME(orb), \
    KIZ_OBJECT_NAME(goal)

#define KIZ_OBJECT_NAME(name) KizObject_##name

enum Kiz_Object_Type {
    KIZ_OBJECT_LIST,

    KizObject_count
};

#undef KIZ_OBJECT_NAME

#define KIZ_OBJECT_NAME(name) #name

global const char *kiz_object_names[] = {
    KIZ_OBJECT_LIST
};

#pragma pack(push, 1)

struct Kiz_Header {
    u32 magic;
    u32 version;

    u32 pad0;
    u32 pad1;

    f32 px;
    f32 py;

    u32 num_layers;
    u32 num_objects;

    f32 camera_min_x;
    f32 camera_min_y;

    f32 camera_max_x;
    f32 camera_max_y;

    u64 pad[2];
};

struct Kiz_Layer {
    u32 first_object;
    u32 object_count;

    s32 z_value;
    u32 pad;
};

struct Kiz_Object {
    u32 type; // for getting the animation or image back

    u32 sub_layer; // sublayer index

    // Terrain only
    //
    u32 side_flags;
    u32 pad;

    u64 terrain_seed;

    v2 p;
    v2 scale;
};

#pragma pack(pop)

function void WriteKiz(Level *level);
function void ReadKiz(str8 filename, Level *level, Mode_Play *play);

#endif  //  LUDUM_KIZ_H_
