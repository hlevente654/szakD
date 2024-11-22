#version 450

layout(location = 0) in vec3 fragCol;   // Vertex szín
layout(location = 1) in vec2 fragTex;   // Textúra koordináták
layout(location = 2) in vec3 fragNorm;  // Világ térbeli normálok
layout(location = 3) in vec3 fragPos;   // Világ térbeli pozíció
layout(location = 4) in vec3 viewPos;   // Kamera pozíció világ térben

layout(set = 1, binding = 0) uniform sampler2D textureSampler;  // Textúra mintázó

layout(location = 0) out vec4 outColour;  // Kimeneti szín

// Spotlight struktúra deklarálása kívül
struct Spotlight {
    vec3 lightDirection; // A fény iránya
    float diffuseStr;    // Diffúz fény erőssége
    vec3 lightColor;     // A fény színe
    float specularStr;   // Speculáris fény erőssége
    float shininess;     // Speculáris élesség

    float innerCutOff;   // A fény kúp szögének belső határa
    float outerCutOff;   // A fény kúp szögének külső határa

    vec3 lightPosition;  // ÚJ: A fény pozíciója
};

// UboLighting struktúra, amely tartalmazza a Spotlight-ot
layout(set = 0, binding = 1) uniform UboLighting {
    vec3 ambiantLightColor; // Ambient fény színe
    float ambiantStr;       // Ambient erősség
    Spotlight spotlight;     // A spotlight struktúra használata
} ubo;

void main() {
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(ubo.spotlight.lightPosition - fragPos); // Spotlight irány
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Ambient lighting
    vec3 ambient = ubo.ambiantStr * ubo.ambiantLightColor;

    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = ubo.spotlight.diffuseStr * diff * ubo.spotlight.lightColor;

    // Specular lighting
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ubo.spotlight.shininess);
    vec3 specular = ubo.spotlight.specularStr * spec * ubo.spotlight.lightColor;

    // Spotlight cutoff
    float theta = dot(lightDir, normalize(-ubo.spotlight.lightDirection));
    float epsilon = max(ubo.spotlight.innerCutOff - ubo.spotlight.outerCutOff, 0.001); // Biztonsági tartomány
    float spotlightIntensity = clamp((theta - ubo.spotlight.outerCutOff) / epsilon, 0.0, 1.0);

    // Attenuation (distance-based)
    float distance = length(ubo.spotlight.lightPosition - fragPos);
    //float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance)); // Példa paraméterek
	float attenuation = 1.0;


    // Combined lighting
    float intensity = spotlightIntensity * attenuation;
    vec4 texColor = texture(textureSampler, fragTex);
    vec3 lighting = ambient + intensity * (diffuse + specular);
    vec3 finalColor = texColor.rgb * lighting;

    outColour = vec4(finalColor, texColor.a);
}