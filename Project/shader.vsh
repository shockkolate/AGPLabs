/*
 *  Copyright 2013 David Farrell
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#version 150
precision highp float;

uniform mat4 u_matProjection;
uniform mat4 u_matCamera;
uniform mat4 u_matModelView;

uniform int u_bPoints;

in vec3 a_vVertex;
in vec3 a_vNormal;
in vec3 a_vTangent;
in vec3 a_vBitangent;
in vec2 a_vTexCoord;

// points
in int a_bAlive;
in float a_fPointSize;
in vec3 a_vRotation;
in vec3 a_vOffset;

smooth out vec3 v_vVertex;
smooth out vec3 v_vNormal;
smooth out vec2 v_vTexCoord;
smooth out vec3 v_vEye;
smooth out vec3 v_vTLight;
smooth out vec3 v_vTEye;
smooth out vec3 v_vTNormal;
smooth out mat3 v_matWorldToTangent;

// points
flat out int v_bAlive;

void main_points(void)
{
    v_bAlive = a_bAlive;

    mat3 matRotationY;
    matRotationY[0] = vec3(cos(a_vRotation.y), 0, -sin(a_vRotation.y));
    matRotationY[1] = vec3(0, 1, 0);
    matRotationY[2] = vec3(sin(a_vRotation.y), 0, cos(a_vRotation.y));

    vec4 vVertex = vec4(matRotationY * (a_vVertex + a_vOffset), 1.0);

    vec4 vModelViewVertex = u_matModelView * vVertex;
    v_vVertex = vec3(vModelViewVertex);

    gl_Position = u_matProjection * vModelViewVertex;
    gl_PointSize = a_fPointSize / length(vModelViewVertex);
}

void main_geometry(void)
{
    vec4 vVertex = vec4(a_vVertex, 1.0);
    vec4 vModelViewVertex = u_matModelView * vVertex;
    v_vVertex = vec3(vModelViewVertex);

    mat3 matNormal = transpose(inverse(mat3(u_matModelView)));
    v_vNormal = normalize(matNormal * normalize(a_vNormal));

    mat3 matTangentToWorld = matNormal * mat3(normalize(a_vTangent), normalize(a_vBitangent), normalize(a_vNormal));
    v_matWorldToTangent = transpose(matTangentToWorld);

    v_vEye = -v_vVertex;

    vec3 vLightPos = vec3(u_matModelView * vec4(0,0,2,1));

    v_vTLight = v_matWorldToTangent * (vLightPos - v_vVertex);
    v_vTEye = v_matWorldToTangent * v_vEye;
    v_vTNormal = v_matWorldToTangent * v_vNormal;

    v_vTexCoord = a_vTexCoord;
    gl_Position = u_matProjection * vModelViewVertex;
}

void main(void)
{
    if(u_bPoints == 1) main_points();
    else main_geometry();
}
