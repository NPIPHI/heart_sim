#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform constants{
    mat4 mvp;
};

struct NodeState{
    float activation;
};

layout(std430, binding = 0) buffer node_state_arr{
    NodeState node_states[];
};

void main() {
    gl_Position = mvp * in_position;
    gl_PointSize = 2.f;

    float ambient = 0.5;
    float direct = 0.5;
    vec3 src_light = vec3(0.577, 0.577, 0.577);

    float light = dot(in_normal.xyz, src_light) * direct + ambient;
    frag_color = vec3(node_states[gl_VertexIndex].activation, 0, 0.1) * light;
}