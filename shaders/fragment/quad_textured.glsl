#version 460 core

in vec2 texture_coords_out;
out vec4 frag_color;

uniform sampler2D texture_data;

void main()
{
    frag_color = texture(texture_data, texture_coords_out);
}
