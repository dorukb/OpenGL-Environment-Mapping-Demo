#version 460 core

uniform vec3 eyePos;
uniform samplerCube cubeSampler;

in vec3 TexCoords;
in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

vec3 dir;
void main(void)
{
	dir = normalize(vec3(fragWorldPos.xyz) - eyePos);
	fragColor = texture(cubeSampler, normalize(reflect(dir,normalize(fragWorldNor))));

		//fragColor = texture(cubeSampler, dir);

}
