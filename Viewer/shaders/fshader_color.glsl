#version 330 core

struct Material
{
	vec3 color;
	float KA;
	float KD;
	float KS;
	float shine;
};


uniform vec3 lightPos[5];
uniform vec3 lightCol[5];
uniform vec3 eye;

uniform Material material;
// Inputs from vertex shader (after interpolation was applied)
in vec3 fragPos;
in vec3 fragNormal;

out vec4 frag_color;

void main()
{
	vec3 IA = material.color * material.KA;
	vec3 ID = vec3(0.0f);
	vec3 IS = vec3(0.0f);
	for (int i=0; i<5; i++)
	{
		// diffuse 
		vec3 norm = normalize(fragNormal);
		vec3 lightDir = normalize(lightPos[i] - fragPos);
		float diff = max(dot(norm, lightDir), 0.0) * material.KD;
		ID = ID + diff * lightCol[i];

		// specular
		vec3 viewDir = normalize(eye - fragPos);
		vec3 reflectDir = reflect(-lightDir, norm);  
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16) * material.KS;
		IS = IS + spec * lightCol[i];
	}
	vec3 I = IA + (ID + IS);

	frag_color = vec4(I,1.0f);
	
}
