function b32 IsFinished(Sprite_Animation *animation) {
    b32 result = (animation->current_frame == (animation->total_frames - 1));
    return result;
}

function void Reset(Sprite_Animation *animation) {
    animation->current_frame = 0;
    animation->time = 0;
}

function v2 GetScaledImageDim(Amt_Image *image) {
    v2 result;

    if (image->width > image->height) {
        result.w = 1.0f;
        result.h = cast(f32) image->height / image->width;
    }
    else {
        result.w = cast(f32) image->width / image->height;
        result.h = 1.0f;
    }

    return result;
}

function rect2 GetBoundingBox(v2 p, v2 dim) {
    rect2 result;
    result.min = p - (0.5f * dim);
    result.max = p + (0.5f * dim);

    return result;
}
