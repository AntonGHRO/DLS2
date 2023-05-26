#version 460 core

layout (location = 7) uniform vec3 color;
layout (location = 8) uniform float alpha;

in vec2 texture_coords_out;
out vec4 frag_color;

void main()
{
    frag_color = vec4(color, alpha);
}
