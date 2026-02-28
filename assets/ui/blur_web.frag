precision mediump float;

// Exactly matching the vertex shader
varying mediump vec2 v_texCoord;
varying mediump vec4 v_color;

uniform sampler2D sf_texture;
uniform vec2 factor_npot;
uniform vec2 offsetFactor;

void main()
{
    // Use mediump for local math to be safe
    mediump vec2 uv = v_texCoord * factor_npot;
    mediump vec4 outColor = vec4(0.0);

    outColor += texture2D(sf_texture, uv - 4.0 * offsetFactor) * 0.0162162162;
    outColor += texture2D(sf_texture, uv - 3.0 * offsetFactor) * 0.0540540541;
    outColor += texture2D(sf_texture, uv - 2.0 * offsetFactor) * 0.1216216216;
    outColor += texture2D(sf_texture, uv - offsetFactor) * 0.1945945946;
    outColor += texture2D(sf_texture, uv) * 0.2270270270;
    outColor += texture2D(sf_texture, uv + offsetFactor) * 0.1945945946;
    outColor += texture2D(sf_texture, uv + 2.0 * offsetFactor) * 0.1216216216;
    outColor += texture2D(sf_texture, uv + 3.0 * offsetFactor) * 0.0540540541;
    outColor += texture2D(sf_texture, uv + 4.0 * offsetFactor) * 0.0162162162;

    gl_FragColor = outColor * v_color;
}