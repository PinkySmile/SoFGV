precision mediump float;

attribute vec2 pos;
attribute vec4 color;
attribute vec2 coord;

// These must exactly match the fragment shader
varying mediump vec2 v_texCoord;
varying mediump vec4 v_color;

uniform mat4 sf_modelview;
uniform mat4 sf_projection;

void main()
{
    v_texCoord = coord;
    v_color = color;
    gl_Position = sf_projection * sf_modelview * vec4(pos, 0.0, 1.0);
}