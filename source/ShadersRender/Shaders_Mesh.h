#pragma once

namespace ShadersRender
{
static const char* s_meshVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec2 a_texCoord2;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_texMat1;
uniform int u_isShadowPass;
uniform int u_isPCBlockerPass;

out vec3 v_worldPos;
out vec3 v_worldNormal;
out vec2 v_texCoord;
out vec2 v_texCoord2;
out vec2 v_shadowUV;
out vec2 v_blockerUV;
out float v_fogDist;

void main()
{
    vec4 pos = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = pos;
    v_worldPos = (u_world * vec4(a_position, 1.0)).xyz;
    v_worldNormal = mat3(u_world) * a_normal;
    v_texCoord = a_texCoord;
    v_texCoord2 = a_texCoord2;
    v_fogDist = pos.w;

    if (u_isShadowPass != 0)
    {
        vec4 pCam = u_view * vec4(v_worldPos, 1.0);
        vec4 tc1 = u_texMat1 * pCam;
        v_shadowUV = tc1.xy;
    }
    else
    {
        v_shadowUV = vec2(0.0);
    }

    if (u_isPCBlockerPass != 0)
    {
        vec4 pCam = u_view * vec4(v_worldPos, 1.0);
        vec4 tc1 = u_texMat1 * pCam;
        v_blockerUV = tc1.xy;
    }
    else
    {
        v_blockerUV = vec2(0.0);
    }
}
)";

static const char* s_meshFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec3 v_worldPos;
in vec3 v_worldNormal;
in vec2 v_texCoord;
in vec2 v_texCoord2;
in vec2 v_shadowUV;
in vec2 v_blockerUV;
in float v_fogDist;

uniform sampler2D u_texture;
uniform sampler2D u_texture2;
uniform int u_useTexture;
uniform int u_useTexture2;
uniform int u_isDungeonMap;
uniform int u_isShadowPass;
uniform int u_isPCBlockerPass;
uniform int u_alphaTest;
uniform float u_alphaRef;
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
        vec2 chrUV = vec2(v_shadowUV.x, 1.0 - v_shadowUV.y);
        vec3 shadow = vec3(1.0);
        if (chrUV.x >= 0.0 && chrUV.x <= 1.0 && chrUV.y >= 0.0 && chrUV.y <= 1.0)
        {
            shadow = texture(u_texture2, chrUV).rgb;
        }
        fragColor = vec4(shadow, 1.0);
    }
    else
    {
        vec4 texColor = vec4(1.0);
        if (u_useTexture != 0)
            texColor = texture(u_texture, v_texCoord);
        if (u_useTexture2 != 0)
        {
            vec4 tex2 = texture(u_texture2, v_texCoord2);
            texColor *= tex2;
        }
        if (u_alphaTest != 0 && texColor.a < u_alphaRef)
            discard;
        vec3 N = normalize(v_worldNormal);
        float NdotL = max(dot(N, u_lightDir), 0.0);
        vec3 lighting = (u_isDungeonMap != 0 || u_useTexture2 != 0) ? vec3(1.0) : (u_ambientColor + u_lightColor * NdotL);
        vec3 litColor = texColor.rgb * lighting;
        vec3 finalColor = ApplyModernFog(litColor, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
        float outAlpha = (u_useTexture != 0) ? texColor.a : 1.0;
        if (u_isPCBlockerPass != 0)
        {
            vec2 bUV = clamp(v_blockerUV, 0.0, 1.0);
            float blockerAlpha = texture(u_texture2, bUV).a;
            outAlpha *= blockerAlpha;
        }
        fragColor = vec4(finalColor, outAlpha);
    }
}
)";

static const char* s_mesh2TexVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_texMat1;

out vec3 v_worldPos;
out vec3 v_worldNormal;
out vec2 v_texCoord;
out vec2 v_texCoord2;
out float v_fogDist;

void main()
{
    vec4 pos = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = pos;
    v_worldPos = (u_world * vec4(a_position, 1.0)).xyz;
    v_worldNormal = mat3(u_world) * a_normal;
    v_texCoord = a_texCoord;
    v_texCoord2 = (u_texMat1 * vec4(a_texCoord, 0.0, 1.0)).xy;
    v_fogDist = pos.w;
}
)";

static const char* s_mesh2TexFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec3 v_worldPos;
in vec3 v_worldNormal;
in vec2 v_texCoord;
in vec2 v_texCoord2;
in float v_fogDist;

