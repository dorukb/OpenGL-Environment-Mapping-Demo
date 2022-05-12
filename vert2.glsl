#version 460 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec4 fragWorldPos;
out vec3 fragWorldNor;
out vec3 TexCoords;

mat4 view;

void main(void)
{
	// Compute the world coordinates of the vertex and its normal.
	// These coordinates will be interpolated during the rasterization
	// stage and the fragment shader will receive the interpolated
	// coordinates.

	fragWorldPos = modelingMatrix * vec4(inVertex, 1);
	fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

	TexCoords = inVertex;

	// remove translation part
	view = viewingMatrix;
	view[0][3] = view[1][3] = view[2][3] = view[3][0] =view[3][1] = view[3][2] = 0.0f;

	gl_Position = projectionMatrix * view * vec4(inVertex, 1);
}

