#version 450

layout(location = 0) in vec3 fragCol;  // Bejövő szín (alap szín a vertex shader-ből)
layout(location = 1) in vec2 fragTex;  // Bejövő textúra koordináták

layout(set = 1, binding = 0) uniform sampler2D textureSampler;  // Textúra mintázó

layout(location = 0) out vec4 outColour;  // Végső kimeneti szín

layout(set = 0, binding = 1) uniform UboAmbLighting {
    vec3 lightColor;
    float ambiantStr;
} uboAmbLighting;

void main() {
    // Textúra szín lekérése
    vec4 texColor = texture(textureSampler, fragTex);

    // Ambient fény kiszámítása: erősség szorozva a fény színével
    vec3 ambient = uboAmbLighting.ambiantStr * uboAmbLighting.lightColor;
	// vec3 ambient = 0.5f * vec3(1.0f, 1.0f, 1.0f);

    // A textúra színe és az ambient fény keverése
    vec3 finalColor = texColor.rgb * ambient;  // Sötétítés a textúra színén az ambient fénynek megfelelően

    // Kimeneti szín
    outColour = vec4(finalColor, texColor.a);
}
