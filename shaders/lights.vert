#version 450

layout(binding = 0) uniform UBO {
    mat4 proj_view;
} ubo;

layout(push_constant) uniform PCO {
    mat4 model; 
} pco;

layout(location = 0) in vec3 position;

void main() {
    gl_Position = ubo.proj_view * pco.model * vec4(position, 1.0);
}