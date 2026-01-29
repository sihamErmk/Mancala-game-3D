#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture1;
uniform bool useTexture;

// Nouveaux booléens pour l'interface
uniform bool isText;
uniform bool isCircle;

void main()
{
    // 1. DESSIN DU CERCLE DE FOND (Semi-transparent)
    if (isCircle) {
        vec4 texColor = texture(texture1, TexCoords);
        // La texture contient l'alpha. On applique une couleur marron très foncé.
        FragColor = vec4(0.2, 0.1, 0.05, texColor.a);
        return;
    }

    // 2. DESSIN DU TEXTE (Chiffres Blancs)
    if (isText) {
        vec4 sampled = texture(texture1, TexCoords);
        // Si le pixel est noir, on le jette (transparence stricte)
        if (sampled.r < 0.1 && sampled.g < 0.1 && sampled.b < 0.1)
            discard;

        // Sinon, le texte est blanc pur
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    // 3. RENDU 3D NORMAL (Bois, Graines...)
    float ambientStrength = 0.45;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec4 baseColor = vec4(objectColor, 1.0);
    if (useTexture) {
        vec4 texColor = texture(texture1, TexCoords);
        baseColor = mix(baseColor, texColor, 0.6);
    }

    vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
    FragColor = vec4(result, 1.0);
}
