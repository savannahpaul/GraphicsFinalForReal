/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #B
in vec3 vPosition;
// TODO #F1
out vec3 theColor;
// TODO #A
uniform mat4 mvpMatrix;
// TODO #G1
uniform float time;
vec3 newVertex;
void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    // TODO #G2
	if(vPosition[0] > 0 && vPosition[1] > 0 && vPosition[2] < 0){
		newVertex = vPosition + 1.2*((sin(time*2)+1)/2) - .2;
		//newVertex = vPosition + 1.2*sin(time);
	}
	else if(vPosition[0] < 0 && vPosition[1] < 0 && vPosition[2] < 0){
		newVertex = vPosition - 1.2*((sin(time*2)+1)/2) - .2;
	}
	else if (vPosition[0] < 0 && vPosition[1] > 0 && vPosition[2] > 0){
		newVertex = vPosition + 1.2*((sin(time)+1)/2) - .2;
	}
	else if (vPosition[0] > 0 && vPosition[1] < 0 && vPosition[2] > 0){
		newVertex = vPosition - 1.2*((sin(time)+1)/2) - .2;
	}
	else {
		newVertex = vPosition + 1.2*((sin(time*5)+1)/2) - .2;
	}
    // TODO #C
    gl_Position = mvpMatrix * vec4(newVertex, 1);
    // TODO #F2
	theColor = glm::vec3(1 + sin(time), 1 + sin(time), 1 + cos(time));
}
