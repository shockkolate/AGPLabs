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

#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "common.h"
#include "LightingManager.h"

class Drawable
{
protected:
    GLuint vao;
    GLuint vbo_vertex;
    GLuint vbo_normal;
    GLuint vbo_tangent;
    GLuint vbo_bitangent;
    GLuint vbo_texcoord;

    GLenum usage;

    glm::vec3 *vertices;
    glm::vec3 *normals;
    glm::vec3 *tangents;
    glm::vec3 *bitangents;
    glm::vec2 *texcoords;

    virtual unsigned int Make(glm::vec3 **vertices,
                              glm::vec3 **normals,
                              glm::vec3 **tangents,
                              glm::vec3 **bitangents,
                              glm::vec2 **texcoords) = 0;
public:
#define GAME_DOMAIN "Drawable::MakeBuffer"
    static GLuint MakeBuffer(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
    {
        GLuint id;

        ASSERT_GL(glGenBuffers(1, &id))
        ASSERT_GL(glBindBuffer(target, id))
        ASSERT_GL(glBufferData(target, size, data, usage))

        return id;
    }
#undef GAME_DOMAIN

#define GAME_DOMAIN "Drawable::UpdateBuffer"
    static void UpdateBuffer(GLuint vbo, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
    {
        ASSERT_GL(glBindBuffer(target, vbo))
        ASSERT_GL(glBufferSubData(target, offset, size, data))
    }
#undef GAME_DOMAIN

#define GAME_DOMAIN "Drawable::MakeVBO"
    static GLuint MakeVBO(GLsizeiptr size, const GLvoid *data, GLuint program_id,
                         const GLchar *attrib_name, GLint attrib_size, GLenum attrib_type,
                         GLenum usage)
    {
        GLuint vbo = Drawable::MakeBuffer(GL_ARRAY_BUFFER, size, data, usage);
        EnableAttrib(program_id, attrib_name, attrib_size, attrib_type);

        return vbo;
    }
#undef GAME_DOMAIN

#define GAME_DOMAIN "Drawable::EnableAttrib"
    static void EnableAttrib(GLuint program_id, const GLchar *attrib_name, GLint attrib_size, GLenum attrib_type)
    {
        ASSERT_GL(GLint attrib_id = glGetAttribLocation(program_id, attrib_name))
        if(attrib_id < 0) fprintf(stderr, "Drawable::make_vbo: attribute \"%s\" is not active in program\n", attrib_name);

        ASSERT_GL(glVertexAttribPointer(attrib_id, attrib_size, attrib_type, GL_FALSE, 0, NULL))
        ASSERT_GL(glEnableVertexAttribArray(attrib_id))
    }
#undef GAME_DOMAIN

    GLint material_id;
    glm::vec3 position;

    Drawable(GLenum usage = GL_STATIC_DRAW) : usage(usage), material_id(0), position(glm::vec3(0)) {}

#define GAME_DOMAIN "Drawable::Init"
    void Init(GLuint program_id)
    {
        unsigned int num = Make(&vertices, &normals, &tangents, &bitangents, &texcoords);

        ASSERT_GL(glGenVertexArrays(1, &this->vao))
        Bind();

        vbo_vertex    = MakeVBO(num * sizeof(glm::vec3), vertices,   program_id, "a_vVertex",    3, GL_FLOAT, usage);
        vbo_normal    = MakeVBO(num * sizeof(glm::vec3), normals,    program_id, "a_vNormal",    3, GL_FLOAT, usage);
        vbo_tangent   = MakeVBO(num * sizeof(glm::vec3), tangents,   program_id, "a_vTangent",   3, GL_FLOAT, usage);
        vbo_bitangent = MakeVBO(num * sizeof(glm::vec3), bitangents, program_id, "a_vBitangent", 3, GL_FLOAT, usage);
        vbo_texcoord  = MakeVBO(num * sizeof(glm::vec2), texcoords,  program_id, "a_vTexCoord",  2, GL_FLOAT, usage);
    }
#undef GAME_DOMAIN

#define GAME_DOMAIN "Drawable::Bind"
    void Bind(void)
    {
        ASSERT_GL(glBindVertexArray(vao))
    }
#undef GAME_DOMAIN

#define GAME_DOMAIN "Drawable::Draw"
    virtual void Draw(GLuint program_id, GLint u_matModelView, glm::mat4 matModelView)
    {
        LightingManager::SetMaterial(program_id, material_id);

        matModelView = glm::translate(matModelView, position);
        ASSERT_GL(glUniformMatrix4fv(u_matModelView, 1, GL_FALSE, glm::value_ptr(matModelView)))

        Bind();
    }
#undef GAME_DOMAIN
};

#endif
