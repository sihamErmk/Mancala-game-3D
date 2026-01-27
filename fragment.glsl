#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform bool lightEnabled;
uniform float materialShininess;
uniform sampler2D texture1;
uniform bool useTexture;

void main()
{
    vec3 result;

    if (!lightEnabled) {
        result = objectColor * 0.1;
    }
    else {
        // --- 1. AMBIENT ---
        float ambientStrength = 0.25; // Un peu plus lumineux pour bien voir le fond
        vec3 ambient = ambientStrength * lightColor;

        // --- 2. NORMALE MAGIQUE (EFFET CREUX) ---
        // Si on regarde la face arrière (l'intérieur du bol), on inverse la normale
        // pour que la lumière rebondisse correctement.
        vec3 norm = normalize(Normal);
        if (!gl_FrontFacing) {
            norm = -norm;
        }

        // --- 3. DIFFUSE ---
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // --- 4. SPECULAR ---
        float specularStrength = 0.6;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        vec3 specular = specularStrength * spec * lightColor;

        // --- 5. TEXTURE ---
        vec4 baseColor = vec4(objectColor, 1.0);
        if (useTexture) {
            vec4 texColor = texture(texture1, TexCoords);
            baseColor = mix(baseColor, texColor, 0.75);
        }

        result = (ambient + diffuse + specular) * baseColor.rgb;
    }

    FragColor = vec4(result, 1.0);
}
