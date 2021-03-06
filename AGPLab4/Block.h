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

#ifndef BLOCK_H
#define BLOCK_H

#include <glm/gtc/type_precision.hpp>

#include "Object.h"

class Block : public Object<Block>
{
protected:
    glm::i8vec4 vertices[36];
    glm::i8vec4 normals[36];

    // TODO: position
public:
    Block(void);
    bool Init(void);
    bool Draw(void);
};

#endif
