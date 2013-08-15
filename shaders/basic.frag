varying highp vec4 the_color;
varying highp float Diffuse;


void main (void)
{	
	//highp vec4 ambient = vec4(0.1, 0.1, 1.0, 0.0);
	//gl_FragColor = ambient + Diffuse * the_color;
	//gl_FragColor = Diffuse * the_color;
	gl_FragColor = the_color;
}
