/*
   Author: Themaister
   License: Public domain
*/

uniform vec2 color_texture_sz;
uniform vec2 color_texture_pow2_sz;
uniform vec2 output_size;

attribute vec4 a_position;
attribute vec2 a_texCoord;

varying vec2 texCoord;

varying vec2 sine_coord_omega;

void main()
{
   gl_Position = a_position;
   texCoord = a_texCoord;

//   float2 texsize = IN.texture_size;
//   float2 delta = 0.5 / texsize;
//   float dx = delta.x;
//   float dy = delta.y;

//	sine_coord_omega = vec2(3.1415 * IN.output_size.x * IN.texture_size.x / IN.video_size.x, 2.0 * 3.1415 * IN.texture_size.y);
	sine_coord_omega = vec2(3.1415 * (2.0 * color_texture_sz.x), 2.0 * 3.1415 * color_texture_sz.y);
}
