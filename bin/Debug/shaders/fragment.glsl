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
uniform bool isText; // NOUVEAU : Pour savoir si on dessine du texte

void main()
{
    // Si c'est du texte, on gère la transparence
    if (isText) {
        vec4 sampled = texture(texture1, TexCoords);
        // Si le pixel est noir (ou presque), on ne l'affiche pas (transparence)
        if (sampled.r < 0.1 && sampled.g < 0.1 && sampled.b < 0.1)
            discard;
        
        // Sinon on affiche le texte en Blanc (ou la couleur demandée)
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); 
        return; 
    }

    // --- Rendu normal (Mancala) ---
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