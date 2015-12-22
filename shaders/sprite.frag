#version 330 core
in vec2 TexCoords;
in vec4 gl_FragCoord;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform float clipY;

void main()
{  
    if (gl_FragCoord.y < clipY)
    {
        color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
    }
    else
    {
        discard;
    }
}