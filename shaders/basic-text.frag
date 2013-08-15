uniform sampler2D sampler2d;

varying highp vec4 the_color;
varying highp vec2 myTexCoord;

void main (void)
{
	gl_FragColor = the_color * texture2D(sampler2d,myTexCoord);

}
