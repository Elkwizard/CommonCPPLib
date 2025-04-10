#version 460 core

uniform vec2 resolution;

in vec2 vertexPosition;
in mat3x2 instanceTransform;
in vec4 instanceColor; 
in float instanceFlags;
in float instanceLineWidth;

out vec2 uv;
out vec4 color;
out float fflags;
out float lineWidth;
out vec2 px;

void main() {
	vec2 pos = (instanceTransform * vec3(vertexPosition, 1.0)).xy;
	pos = pos / resolution * 2.0 - 1.0;
	pos *= vec2(1, -1);
	
	uv = vertexPosition;
	color = instanceColor;
	fflags = instanceFlags;
	lineWidth = instanceLineWidth;
	px = 1.0 / vec2(
		length(instanceTransform[0]),
		length(instanceTransform[1])
	);
	gl_Position = vec4(pos, 0, 1);
}