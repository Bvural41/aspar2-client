#pragma once

namespace ShadersRender
{
static const char* s_defaultVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec2 uScreenSize;
uniform int uIs2D;

out vec4 vColor;
out vec2 vTexCoord;

void main() {
    if (uIs2D == 1) {
        float x = (aPos.x / uScreenSize.x) * 2.0 - 1.0;
        float y = 1.0 - (aPos.y / uScreenSize.y) * 2.0;
        gl_Position = vec4(x, y, 0.0, 1.0);
    } else {
        gl_Position = uProj * uView * uWorld * vec4(aPos.xyz, 1.0);
    }
    vColor = vec4(aColor.b, aColor.g, aColor.r, aColor.a);
    vTexCoord = aTexCoord;
}
)";

static const char* s_defaultFS = R"(#version 300 es
#ifdef GL_ES
precision mediump float;
#endif

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform int uUseTexture;

out vec4 FragColor;

void main() {
    vec4 tex = (uUseTexture != 0) ? texture(uTexture, vTexCoord) : vec4(1.0);
    FragColor = tex * vColor;
    if (FragColor.a < 0.01) discard;
}
)";
}
