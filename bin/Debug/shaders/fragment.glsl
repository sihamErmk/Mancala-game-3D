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
        // --- AMBIENT ---
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;
      
        // --- NORMALE MAGIQUE ---
        // C'est ici que la magie opère pour les trous !
        // Si on regarde la face arrière (l'intérieur du bol), on inverse la normale
        vec3 norm = normalize(Normal);
        if (!gl_FrontFacing) {
            norm = -norm;
        }

        // --- DIFFUSE ---
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // --- SPECULAR ---
        float specularStrength = 0.5; 
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        vec3 specular = specularStrength * spec * lightColor;  
            
        // --- TEXTURE ---
        vec4 baseColor = vec4(objectColor, 1.0);
        if (useTexture) {
            vec4 texColor = texture(texture1, TexCoords);
            baseColor = mix(baseColor, texColor, 0.7); 
        }

        result = (ambient + diffuse + specular) * baseColor.rgb;
    }

    FragColor = vec4(result, 1.0);
}