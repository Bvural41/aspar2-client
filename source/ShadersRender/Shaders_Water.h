#pragma once

namespace ShadersRender
{
static const char* s_waterVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform vec3 u_cameraPos;
uniform float u_waterScale;

out vec4 v_color;
out vec2 v_texCoord;
out vec3 v_worldPos;
out float v_dist;

void main()
{
    vec4 worldPos = u_world * vec4(a_position, 1.0);
    vec4 p = u_worldViewProj * vec4(a_position, 1.0);
    p.z -= 0.0008 * p.w;
    gl_Position = p;
    v_worldPos = worldPos.xyz;
    v_color = a_color;
    float scale = (u_waterScale != 0.0) ? u_waterScale : 0.00125;
    v_texCoord = vec2(worldPos.x * scale, -worldPos.y * scale);
    v_dist = length(u_cameraPos - worldPos.xyz);
}
)";

static const char* s_waterFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D u_texture;
uniform vec4 u_fogColor;
uniform vec2 u_fogRange;
uniform int u_fogEnable;
uniform float u_time;
uniform vec3 u_cameraPos;

in vec4 v_color;
in vec2 v_texCoord;
in vec3 v_worldPos;
in float v_dist;

out vec4 fragColor;

void main()
{
    vec2 ripple = vec2(
        sin(v_texCoord.y * 20.0 + u_time * 1.5) * 0.002,
        cos(v_texCoord.x * 20.0 + u_time * 1.8) * 0.002
    );
    vec4 texColor = texture(u_texture, v_texCoord + ripple);
    vec3 baseWater = vec3(0.12, 0.38, 0.58);
    vec3 waterColor = (length(texColor.rgb) > 0.05) ? mix(baseWater, texColor.rgb, 0.65) : baseWater;
    float waterAlpha = max(v_color.a, 0.75);
    vec3 viewDir = normalize(u_cameraPos - v_worldPos);
    vec3 sunDir = normalize(vec3(0.4, 0.7, 0.5));
    vec3 halfVec = normalize(sunDir + viewDir);
    float spec = pow(max(dot(vec3(0.0, 0.0, 1.0), halfVec), 0.0), 32.0);
    waterColor += vec3(spec * 0.35);
    if (u_fogEnable != 0 && u_fogRange.y > u_fogRange.x)
    {
        float fogFactor = clamp((v_dist - u_fogRange.x) / (u_fogRange.y - u_fogRange.x), 0.0, 1.0);
        waterColor = mix(waterColor, u_fogColor.rgb, fogFactor);
    }
    fragColor = vec4(waterColor, waterAlpha);
}
)";
}
