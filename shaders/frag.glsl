#version 410 core

struct PointLight{
  vec3 lightColor;
  vec3 lightPos;
  float ambientIntensity;

  float specularStrength;

  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Sun {
  vec3 color;
  vec3 direction;
  // TODO: could add some specular/ambient field here too
};

in vec3 v_vertexNormals;
in vec3 v_position;
in vec2 v_TextureCoordinate;

in vec3 viewPosition;

const int lightCount = 1;
uniform PointLight u_pointLights[lightCount];

const int sunCount = 1;
uniform Sun u_suns[sunCount];
uniform sampler2D u_DiffuseTexture;

out vec4 color;

// useful derivations
//int specularFalloff = 32;
vec3 normal = normalize(v_vertexNormals);

vec3 calculateAmbient(PointLight light) {
  return light.ambientIntensity * light.lightColor;
}

vec3 calculateDiffuse(vec3 lightDirection, vec3 lightColor) {
  //vec3 lightDirection = normalize(light.lightPos - v_position);
  float alignment = dot(normal, lightDirection);
  return max(0, alignment) * lightColor;
}

vec3 calculateSpecular(PointLight light) {
  vec3 lightDirection = normalize(light.lightPos - v_position);
  vec3 reflectionDirection = reflect(-lightDirection, normal);
  vec3 viewDirection = normalize(viewPosition - v_position);
  float alignment = dot(viewDirection, reflectionDirection);
  float attenuation = pow(max(0, alignment), 32);
  return attenuation * light.specularStrength * light.lightColor;
}

// Entry point of program
void main()
{
  vec3 diffuseColor = texture(u_DiffuseTexture, v_TextureCoordinate).rgb;
  vec3 ambient = vec3(0.0f, 0.0f, 0.0f);
  vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
  vec3 specular = vec3(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < lightCount; i += 1) {
    PointLight light = u_pointLights[i];
    vec3 lightDirection = normalize(light.lightPos - v_position);
    ambient += calculateAmbient(light);
    diffuse += calculateDiffuse(lightDirection, light.lightColor);
    specular += calculateSpecular(light);
    //ambient += vec3(0.5, 0.5, 0.5) * (int(distance(light.lightPos, v_position)) % 20) / 20.0;
  }
  for (int i = 0; i < sunCount; i += 1) {
    Sun sun = u_suns[i];
    //diffuse += calculateDiffuse(normalize(vec3(1, 4, 2)), vec3(2.0, 2.0, 2.0));
    diffuse += calculateDiffuse(sun.direction, sun.color);
    ambient += sun.color * 0.4f;
  }
  
  vec3 total = ambient + diffuse + specular;
  if (diffuseColor == vec3(1, 1, 1)) {
    discard;
  }
  color = vec4(total.r * diffuseColor.r, total.g * diffuseColor.g, total.b * diffuseColor.b, 1.0f);
	//color = vec4(v_vertexNormals.r,v_vertexNormals.g, v_vertexNormals.b, 1.0f);
}
