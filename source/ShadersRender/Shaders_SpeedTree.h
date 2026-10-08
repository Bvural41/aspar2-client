#pragma once

namespace ShadersRender
{
static const char* s_speedTreeVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;

out vec3 v_worldPos;
out vec4 v_color;
out vec2 v_texCoord;
out float v_fogDist;

void main()
{
    vec4 pos = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = pos;
    v_worldPos = (u_world * vec4(a_position, 1.0)).xyz;
    vec3 col = (length(a_color.rgb) > 0.05) ? vec3(a_color.b, a_color.g, a_color.r) : vec3(1.0);
    v_color = vec4(col, a_color.a);
    v_texCoord = a_texCoord;
    v_fogDist = pos.w;
}
)";

static const char* s_speedTreeFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec3 v_worldPos;
in vec4 v_color;
in vec2 v_texCoord;
in float v_fogDist;

uniform sampler2D u_texture;
uniform int u_alphaTest;
uniform float u_alphaRef;
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
    vec4 texColor = texture(u_texture, v_texCoord);
    vec3 lightCol = (dot(v_color.rgb, v_color.rgb) > 0.05) ? v_color.rgb : vec3(1.0);
    vec4 finalColor = vec4(texColor.rgb * lightCol, texColor.a);
    if (u_alphaTest != 0 && texColor.a <= u_alphaRef)
        discard;
    finalColor.rgb = ApplyModernFog(finalColor.rgb, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
    fragColor = finalColor;
}
)";

static const char* s_speedTreeLeafVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec4 a_leafData;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform vec4 u_treePos;

out vec3 v_worldPos;
out vec4 v_color;
out vec2 v_texCoord;
out float v_fogDist;

void main()
{
    vec3 localPos = a_position;
    vec4 worldPos = u_world * vec4(localPos, 1.0);
    vec4 pos = u_worldViewProj * vec4(localPos, 1.0);
    gl_Position = pos;
    v_worldPos = worldPos.xyz;
    vec3 col = (length(a_color.rgb) > 0.05) ? vec3(a_color.b, a_color.g, a_color.r) : vec3(1.0);
    v_color = vec4(col, a_color.a);
    v_texCoord = a_texCoord;
    v_fogDist = pos.w;
}
)";

static const char* s_speedTreeLeafFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec3 v_worldPos;
in vec4 v_color;
in vec2 v_texCoord;
in float v_fogDist;

uniform sampler2D u_texture;
uniform int u_alphaTest;
uniform float u_alphaRef;
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
    vec4 texColor = texture(u_texture, v_texCoord);
    vec3 lightCol = (dot(v_color.rgb, v_color.rgb) > 0.05) ? v_color.rgb : vec3(1.0);
    vec4 finalColor = vec4(texColor.rgb * lightCol, texColor.a);
    if (u_alphaTest != 0 && texColor.a <= u_alphaRef)
        discard;
    finalColor.rgb = ApplyModernFog(finalColor.rgb, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
    fragColor = finalColor;
}
)";

static const char* s_speedTreeVTF_VS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec2 a_shadowCoord;
layout(location = 4) in vec2 a_windData;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform sampler2D u_texInstanceData;
uniform int u_useVTF;

out vec4 v_color;
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

    vec3 col = (length(a_color.rgb) > 0.05) ? vec3(a_color.b, a_color.g, a_color.r) : vec3(1.0);
    v_color = vec4(col, a_color.a);
    v_texCoord = a_texCoord;
    v_fogDist = pos.w;
}
)";

static const char* s_speedTreeVTF_FS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec4 v_color;
in vec2 v_texCoord;
in float v_fogDist;

uniform sampler2D u_texture;
uniform int u_alphaTest;
uniform float u_alphaRef;
uniform vec4 u_fogColor;
uniform vec2 u_fogRange;
uniform int u_fogEnable;
uniform int u_isShadowPass;

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
    vec4 texColor = texture(u_texture, v_texCoord);
    if (u_alphaTest != 0 && texColor.a <= u_alphaRef)
        discard;

    if (u_isShadowPass != 0)
    {
        fragColor = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
        return;
    }

    vec3 lightCol = (dot(v_color.rgb, v_color.rgb) > 0.05) ? v_color.rgb : vec3(1.0);
    vec4 finalColor = vec4(texColor.rgb * lightCol, texColor.a);
    finalColor.rgb = ApplyModernFog(finalColor.rgb, v_fogDist, u_fogRange, u_fogEnable, u_fogColor);
    fragColor = finalColor;
}
)";
}
