/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// uniforms
uniform float matShine = 80;
uniform mat4 modelMatrix;
uniform vec3 matSpecColor = vec3(1.0, 1.0, 1.0);
uniform vec3 cameraPos;
uniform sampler2D texSampler;

// for lighting
uniform struct Light {
   vec3 position;
   vec3 intensities; // r, g, b intensities
   float attenuation;
   float ambientCoefficient;
} light;

// attributes
uniform vec4 baseColor;

// varyings
in vec4 fragCoord;
in vec4 fragNormal;
in vec2 fragTexCoord;

// final color
out vec4 fragColorOut;

void main() {

	vec3 normal = normalize(transpose(inverse(mat3(modelMatrix))) * vec3(fragNormal));
	vec3 surfacePos = vec3(modelMatrix * fragCoord);
	vec4 surfaceColor = baseColor;
	vec3 surfaceToLight = normalize(light.position - surfacePos);
	vec3 surfaceToCamera = normalize(cameraPos - surfacePos);

	// ambient
	vec3 ambient = light.ambientCoefficient * surfaceColor.rgb * light.intensities;
	
	// diffuse
	float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * light.intensities;
	
	// specular
	float specularCoefficient = 0.0;
	if (diffuseCoefficient > 0.0) pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), matShine);
	vec3 specular = specularCoefficient * matSpecColor * light.intensities;
	
	// attenuation
	float distanceToLight = length(light.position - surfacePos);
	float attenuation = 1.0/(1.0 + light.attenuation * pow(distanceToLight, sqrt(2)));
	
	// linear color
	vec3 linearColor = ambient + attenuation*(diffuse + specular);
	
    // final color
	vec3 gamma = vec3(1.0/2.2);
    // TODO #E
	// fragColorOut = vec4(1,1,1,1);
	// TODO #F4
	// fragColorOut = theColor;
	// fragColorOut = texture(texSampler, fragTexCoord);
    fragColorOut = vec4(pow(linearColor, gamma), surfaceColor.a);
}
