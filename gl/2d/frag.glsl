#version 460 core

#include "masks.glsl"

#define HAS(flag) (flags & flag) != 0
#define GET(field) ((flags >> field##_OFFSET) & field##_MASK)

in vec2 uv;
in vec4 color;
in float fflags;
in float lineWidth;
in vec2 px;

out vec4 finalColor;

void main() {
	int flags = int(fflags);
	int shape = GET(SHAPE);
	int style = GET(STYLE);

	float aa = 1.0;

	if (style == STROKE) {
		vec2 dimensions = 1.0 / px;
		vec2 pos = abs(uv - 0.5) * dimensions;
		if (shape == RECTANGLE) {
			vec2 fromEdge = dimensions * 0.5 - pos;
			float dist = min(fromEdge.x, fromEdge.y);
			if (dist > lineWidth + 0.5) discard;
			aa *= smoothstep(lineWidth + 0.5, lineWidth - 0.5, dist);
		} else if (shape == CIRCLE) {
			float len = length(pos);
			if (
				len > dimensions.x * 0.5 ||
				len < dimensions.x * 0.5 - lineWidth
			) discard;
		}
	} else if (style == FILL) {
		if (shape == RECTANGLE) {

		} else if (shape == CIRCLE) {
			if (length(uv - 0.5) > 0.5) discard;
		} else if (shape == TRIANGLE) {
			if (uv.x + uv.y > 1.0) discard;
		}
	}

	finalColor = color;
	finalColor.rgb *= finalColor.a;
	finalColor *= aa;
}