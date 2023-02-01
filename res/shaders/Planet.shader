#shader vertex
#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 FragPos;

uniform mat4 u_MVP;
uniform mat4 model;

void main()
{
  gl_Position = u_MVP * position;
  FragPos = vec3(model * position);
  v_TexCoord = texCoord;
  v_Normal = mat3(transpose(inverse(model))) * normal;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 FragPos;

uniform sampler2D u_Texture;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{
  float ambientStrength = 0.1;
  vec3 ambientResult = ambientStrength * lightColor;
  vec4 ambient = vec4(ambientResult, 1.0);

  vec3 norm = normalize(v_Normal);
  vec3 lightDir = normalize(FragPos - lightPos);

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuseResult = diff * lightColor;
  vec4 diffuse = vec4(diffuseResult, 1.0);

  vec4 texColor = texture(u_Texture, v_TexCoord);

  color = (ambient + diffuse) * texColor;
}
