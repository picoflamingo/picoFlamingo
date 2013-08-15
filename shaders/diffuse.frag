varying highp vec4     the_color;
varying highp vec4     the_normal;
varying highp vec4     the_light;
varying highp float    Diffuse;

const highp vec4 ambient_light = vec4 (0.1, 0.1, 0.1, 1.0);


void main (void)
{	

  highp vec3 nnormal = normalize (the_normal.xyz);
  highp vec3 nlightVec = normalize (the_light.xyz);

  highp float DiffuseTerm = clamp(dot(nnormal, nlightVec), 0.0, 1.0);

  gl_FragColor = ambient_light  +
                 (DiffuseTerm * the_color);


}
