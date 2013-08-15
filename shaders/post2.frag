uniform sampler2D sampler2d;

//varying highp vec4 the_color;
//varying mediump vec2    myTexCoord;

varying mediump vec2  TexCoord;

void main (void)
{

  //highp float k1 = -0.1;
  highp float k1 = 0.1;
  //highp vec2 screenDimen = vec2 (800, 600);
  highp vec2 screenDimen = vec2 (1, 1);
  //highp vec2 texScale = vec2 (1.0, 1.0);
  highp vec2 texScale = vec2 (0.95, 0.95);
  //highp vec2 texScale = vec2 (1.2, 1.2);


  // DEBUG: Just render texture
  //gl_FragColor =  texture2D(sampler2d, TexCoord);


    // We are rendering onto a texture rectangle and texture
    // coordinates are [0,0] to (W,H) instead of [0,0] to (1,1).

    highp vec2 screenCenter = screenDimen/2.0;  // Find the screen center 
    highp float norm = length(screenCenter);  // Distance between corner and center 

    // get a vector from center to where we are at now (in screen space) and normalize it
    highp vec2 radial_vector = ( TexCoord - screenCenter ) / norm;
    highp float radial_vector_len = length(radial_vector);
    highp vec2 radial_vector_unit = radial_vector / radial_vector_len;


    // Compute the new distance from the screen center.
    highp float new_dist = radial_vector_len + k1 * pow(radial_vector_len,3.0);

    // Now, compute texture coordinate we want to lookup. 

    // Find the coordinate we want to lookup
    highp vec2 warp_coord = radial_vector_unit * (new_dist * norm);

    // Scale the image vertically and horizontally to get it to fill the screen
    warp_coord = warp_coord * texScale;

    // Translate the coordinte such that the (0,0) is back at the screen center
    warp_coord = warp_coord + screenCenter;


    // If we lookup a texture coordinate that is not on the texture, return a solid color 
    if ((warp_coord.s > float(screenDimen.x)  || warp_coord.s < 0.0) ||
	(warp_coord.t > float(screenDimen.y) || warp_coord.t < 0.0))
        gl_FragColor = vec4(0,0,0,1); // black
    else
	gl_FragColor = texture2D(sampler2d, warp_coord);  // lookup into the texture

}
