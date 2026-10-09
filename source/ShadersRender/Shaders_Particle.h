#pragma once

namespace ShadersRender
{
static const char* s_particleVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;
uniform int u_useVertexColor;
uniform vec4 u_particleColor;

out vec4 v_color;
out vec2 v_texCoord;

void main()
{
    gl_Position = u_worldViewProj * vec4(a_position, 1.0);
    v_texCoord = a_texCoord;
    if (u_useVertexColor != 0)
        v_color = vec4(a_color.b, a_color.g, a_color.r, a_color.a);
    else
        v_color = u_particleColor;
}
)";

static const char* s_particlePCT_VS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;

out vec4 v_color;
out vec2 v_texCoord;

void main()
{
    gl_Position = u_worldViewProj * vec4(a_position, 1.0);
    v_texCoord = a_texCoord;
    v_color = vec4(a_color.b, a_color.g, a_color.r, a_color.a);
}
)";

static const char* s_particleFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec4 v_color;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform int u_useTexture;
uniform int u_colorOp;

out vec4 fragColor;

void main()
{
    vec4 texColor = (u_useTexture != 0) ? texture(u_texture, v_texCoord) : vec4(1.0);
    vec4 finalColor;
    finalColor.a = texColor.a * v_color.a;

    if (u_colorOp == 1)
    {
        discard;
    }
    else if (u_colorOp == 2)
    {
        finalColor.rgb = v_color.rgb;
    }
    else if (u_colorOp == 3)
    {
        finalColor.rgb = texColor.rgb;
    }
    else if (u_colorOp == 5)
    {
        finalColor.rgb = clamp(texColor.rgb * v_color.rgb * 2.0, 0.0, 1.0);
    }
    else if (u_colorOp == 6)
    {
        finalColor.rgb = clamp(texColor.rgb * v_color.rgb * 4.0, 0.0, 1.0);
    }
    else if (u_colorOp == 7)
    {
        finalColor.rgb = clamp(texColor.rgb + v_color.rgb, 0.0, 1.0);
    }
    else
    {
        finalColor.rgb = texColor.rgb * v_color.rgb;
    }

    if (finalColor.a < 0.004)
        discard;

    fragColor = finalColor;
}
)";
}
