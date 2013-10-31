#version 150
precision highp float;

uniform sampler2D u_texture;
uniform sampler2D u_nmap;
uniform sampler2D u_hmap;

uniform mat4 u_matProjection;
uniform mat4 u_matModelView;
uniform mat4 u_matObjectModelView;

smooth in vec3 v_vVertex;
smooth in vec3 v_vNormal;
smooth in vec3 v_vTangent;
smooth in vec3 v_vBitangent;
smooth in vec2 v_vTexCoord;
smooth in vec4 v_vEyeCameraPosition;
smooth in mat3 matTBN;

out vec4 o_vColor;

void main(void)
{
    // test values

    vec4 vLightPos = vec4(0.0);
    vec3 vAmbient = vec3(0.05);
    vec3 vDiffuse = vec3(0.0, 0.8, 1.0);
    vec3 vSpecular = vec3(0.8, 1.0, 1.0);
    float fShininess = 64.0;

    // texcoord
    vec2 vTexCoord = v_vTexCoord.st;

    // height mapping
    /*float height = texture(u_hmap, vTexCoord).r;
    float fHSB = height * 0.04 + 0.02;
    vTexCoord += normalize(v_vEyeCameraPosition).xy * fHSB;*/

    vec3 vColor = vAmbient;
    vec3 vFinalDiffuse;
    vec3 vFinalSpecular;

    vec3 vAux = vec3(u_matModelView * vLightPos - v_vEyeCameraPosition);
    vec3 vLightDir = normalize(vAux);
    float fDistance = length(vAux);

    // halfway vector between light direction and object
    vec3 vHalfVector = normalize(vLightDir - normalize(vec3(v_vEyeCameraPosition)));

    // normalize the interpolated normal
    vec3 vNormal = normalize(v_vNormal);

    // make use of normal map
    /*vec3 vNMColor = texture(u_nmap, vTexCoord).xyz;
    vNormal = vNMColor * 2.0 - 1.0;*/

    float fNDotL = dot(vNormal, vLightDir);
    float fNDotHV = max(0.0, dot(vNormal, vHalfVector));

    // if light hits surface
    if(fNDotL > 0.0)
    {
        float fDivisor = 1.0 +
                         fDistance * 0.05 +
                         pow(fDistance, 2.0) * 0.01 +
                         pow(fDistance, 3.0) * 0.002 +
                         pow(fDistance, 4.0) * 0.002;
        float fAttenuation = 1.0 / fDivisor;
        vFinalDiffuse = fAttenuation * vDiffuse * fNDotL;

        vFinalSpecular = fAttenuation * vSpecular * pow(fNDotHV, fShininess);
    }

    vColor += vFinalDiffuse + vFinalSpecular;

    o_vColor = vec4(vColor, 1.0);
    o_vColor = texture(u_texture, vTexCoord) * o_vColor * 1.4;
}
