#version 330 core
in vec2 position;
out vec4 color;
uniform float time;


void main() {
	float c = sin( position.x * cos(time * 0.1) * 10.0 ) + cos( position.y * cos( time * 0.1) * 10.0 );
	color = vec4( vec3( c, c * 0.5, sin( c + time / 3.0 ) * 0.75 ), 1.0 );
}