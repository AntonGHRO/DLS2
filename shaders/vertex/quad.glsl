#version 460 core

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 texture_coords;

layout (location = 2) uniform vec2 position;
layout (location = 3) uniform vec2 size;
layout (location = 4) uniform vec2 rect_offset;
layout (location = 5) uniform mat2 transform;
layout (location = 6) uniform vec2 offset;
layout (location = 7) uniform vec3 color;
layout (location = 8) uniform float alpha;
layout (location = 9) uniform float angle;
layout (location = 10) uniform float ratio;

out vec2 texture_coords_out;

void main()
{
    float cosa = cos(angle);
    float sina = sin(angle);

    if(position.x == 0.0 && position.y == 0.0 && size.x == 0.0 && size.y == 0.0) {
        if(gl_VertexID == 0) gl_Position = vec4(-1.0, -1.0f, 0.0, 1.0);
        else if(gl_VertexID == 1) gl_Position = vec4(+1.0, -1.0f, 0.0, 1.0);
        else if(gl_VertexID == 2) gl_Position = vec4(-1.0, +1.0f, 0.0, 1.0);
        else if(gl_VertexID == 3) gl_Position = vec4(+1.0, +1.0f, 0.0, 1.0);
    }
    else {
        if(ratio == 0.0) {
            gl_Position = vec4(
                transform * vec2(
                    (cosa*size.x*vertex_position.x - sina*size.y*vertex_position.y - size.x*rect_offset.x) + position.x - offset.x,
                    (sina*size.x*vertex_position.x + cosa*size.y*vertex_position.y - size.y*rect_offset.y) + position.y - offset.y
                ),
                0.0,
                1.0
            );
        }
        else {
            gl_Position = vec4(
                vec2(
                    (size.x*ratio*vertex_position.x - size.x*rect_offset.x) + ratio*position.x,
                    (size.y*vertex_position.y - size.y*rect_offset.y) + position.y
                ),
                0.0,
                1.0
            );
        }
    }

    texture_coords_out = texture_coords;
}