uniform sampler2D u_texture;
uniform sampler2D u_texture2;
uniform int u_useTexture;
uniform int u_useTexture2;
uniform int u_alphaTest;
uniform float u_alphaRef;
uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform vec4 u_fogColor;
uniform vec2 u_fogRange;
uniform int u_fogEnable;

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
    vec4 tex0 = (u_useTexture != 0) ? texture(u_texture, v_texCoord) : vec4(1.0);
    vec4 tex1 = (u_useTexture2 != 0) ? texture(u_texture2, v_texCoord2) : vec4(1.0);
    vec4 texColor = tex0 * tex1;

    if (u_alphaTest != 0 && texColor.a < u_alphaRef)
        discard;

    vec3 N = normalize(v_worldNormal);
    float NdotL = max(dot(N, u_lightDir), 0.0);
    vec3 lighting = u_ambientColor + u_lightColor * NdotL;
    vec3 litColor = texColor.rgb * lighting;
    vec3 finalColor = ApplyModernFog(litColor, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
    float outAlpha = (u_useTexture != 0) ? texColor.a : 1.0;
    fragColor = vec4(finalColor, outAlpha);
}
)";

static const char* s_meshSkinnedVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec4 a_blendWeights;
layout(location = 4) in uvec4 a_blendIndices;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_boneMatrices[64];
uniform int u_useSkinning;

out vec3 v_worldPos;
out vec3 v_worldNormal;
out vec2 v_texCoord;
out float v_fogDist;

void main()
{
    vec4 inputPos = vec4(a_position, 1.0);
    vec4 skinnedPos = vec4(0.0);
    vec3 skinnedNormal = vec3(0.0);

    if (u_useSkinning != 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            float weight = a_blendWeights[i];
            if (weight > 0.0)
            {
                uint idx = a_blendIndices[i];
                if (idx < 64u)
                {
                    mat4 bone = u_boneMatrices[idx];
                    skinnedPos += weight * (bone * inputPos);
                    skinnedNormal += weight * (mat3(bone) * a_normal);
                }
            }
        }
        float totalWeight = a_blendWeights.x + a_blendWeights.y + a_blendWeights.z + a_blendWeights.w;
        if (totalWeight < 0.001)
        {
            skinnedPos = inputPos;
            skinnedNormal = a_normal;
        }
        else
        {
            skinnedPos.w = 1.0;
            float nLen = length(skinnedNormal);
            if (nLen > 0.0001)
                skinnedNormal /= nLen;
            else
                skinnedNormal = a_normal;
        }
    }
    else
    {
        skinnedPos = inputPos;
        skinnedNormal = a_normal;
    }

    vec4 pos = u_worldViewProj * skinnedPos;
    gl_Position = pos;
    v_worldPos = (u_world * skinnedPos).xyz;
    v_worldNormal = mat3(u_world) * skinnedNormal;
    v_texCoord = a_texCoord;
    v_fogDist = pos.w;
}
)";

static const char* s_meshVTF_VS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform sampler2D u_texInstanceData;
uniform int u_useVTF;

out vec3 v_worldPos;
out vec3 v_worldNormal;
out vec2 v_texCoord;
out float v_fogDist;

void main()
{
    mat4 instWorld = u_world;
    if (u_useVTF != 0)
    {
        int baseTexel = gl_InstanceID * 4;
        vec4 row0 = texelFetch(u_texInstanceData, ivec2(baseTexel + 0, 0), 0);
        vec4 row1 = texelFetch(u_texInstanceData, ivec2(baseTexel + 1, 0), 0);
        vec4 row2 = texelFetch(u_texInstanceData, ivec2(baseTexel + 2, 0), 0);
        vec4 row3 = texelFetch(u_texInstanceData, ivec2(baseTexel + 3, 0), 0);
        instWorld = mat4(row0, row1, row2, row3);
    }

    vec4 worldPos = instWorld * vec4(a_position, 1.0);
    vec4 pos = (u_useVTF != 0) ? (u_proj * u_view * worldPos) : (u_worldViewProj * vec4(a_position, 1.0));
    gl_Position = pos;
    v_worldPos = worldPos.xyz;
    v_worldNormal = mat3(instWorld) * a_normal;
    v_texCoord = a_texCoord;
    v_fogDist = pos.w;
}
)";
}
