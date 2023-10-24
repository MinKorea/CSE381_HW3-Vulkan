#version 450

const int NUM_LIGHTS = 4;
const vec3 LIGHT_COLOR = vec3(1.0f, 1.0f, 1.0f);

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(push_constant) uniform PCO {
    layout(offset = 64) vec3 cam_pos;
    layout(offset = 80) int is_colliding;
} pco;

layout(set = 0, binding = 0) uniform UBO {
    layout(offset = 64) vec4 lights[NUM_LIGHTS];
} ubo;

layout(set = 1, binding = 0) uniform sampler2D color_map;

layout(location = 0) out vec4 out_color;

void main() {
    // read color from texure
    vec3 color = texture(color_map, in_uv).rgb;
    if (pco.is_colliding > 0) {
        color = vec3(1.0f, 0.0f, 0.0f);
    }

    vec3 N = normalize(in_normal);
    vec3 V = normalize(pco.cam_pos.xyz - in_position);
    vec3 light_contribtion = 0.15 * color; // ambient
    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 L = normalize(ubo.lights[i].xyz - in_position);
        vec3 diffuse = max(dot(L, N), 0.0) * LIGHT_COLOR;

        vec3 H = normalize(L + V);
        vec3 specular = pow(max(dot(N, H), 0.0), 64) * LIGHT_COLOR;

        float max_dist = 2.0;
        float distance = min(length(ubo.lights[i].xyz - in_position), max_dist);
        float attenuation = 1.0 / (distance * distance);

        light_contribtion += (diffuse + specular) * attenuation;
    }

    color = color * light_contribtion;
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    out_color = vec4(color, 1.0f);
}