#version 330 core

layout(location = 0) in vec3 vertexPosition_modelSpace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelSpace;

out vec2 UV;
out vec3 Normal_cameraSpace;
out vec3 LightDirection_cameraSpace;
out vec3 Position_worldSpace;
out vec3 EyeDirection_cameraSpace;

uniform mat4 M;
uniform mat4 V;
uniform mat4 MVP;
uniform vec3 LightPosition_worldSpace;

void main() {
    vec3 pos = vertexPosition_modelSpace;
    gl_Position = MVP * vec4(pos, 1);

    Position_worldSpace = (M * vec4(vertexPosition_modelSpace, 1)).xyz;

    vec3 vertexPosition_cameraSpace = (V * M * vec4(vertexPosition_modelSpace, 1)).xyz;
    EyeDirection_cameraSpace = vec3(0, 0, 0) - vertexPosition_cameraSpace;

    vec3 lightPosition_cameraSpace = (V * vec4(LightPosition_worldSpace, 1)).xyz;
    LightDirection_cameraSpace = lightPosition_cameraSpace + EyeDirection_cameraSpace;

    Normal_cameraSpace = (V * M * vec4(vertexNormal_modelSpace, 0)).xyz;

    UV = vertexUV;
}
