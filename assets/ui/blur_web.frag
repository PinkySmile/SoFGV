#version 100
precision mediump float;
varying vec4 sf_color;
varying vec2 sf_texCoord;
uniform sampler2D sf_sampler;
uniform mat4 sf_texture;
uniform vec2 factor_npot;
uniform float sf_repeated;
uniform vec2 offsetFactor;

void main()
{
    vec4 coord = sf_texture * vec4(sf_texCoord, 0.0, 1.0);

    if (sf_repeated > 0.5)
        coord.xy = mod(coord.xy, factor_npot.xy);
    else
        coord.xy = clamp(coord.xy, vec2(0.0), factor_npot.xy);

    mediump vec2 uv = coord.xy;
    mediump vec4 outColor = vec4(0.0);

    outColor += texture2D(sf_sampler, uv - 4.0 * offsetFactor) * 0.0162162162;
    outColor += texture2D(sf_sampler, uv - 3.0 * offsetFactor) * 0.0540540541;
    outColor += texture2D(sf_sampler, uv - 2.0 * offsetFactor) * 0.1216216216;
    outColor += texture2D(sf_sampler, uv - offsetFactor) * 0.1945945946;
    outColor += texture2D(sf_sampler, uv) * 0.2270270270;
    outColor += texture2D(sf_sampler, uv + offsetFactor) * 0.1945945946;
    outColor += texture2D(sf_sampler, uv + 2.0 * offsetFactor) * 0.1216216216;
    outColor += texture2D(sf_sampler, uv + 3.0 * offsetFactor) * 0.0540540541;
    outColor += texture2D(sf_sampler, uv + 4.0 * offsetFactor) * 0.0162162162;

    gl_FragColor = outColor * sf_color;
}