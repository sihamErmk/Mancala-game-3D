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
uniform bool useTexture; // Pour activer/désactiver la texture selon l'objet

void main()
{
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
  
    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    // Base Color (Mix texture if enabled)
    vec4 baseColor = vec4(objectColor, 1.0);
    if (useTexture) {
        baseColor = mix(baseColor, texture(texture1, TexCoords), 0.5); // Mélange 50% couleur / 50% texture
    }

    vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
    FragColor = vec4(result, 1.0);
}