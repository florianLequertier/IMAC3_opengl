#version 410 core

#define FRAG_COLOR	0

precision highp int;

layout(location = FRAG_COLOR, index = 0) out vec3 FragColor;

void main()
{
	FragColor = vec3(1,0,0);
}
