#pragma once

namespace ShadersRender
{
static const char* s_bloomBrightFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec4 u_bloomParams;
uniform vec4 u_texelSize;
uniform vec4 u_blurDirection;

out vec4 fragColor;

void main()
{
    vec3 color = texture(u_texture, v_texCoord).rgb;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float threshold = u_bloomParams.x;
    float knee = threshold * 0.5;
    float soft = luminance - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);
    float contribution = max(soft, luminance - threshold) / max(luminance, 0.00001);

    fragColor = vec4(color * contribution * u_bloomParams.y, 1.0);
}
)";

static const char* s_bloomBlurFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform vec4 u_texelSize;
uniform vec4 u_blurDirection;

out vec4 fragColor;

const float weights[5] = float[5](0.227027, 0.194594, 0.121622, 0.054054, 0.016216);

void main()
{
    vec2 dir = vec2(u_blurDirection.x * u_texelSize.x, u_blurDirection.y * u_texelSize.y);
    vec3 result = texture(u_texture, v_texCoord).rgb * weights[0];

    for (int i = 1; i < 5; i++)
    {
        vec2 offset = dir * float(i);
        result += texture(u_texture, v_texCoord + offset).rgb * weights[i];
        result += texture(u_texture, v_texCoord - offset).rgb * weights[i];
    }

    fragColor = vec4(result, 1.0);
}
)";

static const char* s_bloomCompositeFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec2 v_texCoord;

uniform sampler2D u_texScene;
uniform sampler2D u_texBloom;
uniform sampler2D u_texGodRays;
uniform sampler2D u_texSSAO;
uniform int u_enableBloom;
uniform int u_enableGodRays;
uniform int u_enableSSAO;

out vec4 fragColor;

void main()
{
    vec3 scene = texture(u_texScene, v_texCoord).rgb;
    vec3 bloom = (u_enableBloom != 0) ? texture(u_texBloom, v_texCoord).rgb : vec3(0.0);
    vec3 godRays = (u_enableGodRays != 0) ? texture(u_texGodRays, v_texCoord).rgb : vec3(0.0);
    float ao = 1.0;
    if (u_enableSSAO != 0)
    {
        vec4 aoSample = texture(u_texSSAO, v_texCoord);
        ao = (aoSample.a > 0.5) ? aoSample.r : 1.0;
    }
    fragColor = vec4(scene * ao + bloom + godRays, 1.0);
}
)";

static const char* s_godraysVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
)";

static const char* s_godraysFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

in vec2 v_texCoord;

uniform sampler2D u_texScene;
uniform vec4 u_lightScreenPos;
uniform vec4 u_rayParams;
uniform vec4 u_rayColor;

out vec4 fragColor;

void main()
{
    vec2 lightPos = u_lightScreenPos.xy;
    float intensity = u_lightScreenPos.z;
    float decay = u_lightScreenPos.w;

    float density = u_rayParams.x;
    float weight = u_rayParams.y;
    float exposure = u_rayParams.z;
    int numSamples = int(u_rayParams.w);
    if (numSamples <= 0 || numSamples > 64)
        numSamples = 32;

    vec2 deltaTexCoord = v_texCoord - lightPos;
    deltaTexCoord *= (1.0 / float(numSamples)) * density;

    vec2 uv = v_texCoord;
    float illuminationDecay = 1.0;
    vec3 color = vec3(0.0);

    for (int i = 0; i < numSamples; i++)
    {
        uv -= deltaTexCoord;
        vec3 s = texture(u_texScene, clamp(uv, 0.0, 1.0)).rgb;
        s *= illuminationDecay * weight;
        color += s;
        illuminationDecay *= decay;
    }

    color *= exposure * intensity;
    color *= u_rayColor.rgb;

    fragColor = vec4(color, 1.0);
}
)";
}
