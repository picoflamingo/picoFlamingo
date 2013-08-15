attribute highp vec4    myVertex;
attribute highp vec4    myColor;
attribute highp vec4    myNormal;

uniform mediump mat4    myPMVMatrix;
uniform mediump mat4    myModelViewIT; 
uniform mediump vec3    lightPosition;

varying highp vec4     the_color;
varying highp float    Diffuse;

void main(void)			
{			
	//Diffuse = max(dot(normalize (myPMVMatrix * myNormal).xyz, normalize(lightPosition)), 0.0);
	Diffuse = max(dot(normalize (myModelViewIT * myNormal).xyz, normalize(lightPosition)), 0.0);

	the_color = myColor;

	gl_Position = myPMVMatrix * myVertex;
}
