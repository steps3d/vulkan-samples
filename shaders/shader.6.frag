#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube image;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 n;
layout(location = 2) in vec3 v;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = 	texture ( image, reflect ( normalize(v), normalize(n) ) );
//	outColor = texture ( image,n );
;
}