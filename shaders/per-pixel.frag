varying highp vec4 the_color;
varying highp vec4     the_normal;
varying highp vec4     the_view;
varying highp vec4     the_light;
varying highp float    Diffuse;

const highp float specAmount = 0.9;
const highp float diffuseAmount = 1.0 - specAmount;
const highp float diffuse_light = 0.9;
//const highp vec4 ambient_light = vec4 (0.7, 0.7, 0.7, 0.0);
const highp vec4 ambient_light = vec4 (0.5, 0.5, 0.5, 0.0);
//const highp vec4 specular_light = vec4 (0.3, 0.3, 0.3, 0.0);
const highp vec4 specular_light = vec4 (0.5, 0.5, 0.5, 0.0);

highp vec4 material_ambient = vec4 (0.5, 0.5, 0.5, 1.0);
//highp vec4 material_diffuse = vec4 (0.7, 0.7, 0.7, 1.0);
highp vec4 material_diffuse = vec4 (0.5, 0.5, 0.5, 1.0);
highp vec4 material_specular = vec4 (0.5, 0.5, 0.5, 1.0);
highp float material_shininess = 1.5;

void main (void)
{	

  highp vec3 nnormal = normalize (the_normal.xyz);
  highp vec3 nlightVec = normalize (the_light.xyz);
  highp vec3 nviewVec = normalize (the_view.xyz);
  highp vec3 reflecVec = reflect (-nlightVec, nnormal);

  highp float specValue = clamp (dot(reflecVec, nviewVec), 0.0, 1.0);
  specValue = pow(specValue, material_shininess);

  highp vec4 diffuse = diffuse_light * material_diffuse * diffuseAmount * max(dot(nlightVec, nnormal), 0.0);
  //highp vec4 diffuse = vec4 (1.0, 1.0, 1.0, 1.0);
  //highp vec4 diffuse = vec4 (0.5, 0.5, 0.5, 1.0);



  gl_FragColor = ambient_light * material_ambient +
                 (diffuse * the_color) +
                 vec4 ((specular_light * material_specular * specAmount * specValue).rgb, 1.0);


}
