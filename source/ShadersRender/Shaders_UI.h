#pragma once

namespace ShadersRender
{
static const char* s_uiVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_projMatrix;
uniform mat4 u_world;
uniform mat4 u_texMat1;

out vec4 v_color;
out vec2 v_texCoord;
out vec2 v_maskCoord;

void main()
{
    gl_Position = u_projMatrix * vec4(a_position, 1.0);
    v_color = vec4(a_color.b, a_color.g, a_color.r, a_color.a);
    v_texCoord = a_texCoord;
    vec4 worldPos = u_world * vec4(a_position, 1.0);
    v_maskCoord = vec2(
        worldPos.x * u_texMat1[0][0] + u_texMat1[3][0],
        worldPos.y * u_texMat1[1][1] + u_texMat1[3][1]
    );
}
)";

static const char* s_uiFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec4 v_color;
in vec2 v_texCoord;
in vec2 v_maskCoord;

uniform sampler2D u_texture;
uniform sampler2D u_maskTexture;
uniform int u_useTexture;
uniform int u_useMask;
uniform int u_useTFactor;
uniform vec4 u_tfactorColor;

out vec4 fragColor;

void main()
{
    if (u_useTexture != 0)
        fragColor = texture(u_texture, v_texCoord) * v_color;
    else
        fragColor = v_color;

    if (u_useTFactor == 1)
    {
        fragColor.rgb *= u_tfactorColor.rgb;
        fragColor.a *= u_tfactorColor.a;
    }
    else if (u_useTFactor == 2)
    {
        fragColor.rgb = u_tfactorColor.rgb;
        if (u_useTexture == 0)
            fragColor.a = u_tfactorColor.a;
    }
    else if (u_useTFactor == 3)
    {
        fragColor.rgb += u_tfactorColor.rgb;
    }

    if (u_useMask != 0)
    {
        if (v_maskCoord.x < 0.0 || v_maskCoord.x > 1.0 || v_maskCoord.y < 0.0 || v_maskCoord.y > 1.0)
        {
            discard;
        }
        vec4 maskColor = texture(u_maskTexture, v_maskCoord);
        fragColor.a *= maskColor.a;
        fragColor.rgb *= maskColor.rgb;
    }

    if (fragColor.a < 0.01)
        discard;
}
)";
}
