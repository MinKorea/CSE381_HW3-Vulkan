#version 450

layout(binding = 0) uniform UBO {
    mat4 proj_view;
} ubo;

layout(push_constant) uniform PCO {
    mat4 model;
} pco;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;

void main() {
    gl_Position = ubo.proj_view * pco.model * vec4(position, 1.0);
    out_position = vec3(pco.model * vec4(position, 1.0));
    out_normal = transpose(inverse(mat3(pco.model))) * normal;
    out_uv = uv; 
}