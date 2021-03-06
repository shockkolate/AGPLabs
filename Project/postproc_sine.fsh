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

uniform sampler2D u_sFBO;
uniform float u_fOffset;

smooth in vec2 v_vCoord;

out vec4 o_vColor;

void main(void)
{
    vec2 vTexCoord = v_vCoord;
    vTexCoord.x += sin(vTexCoord.y * 4 * 2*3.14159265 + u_fOffset) /  100;
    o_vColor = texture2D(u_sFBO, vTexCoord);
}
