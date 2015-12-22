#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
// Parameters for texture atlas packed into a vec4(x:width, y:height, z:column, w:row)
uniform vec4 atlasParams;

void main()
{
    TexCoords.x = (atlasParams.z * atlasParams.x) + (atlasParams.x * vertex.z);
    TexCoords.y = (atlasParams.w * atlasParams.y) + (atlasParams.y * vertex.w);    
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}