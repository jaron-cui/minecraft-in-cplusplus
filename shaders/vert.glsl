#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexNormals;
layout(location=2) in vec2 textureCoordinate;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection; // We'll use a perspective projection

uniform vec3 u_viewPosition;

// Pass vertex colors into the fragment shader
out vec3 v_vertexNormals;
out vec3 v_position;
out vec2 v_TextureCoordinate;

out vec3 viewPosition;

void main()
{
  v_vertexNormals= vertexNormals;
  v_position = position;
  v_TextureCoordinate = textureCoordinate;

  viewPosition = u_viewPosition;

  vec4 newPosition = u_Projection * u_ViewMatrix * u_ModelMatrix * vec4(position,1.0f);
                                                                    // Don't forget 'w'
	gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}


