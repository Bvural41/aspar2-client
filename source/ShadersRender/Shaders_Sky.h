#pragma once

namespace ShadersRender
{
static const char* s_skyVS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_worldViewProj;
uniform mat4 u_world;
uniform mat4 u_texMat0;
uniform int u_isCloud;
uniform float u_time;

out vec4 v_color;
out vec2 v_texCoord;
out vec2 v_texCoord2;
out vec3 v_worldPos;
out vec2 v_localPos;
out float v_height;

void main()
{
    vec4 worldPos = u_world * vec4(a_position, 1.0);
    vec4 p = u_worldViewProj * vec4(a_position, 1.0);
    gl_Position = vec4(p.xy, p.w * 0.9999, p.w);
    v_worldPos = worldPos.xyz;
    v_color = vec4(a_color.b, a_color.g, a_color.r, a_color.a);
    v_localPos = a_position.xy;
    v_height = (1.0 - a_position.z) * 0.5;
    if (u_isCloud != 0)
    {
        vec2 cloudUV = vec2(a_texCoord.x * u_texMat0[0][0], a_texCoord.y * u_texMat0[1][1]) + u_texMat0[3].xy + u_texMat0[2].xy;
        v_texCoord = cloudUV;
        vec2 tc2 = cloudUV * 0.7;
        tc2.x += u_time * 0.005;
        tc2.y += u_time * 0.003;
        v_texCoord2 = tc2;
    }
    else
    {
        v_texCoord = a_texCoord;
        v_texCoord2 = a_texCoord;
    }
}
)";

static const char* s_skyFS = R"(#version 300 es
#ifdef GL_ES
precision highp float;
precision highp int;
#endif

uniform sampler2D u_texture;
uniform int u_isCloud;
uniform int u_useTexture;
uniform float u_time;
uniform vec3 u_cameraPos;
uniform vec4 u_skyColors[16];
uniform int u_skyColorCount;
uniform int u_skyMode;
uniform vec3 u_sunDirection;
uniform float u_sunIntensity;

in vec4 v_color;
in vec2 v_texCoord;
in vec2 v_texCoord2;
in vec3 v_worldPos;
in vec2 v_localPos;
in float v_height;

out vec4 fragColor;

float hash2d(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise2d(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash2d(i);
    float b = hash2d(i + vec2(1.0, 0.0));
    float c = hash2d(i + vec2(0.0, 1.0));
    float d = hash2d(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm2d(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 3; i++)
    {
        value += amplitude * noise2d(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    if (u_skyMode == 0)
    {
        vec4 color = v_color;
        if (u_skyColorCount > 1)
        {
            float t = clamp(v_height, 0.0, 1.0) * float(u_skyColorCount - 1);
            int seg = min(int(t), u_skyColorCount - 2);
            float f = t - float(seg);
            f = f * f * (3.0 - 2.0 * f);
            color = mix(u_skyColors[seg], u_skyColors[seg + 1], f);
        }

        vec3 viewDir = normalize(v_worldPos - u_cameraPos);
        vec3 sunDir = normalize(u_sunDirection);
        float sunDot = max(0.0, dot(viewDir, sunDir));
        float sunDisc = smoothstep(0.998, 0.9995, sunDot);
        float sunGlow = pow(sunDot, 32.0) * 0.5 + pow(sunDot, 8.0) * 0.2;
        vec3 sunColor = vec3(1.0, 0.95, 0.8);
        color.rgb += sunDisc * sunColor * 1.5 + sunGlow * sunColor * u_sunIntensity;

        fragColor = vec4(color.rgb, 1.0);
        return;
    }

    if (u_skyMode == 1)
    {
        vec4 texColor = texture(u_texture, v_texCoord);
        fragColor = vec4(texColor.rgb, 1.0);
        return;
    }

    vec3 sunDir = normalize(u_sunDirection);
    float sIntensity = u_sunIntensity;

    vec2 distortion = vec2(
        fbm2d(v_texCoord * 3.0 + u_time * 0.1) - 0.5,
        fbm2d(v_texCoord * 3.0 + vec2(5.2, 1.3) + u_time * 0.08) - 0.5
    ) * 0.03;

    vec2 uv1 = v_texCoord + distortion;
    vec4 cloud1 = texture(u_texture, uv1);
    vec2 uv2 = v_texCoord2 + distortion * 0.7;
    vec4 cloud2 = texture(u_texture, uv2);
    vec4 clouds = cloud1 * 0.6 + cloud2 * 0.4;

    float rgbMax = max(clouds.r, max(clouds.g, clouds.b));
    float invAlpha = clamp((1.0 - clouds.a) / 0.26, 0.0, 1.0);
    float cloudDensity = (rgbMax > 0.02) ? rgbMax : invAlpha;
    float softAlpha = cloudDensity * cloudDensity * (3.0 - 2.0 * cloudDensity);
    if (dot(v_color.rgb, v_color.rgb) < 0.001 && v_color.a < 0.001)
    {
        discard;
    }

    vec3 vCol = v_color.rgb;

    vec3 viewDir = normalize(v_worldPos - u_cameraPos);
    float sunDot = dot(viewDir, sunDir);
    float rimLight = clamp(pow(clamp(sunDot + 0.2, 0.0, 1.0), 3.0), 0.0, 1.0);

    float sunHeight = clamp(sunDir.z, 0.0, 1.0);
    vec3 warmColor = vec3(1.0, 0.85, 0.65);
    vec3 coolColor = vec3(0.9, 0.95, 1.0);
    vec3 sunTint = mix(warmColor, coolColor, sunHeight);

    vec3 cloudColor = vCol * (softAlpha * 0.75);
    vec3 rimColor = sunTint * rimLight * sIntensity * 0.35;
    cloudColor += rimColor * softAlpha;

    float sunProximity = clamp(pow(clamp(sunDot, 0.0, 1.0), 8.0), 0.0, 1.0);
    cloudColor += sunTint * sunProximity * sIntensity * 0.2 * softAlpha;

    float edgeDist = max(abs(v_localPos.x), abs(v_localPos.y));
    float edgeFade = 1.0 - clamp((edgeDist - 0.7) / 0.3, 0.0, 1.0);

    fragColor = vec4(cloudColor * edgeFade, softAlpha * edgeFade);
}
)";
}
