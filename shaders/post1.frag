uniform sampler2D sampler2d;

//varying highp vec4 the_color;
//varying mediump vec2    myTexCoord;

varying mediump vec2  TexCoord;

void main (void)
{
	//gl_FragColor = vec4 (1.0, 0.0 ,0.0, 1.0);
	gl_FragColor =  texture2D(sampler2d, TexCoord);
}
