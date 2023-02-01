#shader vertex
#version 330 core
layout(location = 0) in vec4 position;

uniform mat4 u_MVP;

void main()
{
  gl_Position = u_MVP * position;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
  color = vec4(0.753, 0.753, 0.753, 1.0);
}
