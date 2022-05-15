#version 460 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec3 TexCoords;
out vec4 fragWorldPos;
out vec3 fragWorldNor;

void main(void)
{
	fragWorldPos = modelingMatrix * vec4(inVertex, 1);
	fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

	gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
}

