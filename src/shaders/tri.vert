#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 pos;

layout(push_constant) uniform constants{
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(in_position, 1);
    //    gl_PointSize = 1.f + clamp(frag_color.x, 0.f, 1.f)*3.f;
    gl_PointSize = 2.f;
    frag_color = in_color.rgb;
    pos = in_position;
}