#version 330 core

in vec2 UV;
in vec3 Normal_cameraSpace;
in vec3 LightDirection_cameraSpace;
in vec3 Position_worldSpace;
in vec3 EyeDirection_cameraSpace;

out vec3 color;

uniform sampler2D myTextureSampler;
uniform vec3 LightPosition_worldSpace;
uniform vec3 LightColor;

void main() {
    vec3 materialDiffuseColor = texture(myTextureSampler, UV).rgb;
    vec3 materialAmbientColor = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(1, 1, 1);

    float distanceToLight = length(LightPosition_worldSpace - Position_worldSpace);

    vec3 n = normalize(Normal_cameraSpace);
    vec3 l = normalize(LightDirection_cameraSpace);

    float cosTheta = clamp(dot(n, l), 0, 1);

    vec3 diffuseColor = materialDiffuseColor * (LightColor * cosTheta / distanceToLight*distanceToLight);


    vec3 eyeVector = normalize(EyeDirection_cameraSpace);
    vec3 reflectionDirection = reflect(-l, n);

    float cosAlpha = clamp(dot(eyeVector, reflectionDirection), 0, 1);

    vec3 specularColor = materialSpecularColor * LightColor * pow(cosAlpha, 2) / (distanceToLight * distanceToLight);

    color =
        materialAmbientColor + diffuseColor + specularColor;
    ;
}