#pragma once

constexpr const char* vertexShaderPrg = R"foo(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform vec2 screenFormat;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = vec4(vertex.x * screenFormat.x, vertex.y * screenFormat.y, 0.0f, 1.0f);
}
)foo";

constexpr const char* fragmentShaderPrg = R"foo(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D Texture;
uniform vec3 palette[64];

void main()
{   
    color = vec4(palette[int(texture(Texture, TexCoords) * 255.0f)], 1.0);
}  
)foo";