#version 330 core

uniform mat3 projMatrix;

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 position;
layout(location = 2) in vec3 values;

out vec2 pass_texcoord;
out vec3 pass_values;

void main()
{
    pass_texcoord = vertex;

    vec2 pos = (projMatrix * vec3(position, 1)).xy;
    
    pass_values = values;
    
    gl_Position = vec4(vertex * 0.02 + pos, 0, 1);
}
