attribute highp vec4    myVertex;
attribute highp vec4    myColor;
attribute highp vec4    myNormal;
attribute mediump vec2  myUV;


uniform mediump mat4    myPMVMatrix;
uniform mediump mat4    myModelViewIT; 
uniform mediump vec3    lightPosition;

//varying highp vec4     the_color;
//varying highp float    Diffuse;

varying mediump vec2  TexCoord;

void main(void)			
{			
	//Diffuse = max(dot(normalize (myModelViewIT * myNormal), normalize(lightPosition)), 0.0);
	//the_color = myColor;

	//gl_Position = myPMVMatrix * myVertex;
	gl_Position = myVertex;
	TexCoord = myUV;
}
