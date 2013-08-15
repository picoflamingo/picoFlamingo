attribute highp vec4    myVertex;
attribute highp vec4    myColor;
//attribute highp vec4    myNormal;
attribute highp vec3    myNormal;
attribute highp vec2    mytex;

uniform mediump mat4    myPMVMatrix;
uniform mediump mat4    myModelViewIT; 
uniform mediump vec3    lightPosition;

varying highp vec4     the_color;
varying highp vec4     the_normal;
varying highp vec4     the_light;
varying highp float    Diffuse;

void main(void)			
{			

        the_normal = -myModelViewIT * vec4 (myNormal, 1.0);
	vec4 ecPos = myPMVMatrix * myVertex;
	the_light = normalize (vec4 (lightPosition, 1.0) - ecPos);

	the_color = myColor;

	gl_Position = myPMVMatrix * myVertex;
}
