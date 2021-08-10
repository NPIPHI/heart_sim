#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 pos;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(frag_color, 1) * dot(pos, vec3(0, 0, 1));
}