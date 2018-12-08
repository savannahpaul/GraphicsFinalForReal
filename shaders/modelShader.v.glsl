/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #B
in vec3 vPosition;
in vec3 vMormal;
// TODO #F1
out vec3 theColor;
// TODO #A
uniform mat4 mvpMatrix;
uniform vec3 camPos;
uniform float time;
void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
	
	gl_Position = mvpMatrix * vec4(vPosition,1);
		
	//color calculations
	float lightIntensity = 1;
	float reflectivity =.75;
    vec3 lightPos = vec3(10,10,10);
	vec3 lightDir = normalize( lightPos - vPosition);
	vec3 viewDir = normalize(camPos - vPosition);
	vec3 reflectDir = reflect(-lightDir, vMormal);
	
	float ambient = .25; //just setting ambient lighting as a constant 
	float diffuse = lightIntensity * reflectivity * max( dot(vMormal,lightDir), 0.0);	
	float specular = lightIntensity * reflectivity * pow(max(dot(viewDir, reflectDir),0.0), 32);
	
	float light = ambient + diffuse + specular;
	
    // TODO #F2
	theColor = vec3(light,light,light);
}
