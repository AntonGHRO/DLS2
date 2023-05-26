#version 460 core

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

layout (location = 1) uniform mat2 transform;
layout (location = 2) uniform vec3 color;
layout (location = 3) uniform vec2 offset;
layout (location = 4) uniform vec2 win_size;
layout (location = 5) uniform float px_per_unit;
layout (location = 6) uniform float unit_boundary;

out vec4 frag_color;

vec2 pos;
vec2 offset_t;
vec2 win_size_t;

void main()
{
    frag_color = vec4(color, 0.0);

    offset_t = 0.5 * transform * offset;
    win_size_t = 0.5 * win_size;

    pos.x = map(gl_FragCoord.x, 0, win_size.x, -win_size_t.x, win_size_t.x) + offset_t.x * win_size.x;
    pos.y = map(gl_FragCoord.y, 0, win_size.y, -win_size_t.y, win_size_t.y) + offset_t.y * win_size.y;

    if(fract(pos.x / px_per_unit) <= unit_boundary || fract(pos.y / px_per_unit) <= unit_boundary)
        frag_color.a = 0.2;
}
