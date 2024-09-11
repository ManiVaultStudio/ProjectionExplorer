#version 330 core

in vec2 pass_texcoord;
in vec3 pass_values;

out vec4 fragColor;

void main()
{
    if (length(pass_texcoord) > 1.0) discard;
    
    fragColor = vec4(pass_values.r, pass_values.g, pass_values.b, 1);
}
