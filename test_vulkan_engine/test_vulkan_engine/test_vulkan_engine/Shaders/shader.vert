#version 450

layout(location = 0) in vec3 pos;       // Pozíció
layout(location = 1) in vec3 col;       // Szín
layout(location = 2) in vec2 tex;       // Textúra koordináták
layout(location = 3) in vec3 norm;      // Normálvektorok

layout(set = 0, binding = 0) uniform UboViewProjection {
    mat4 projection;
    mat4 view;
} uboViewProjection;

layout(push_constant) uniform PushModel {
    mat4 model;
} pushModel;

layout(location = 0) out vec3 fragCol;   // Szín továbbítása
layout(location = 1) out vec2 fragTex;   // Textúra koordináták továbbítása
layout(location = 2) out vec3 fragNorm;  // Normál továbbítása világ térben
layout(location = 3) out vec3 fragPos;   // Fragment világ térbeli pozíciója
layout(location = 4) out vec3 viewPos;   // Kamera pozíciója világ térben

void main() {
    mat4 modelMatrix = pushModel.model;
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix))); // Normál transzformálása

    gl_Position = uboViewProjection.projection * uboViewProjection.view * modelMatrix * vec4(pos, 1.0);
    
    fragCol = col;
    fragTex = tex;
    fragNorm = normalize(normalMatrix * norm);  // Normalizált normál átvitele világ térbe
    fragPos = vec3(modelMatrix * vec4(pos, 1.0)); // Fragment pozíció világ térben

    // Kamera pozíció helyes kiszámítása
    viewPos = vec3(inverse(uboViewProjection.view) * vec4(0, 0, 0, 1));
}