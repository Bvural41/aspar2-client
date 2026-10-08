#pragma once

namespace ShadersRender
{
static const char* s_terrainVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_worldViewProj;
uniform mat4 u_view;
uniform mat4 u_world;
uniform mat4 u_texMat0;
uniform mat4 u_texMat1;

out vec3 v_worldPos;
out vec3 v_worldNormal;
out vec2 v_texCoord0;
out vec2 v_texCoord1;
out float v_fogDist;

void main()
{
    vec4 pos = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = pos;
    v_worldPos = a_position;
    v_worldNormal = mat3(u_world) * a_normal;
    vec4 pCam = u_view * vec4(a_position, 1.0);
    vec4 tc0 = u_texMat0 * pCam;
    vec4 tc1 = u_texMat1 * pCam;
    v_texCoord0 = tc0.xy;
    v_texCoord1 = tc1.xy;
    v_fogDist = pos.w;
}
)";

static const char* s_terrainFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec3 v_worldPos;
in vec3 v_worldNormal;
in vec2 v_texCoord0;
in vec2 v_texCoord1;
in float v_fogDist;

uniform sampler2D u_tileTexture;
uniform sampler2D u_splatTexture;
uniform int u_useTileTexture;
uniform int u_useSplatAlpha;
uniform int u_isShadowPass;
uniform int u_isDungeonMap;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform vec4 u_fogColor;
uniform vec2 u_fogRange;
uniform int u_fogEnable;
uniform vec3 u_cameraPos;
uniform vec3 u_sunDir;

out vec4 fragColor;

vec3 ApplyModernFog(vec3 color, float dist, vec2 fogRange, int fogEnable, vec4 fogColor)
{
    if (fogEnable == 0)
        return color;
    float fogStart = max(fogRange.x, 2000.0);
    float fogEnd = max(fogRange.y, fogStart + 2000.0);
    if (dist <= fogStart)
        return color;
    float t = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    float fogAmt = pow(smoothstep(0.0, 1.0, t), 1.25);
    return mix(color, fogColor.rgb, fogAmt);
}

void main()
{
    if (u_isShadowPass != 0)
    {
        vec3 shadow = vec3(1.0);

        if (u_isDungeonMap == 0 && u_useTileTexture != 0 &&
            v_texCoord0.x >= 0.0 && v_texCoord0.x <= 1.0 &&
            v_texCoord0.y >= 0.0 && v_texCoord0.y <= 1.0)
        {
            vec4 staticShadow = texture(u_tileTexture, v_texCoord0);
            shadow *= staticShadow.rgb;
        }

        if (u_useSplatAlpha != 0)
        {
            vec2 chrUV = vec2(v_texCoord1.x, 1.0 - v_texCoord1.y);
            if (chrUV.x >= 0.0 && chrUV.x <= 1.0 && chrUV.y >= 0.0 && chrUV.y <= 1.0)
            {
                vec4 chrShadow = texture(u_splatTexture, chrUV);
                shadow *= chrShadow.rgb;
            }
        }

        fragColor = vec4(shadow, 1.0);
        return;
    }

    if (u_useTileTexture == 0)
    {
        fragColor = vec4(u_fogColor.rgb, 1.0);
        return;
    }
    vec4 tileColor = texture(u_tileTexture, v_texCoord0);

    float alpha = 1.0;
    if (u_useSplatAlpha != 0)
    {
        alpha = texture(u_splatTexture, v_texCoord1).a;
        if (alpha <= 0.005)
            discard;
    }
    else
    {
        // DX9Ex Alpha Test: Discard transparent / void terrain textures.
        // In Metin2 dungeons, empty space / void is painted with field 01.dds which has alpha 0.0.
        // DirectX 9Ex has ALPHATESTENABLE=TRUE, ALPHAFUNC=GREATER, ALPHAREF=0 which discards these pixels,
        // allowing the 3D dungeon floor meshes (CDungeonBlock) underneath to be visible.
        if (tileColor.a <= 0.05)
            discard;
    }

    vec3 N = v_worldNormal;
    float lenN = length(N);
    if (lenN > 0.01)
        N = normalize(N);
    else
        N = vec3(0.0, 0.0, 1.0);
    float NdotL = max(dot(N, u_lightDir), 0.0);
    vec3 lighting = (u_isDungeonMap != 0 || (u_ambientColor.r >= 0.99 && u_lightColor.r <= 0.01)) ? vec3(1.0) : clamp(u_ambientColor + u_lightColor * NdotL, 0.85, 1.0);
    vec3 litColor = tileColor.rgb * lighting;
    vec3 finalColor = ApplyModernFog(litColor, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
    fragColor = vec4(finalColor, alpha);
}
)";
}
