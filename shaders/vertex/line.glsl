#version 460 core

layout(location = 0) in int index;

layout(location = 1) uniform mat2 transform;
layout(location = 2) uniform vec2 offset;
layout(location = 3) uniform vec2 position0;
layout(location = 4) uniform vec2 position1;
layout(location = 5) uniform vec3 color0;
layout(location = 6) uniform vec3 color1;

out vec3 color;

void main()
{
    if(index == 0)
    {
        gl_Position = vec4(transform * (position0 - offset), 0.0, 1.0);
        color = color0;
    }
    else if(index == 1)
    {
        gl_Position = vec4(transform * (position1 - offset), 0.0, 1.0);
        color = color1;
    }
}
