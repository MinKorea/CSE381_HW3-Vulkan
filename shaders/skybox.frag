#version 450

layout(location = 0) in vec3 frag_uvw;

layout(set = 0, binding = 0) uniform samplerCube background_sampler; 

layout(location = 0) out vec4 out_color;

void main() {
    vec3 env_color = texture(background_sampler, frag_uvw).rgb;
    env_color = env_color / (env_color + vec3(1.0));
    env_color = pow(env_color, vec3(1.0 / 2.2));
    out_color = vec4(env_color, 1.0);
}