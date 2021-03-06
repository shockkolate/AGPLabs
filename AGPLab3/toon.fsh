#version 150
precision highp float;

uniform mat4 u_matProjection;
uniform mat4 u_matModelView;
uniform mat4 u_matObjectModelView;

smooth in vec4 v_vVertex;
smooth in vec3 v_vNormal;
smooth in vec4 v_vEyeCameraPosition;

out vec4 o_vColor;

void main(void)
{
    // test values
    vec3 vAmbient = vec3(0.05);
    vec3 vDiffuse = vec3(1.0, 0.4, 0.0);
    vec3 vSpecular = vec3(1.0, 0.8, 0.1);
    //vec4 vLightPos = vec4(3.0, 5.0, 3.0, 1.0);
    vec4 vLightPos = vec4(0.0);

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

    float fNDotL = dot(vNormal, vLightDir);
    float fNDotHV = max(0.0, dot(vNormal, vHalfVector));

    // if light hits surface
    if(fNDotL > 0.0)
    {
        float fDivisor = 1.0 +
                         fDistance * 0.0 +
                         pow(fDistance, 2.0) * 0.0 +
                         pow(fDistance, 3.0) * 0.0001 +
                         pow(fDistance, 4.0) * 0.0003;
        float fAttenuation = 1.0 / fDivisor;
        vFinalDiffuse = fAttenuation * vDiffuse * fNDotL;

        vFinalSpecular = vSpecular * pow(fNDotHV, 128.0);
    }

    // diffuse toon shading
    vColor += round(vFinalDiffuse * 19.0) / 19.0;

    // toon scan blocking
    vColor *= round((int(u_matProjection * gl_FragCoord.x) % 37) / 23.0);
    vColor *= round((int(u_matProjection * gl_FragCoord.y) % 37) / 23.0);
    vColor /= 2.0;

    vColor.g *= 1.0 - fNDotHV * -(u_matModelView * vec4(vAux, 1.0)).y;
    vColor.b *= 1.0 - fNDotHV * -(u_matModelView * vec4(vAux, 1.0)).y * 0;

    if(vColor.g < 0.2) vColor = vec3(1.0, 0.3, 0.9);

    // specular toon shading
    vColor += round(vFinalSpecular * 7.0) / 7.0;

    o_vColor = vec4(vColor, 1.0);
}
