#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(push_constant) uniform PCO {
    mat4 proj;
    mat4 view;
} pco;

layout(location = 0) out vec3 frag_uvw;

void main() {
    gl_Position = pco.proj * mat4(mat3(pco.view)) * vec4(position, 1.0);
    frag_uvw = position;
}