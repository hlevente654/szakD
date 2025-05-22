#version 450
layout(location = 0) in vec3 inPosition;    // csak pozíció
layout(set = 0, binding = 0) uniform LightVP {
    mat4 lightViewProj;                     // fény nézőpont mátrixa
} ubo;
void main() {
    // A világpozíció transzformálása a fény view-projection mátrixával:
    gl_Position = ubo.lightViewProj * vec4(inPosition, 1.0);
}
