#version 430

out vec4 color;

uniform float szinez;

void main(void)
{
	float radius = 0.5;
    float dist = distance(gl_PointCoord, vec2(0.5, 0.5));

	if(szinez == 0) {
		color = vec4(0.0, 0.5, 0.0, 1.0);
	}
	else if(szinez == 1) {
		color = vec4(0.5, 0.5, 1.0, 0.0);
	}
	else if(szinez == 2) {
		if (dist < radius) {
			color = vec4(1.0, 0.5, 0.0, 1.0);
		} else {
			discard;
		}
	}
}