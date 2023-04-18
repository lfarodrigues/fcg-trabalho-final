#version 330

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

in vec3 a_Vertex;

out vec4 v_VertexPos;

void main()
{
    v_VertexPos = u_View * u_Model * vec4(a_Vertex, 1.0);

    gl_Position = u_Projection * v_VertexPos;
}
