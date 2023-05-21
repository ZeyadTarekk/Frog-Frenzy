#version 330 core

out vec4 frag_color;
in vec2 tex_coord;


//uniform sampler2D hdrBuffer;
uniform sampler2D tex;
uniform float distortionAmount = 0.1;

void main()
{             
    // Calculate the distortion offset based on the texture coordinates
    vec2 distortionOffset = vec2(
        sin(tex_coord.y* 10.0) * distortionAmount,
        cos(tex_coord.x* 10.0) * distortionAmount  
    );

    // Sample the color texture with distorted texture coordinates
    vec4 distortedColor = texture(tex, tex_coord + distortionOffset);

    // Output the distorted color
    frag_color = distortedColor;
}