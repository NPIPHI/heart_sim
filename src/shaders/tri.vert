#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

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
    gl_Position = mvp * vec4(in_position, 1);
    gl_PointSize = 2.f;
    frag_color = vec3(node_states[gl_VertexIndex].activation, 0, 0.1);
}