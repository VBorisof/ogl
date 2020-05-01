#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;
uniform mat4 MVP;

void main() {
    vec3 pos = vertexPosition_modelspace;

    gl_Position = MVP * vec4(pos, 1);

    UV = vertexUV;
}
