#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define FRAG_COLOR	0

precision highp float;
precision highp int;

uniform mat4 MVP;
uniform mat4 NormalMatrix;
uniform vec2 TextureRepetition;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;


out block
{
	vec2 TexCoord; 
	vec3 Position;
	vec3 Normal;
} Out;

void main()
{	
	
	vec3 pos = Position;

	Out.TexCoord = TexCoord * TextureRepetition;
	Out.Position = pos;
	Out.Normal = vec3(NormalMatrix * vec4(Normal, 0));

	gl_Position = MVP * vec4(Position,1);
	
}
