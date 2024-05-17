#version 430 core
out vec4 Color; 
in vec3 myColor; 
  
void main(void)
{
    Color = vec4(myColor, 0.0);
}