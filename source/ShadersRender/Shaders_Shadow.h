#pragma once

namespace ShadersRender
{
static const char* s_shadowVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;

out vec2 v_texCoord;
out float v_depth;

void main()
{
    vec4 pos = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = pos;
    v_texCoord = a_texCoord;
    v_depth = pos.z / pos.w;
}
)";

static const char* s_shadowFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec2 v_texCoord;
in float v_depth;

uniform sampler2D u_texture;
uniform int u_alphaTest;
uniform float u_alphaRef;

out vec4 fragColor;

void main()
{
    if (u_alphaTest != 0)
    {
        vec4 texColor = texture(u_texture, v_texCoord);
        if (texColor.a < u_alphaRef)
            discard;
    }
    fragColor = vec4(v_depth, 0.0, 0.0, 1.0);
}
)";

static const char* s_shadowSkinnedVS = R"(#version 300 es
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
uniform mat4 u_boneMatrices[64];
uniform int u_useSkinning;

out vec2 v_texCoord;
out float v_depth;

void main()
{
    vec4 inputPos = vec4(a_position, 1.0);
    vec4 skinnedPos = vec4(0.0);

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
                    skinnedPos += weight * (u_boneMatrices[idx] * inputPos);
                }
            }
        }
        float totalWeight = a_blendWeights.x + a_blendWeights.y + a_blendWeights.z + a_blendWeights.w;
        if (totalWeight < 0.001)
            skinnedPos = inputPos;
        else
            skinnedPos.w = 1.0;
    }
    else
    {
        skinnedPos = inputPos;
    }

    vec4 pos = u_worldViewProj * skinnedPos;
    gl_Position = pos;
    v_texCoord = a_texCoord;
    v_depth = pos.z / pos.w;
}
)";

static const char* s_shadowVTF_VS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
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

out vec2 v_texCoord;
out float v_depth;

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
    v_texCoord = a_texCoord;
    v_depth = pos.z / pos.w;
}
)";
}
