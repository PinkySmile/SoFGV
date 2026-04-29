#version 100
attribute vec2 position;
attribute vec4 color;
attribute vec2 texCoord;
varying vec4 sf_color;
varying vec2 sf_texCoord;
uniform mat4 sf_modelview;
uniform mat4 sf_projection;
void main()
{
    vec2 pos = position;
    sf_color = color;
    sf_texCoord = texCoord;
    gl_Position = sf_projection * sf_modelview * vec4(pos.xy, 0.0, 1.0);
}