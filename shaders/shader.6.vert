#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat3 nm;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 n;
layout(location = 2) out vec3 v;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    fragTexCoord = texCoord;

	n            = normalize ( ubo.nm * normal );
	v            = normalize ( vec3 ( ubo.view * ubo.model * vec4(pos, 1.0) ) );
}
