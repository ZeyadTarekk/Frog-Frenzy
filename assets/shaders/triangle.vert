#version 330

// This vertex shader should be used to render a triangle whose normalized device coordinates are:
// (-0.5, -0.5, 0.0), ( 0.5, -0.5, 0.0), ( 0.0,  0.5, 0.0)
// And it also should send the vertex color as a varying to the fragment shader where the colors are (in order):
// (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)

#define FIRST_VERTEX_COLOR 0
#define SECOND_VERTEX_COLOR 1

out Varyings {
    vec3 color;
} vs_out;

// Currently, the triangle is always in the same position, but we don't want that.
// So two uniforms should be added: translation (vec2) and scale (vec2).
// Each vertex "v" should be transformed to be "scale * v + translation".
// The default value for "translation" is (0.0, 0.0) and for "scale" is (1.0, 1.0).

// define the uniforms for translation and scale
uniform vec2 translation = vec2(0.0, 0.0);
uniform vec2 scale = vec2(1.0, 1.0);

//* DONE: (Req 1) Finish this shader

void main() {
    // Define the vertices of the triangle in normalized device coordinates
    vec3 vertices[3] = vec3[3](vec3(-0.5, -0.5, 0.0), vec3(0.5, -0.5, 0.0), vec3(0.0, 0.5, 0.0));

    // define the positions of the triangle in normalized device coordinates
    vec3 position = vec3(scale * vertices[gl_VertexID].xy + translation, 1.0);

    // assign color to each vertex in the triangle
    if(gl_VertexID == FIRST_VERTEX_COLOR) {
        vs_out.color = vec3(1.0, 0.0, 0.0); // set to red

    } else if(gl_VertexID == SECOND_VERTEX_COLOR) {
        vs_out.color = vec3(0.0, 1.0, 0.0); // set to green

    } else {
        vs_out.color = vec3(0.0, 0.0, 1.0); // set to blue
    }

    // Set the position of the current vertex
    gl_Position = vec4(position, 1.0);

    // !  fourth component is typically set to 1.0, as it ensures that the vector is not affected by perspective division 
    // !  during the transformation to clip space.
}
