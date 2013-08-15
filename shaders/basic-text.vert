attribute highp vec4    myVertex;
attribute highp vec4    myColor;
attribute mediump vec4  myUV;

uniform mediump mat4    myPMVMatrix;

varying highp vec2    myTexCoord;
varying highp vec4     the_color;


void main(void)
{
	gl_Position = myPMVMatrix * myVertex;
	the_color = myColor;
        myTexCoord = myUV.st;
 }

