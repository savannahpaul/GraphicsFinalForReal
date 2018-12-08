/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// attributes
in vec2 texCoord;
in vec4 vertCoord;
in vec4 vertNormal;

// passed through to fragment shader
out vec4 fragCoord;
out vec4 fragNormal;
out vec2 fragTexCoord;

// modelview matrix
uniform mat4 mvMatrix;

void main() {
	
    // some stuff to pass through
	fragCoord = vertCoord;
	fragNormal = vertNormal;
    fragTexCoord = texCoord;	

	// transformations!
	gl_Position = mvMatrix * vertCoord;	
}