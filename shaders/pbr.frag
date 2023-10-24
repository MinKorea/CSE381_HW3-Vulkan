#version 450

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 frag_uvw;


layout(set = 0, binding = 0) uniform UBO {
    layout(offset = 64) vec3 cam_pos;
} ubo;

// global descriptors
layout(set = 0, binding = 1) uniform samplerCube irradiance_sampler;
layout(set = 0, binding = 2) uniform samplerCube prefilter_sampler;
layout(set = 0, binding = 3) uniform sampler2D brdf_sampler;

// material descriptors
layout(set = 1, binding = 0) uniform sampler2D color_sampler;
layout(set = 1, binding = 1) uniform sampler2D normal_sampler;
layout(set = 1, binding = 2) uniform sampler2D ao_sampler;
layout(set = 1, binding = 3) uniform sampler2D emission_sampler;
layout(set = 1, binding = 4) uniform sampler2D metallic_roughness_sampler;

#define PI 3.1415926535897932384626433832795
layout(location = 0) out vec4 out_color;

const float OCCLUSION_STRENGTH = 1.0f;
const float EMISSIVE_STRENGTH = 1.0f;

vec3 get_normal_from_map()
{
    vec3 tangentNormal = texture(normal_sampler, in_uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(frag_uvw);
    vec3 Q2  = dFdy(frag_uvw);
    vec2 st1 = dFdx(in_uv);
    vec2 st2 = dFdy(in_uv);

    vec3 N   = normalize(in_normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilter_reflection(vec3 R, float roughness) {
    const float MAX_REFLECTION_LOD = 9.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(prefilter_sampler, R, lodf).rgb;
    vec3 b = textureLod(prefilter_sampler, R, lodc).rgb;
    return mix(a, b, lod - lodf);
}

vec3 specular_contribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * texture(color_sampler, in_uv).rgb / PI + spec) * dotNL;
	}

	return color;
}


void main() {
    vec3 N = get_normal_from_map();
    vec3 V = normalize(ubo.cam_pos.rgb - frag_uvw);
    vec3 R = reflect(-V, N);

    float metallic = texture(metallic_roughness_sampler, in_uv).b; 
    float roughness = texture(metallic_roughness_sampler, in_uv).g;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, texture(color_sampler, in_uv).rgb, metallic);

    vec3 light_pos = vec3(5.0, 5.0, 5.0);
    vec3 L = normalize(light_pos - frag_uvw);
    vec3 Lo = specular_contribution(L, V, N, F0, metallic, roughness);

    vec2 brdf = texture(brdf_sampler, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 reflection = prefilter_reflection(R, roughness).rgb;
    vec3 irradiance = texture(irradiance_sampler, N).rgb;

    vec3 diffuse = irradiance * texture(color_sampler, in_uv).rgb;

    vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

    vec3 specular = reflection * (F * (brdf.x + brdf.y));

    vec3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    vec3 ambient = (kD * diffuse + specular);

    vec3 color = ambient + Lo;


    // Optional shading 
    float ao = texture(ao_sampler, in_uv).r;
    color = mix(color, color * ao, OCCLUSION_STRENGTH);

    // Optional Emissive
    vec3 emissive = texture(emission_sampler, in_uv).rgb;
    color += emissive;


    color = color / (color + vec3(1.0));

    color = pow(color, vec3(1.0 / 2.2));

    out_color = vec4(color, 1.0);
}