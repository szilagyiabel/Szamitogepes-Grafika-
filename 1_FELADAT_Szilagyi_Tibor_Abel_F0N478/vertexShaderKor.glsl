#version 430 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1
  
out vec3 myColor;
uniform float offsetX;
uniform float offsetY;


void main(void)
{
	gl_Position = vec4(aPos.x + offsetX, aPos.y + offsetY, aPos.z, 1.0);
	myColor = aColor;
} 