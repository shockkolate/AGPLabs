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

#include "Game.h"

void Game::PrintShaderError(GLint shader)
{
    GLint max_len = 0;
    GLint log_len = 0;
    unsigned char is_shader = glIsShader(shader);

    if(is_shader) glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);
    else glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_len);

    if(max_len > 0)
    {
        GLchar *message = (GLchar *)malloc(sizeof(GLchar) * max_len);

        if(is_shader) glGetShaderInfoLog(shader, max_len, &log_len, message);
        else glGetProgramInfoLog(shader, max_len, &log_len, message);

        fprintf(stderr, "%s\n", message);
        free(message);
    }
}

bool Game::InitSDL(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL_Init: error: %s\n", SDL_GetError());
        return false;
    }

    // create dummy window and context to test default options

    this->wnd = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if(!wnd)
    {
        fprintf(stderr, "SDL_CreateWindow: dummy error: %s\n", SDL_GetError());
        return false;
    }

    this->ctx = SDL_GL_CreateContext(wnd);
    if(!this->ctx)
    {
        fprintf(stderr, "SDL_GL_CreateContext: dummy error: %s\n", SDL_GetError());
        return false;
    }

    // debug info
    fprintf(stderr, "[DUMMY] OpenGL version: %s\n", glGetString(GL_VERSION));
    fprintf(stderr, "[DUMMY] GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // destroy dummy window and context
    SDL_GL_DeleteContext(this->ctx);
    SDL_DestroyWindow(this->wnd);

    // set attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // create real window and context

    this->wnd = SDL_CreateWindow("Game",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 1280, 720,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(!this->wnd)
    {
        fprintf(stderr, "SDL_CreateWindow: error: %s\n", SDL_GetError());
        return false;
    }

    this->ctx = SDL_GL_CreateContext(wnd);
    if(!this->ctx)
    {
        fprintf(stderr, "SDL_GL_CreateContext: error: %s\n", SDL_GetError());
        return false;
    }

    // debug info
    fprintf(stderr, "OpenGL version: %s\n", glGetString(GL_VERSION));
    fprintf(stderr, "GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // enable vsync
    SDL_GL_SetSwapInterval(1);

    return true;
}

bool Game::InitGLEW(void)
{
#if defined(_WIN32) || defined(__linux__)
    glewExperimental = true;

    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "glewInit: error\n");
        return 1;
    }
#endif

    return true;
}

bool Game::InitShaders(const char *v_path, const char *f_path)
{
    long v_len, f_len;
    const char *v_src = ResourceManager::Load(v_path, &v_len);
    const char *f_src = ResourceManager::Load(f_path, &f_len);

    if(v_src == NULL)
    {
        fprintf(stderr, "::InitShaders: error: failed to load vertex shader\n");
        return false;
    }

    if(f_src == NULL)
    {
        fprintf(stderr, "::InitShaders: error: failed to load fragment shader\n");
        return false;
    }

    GLuint v_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint f_id = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(v_id, 1, &v_src, (GLint *)&v_len);
    glShaderSource(f_id, 1, &f_src, (GLint *)&f_len);

    GLint compile_status;

    glCompileShader(v_id);
    glGetShaderiv(v_id, GL_COMPILE_STATUS, &compile_status);
    if(!compile_status)
    {
        fprintf(stderr, "glCompileShader(v_id): error\n");
        this->PrintShaderError(v_id);
        return false;
    }

    glCompileShader(f_id);
    glGetShaderiv(f_id, GL_COMPILE_STATUS, &compile_status);
    if(!compile_status)
    {
        fprintf(stderr, "glCompileShader(f_id): error\n");
        this->PrintShaderError(f_id);
        return false;
    }

    this->program_id = glCreateProgram();
    glAttachShader(this->program_id, v_id);
    glAttachShader(this->program_id, f_id);

    glBindAttribLocation(this->program_id, GAME_ATTRIB_VERTEX, "a_vVertex");
    glBindAttribLocation(this->program_id, GAME_ATTRIB_NORMAL, "a_vNormal");
    glBindAttribLocation(this->program_id, GAME_ATTRIB_TEXCOORD, "a_vTexCoord");

    glLinkProgram(this->program_id);
    glUseProgram(this->program_id);

    return true;
}

bool Game::DestroySDL(void)
{
    SDL_DestroyWindow(this->wnd);
    SDL_Quit();
    return true;
}

bool Game::Init(void)
{
    if(!this->InitSDL()) return false;
    if(!this->InitGLEW()) return false;
    if(!this->InitShaders("parallax-occlusion.vsh", "parallax-occlusion.fsh")) return false;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->matIdentity = glm::mat4(1.0f);
    this->camera = glm::translate(this->matIdentity, glm::vec3(0.0f, 0.0f, -5.0f));

    this->cube.Init(this->program_id);

    // texture

    SDL_Surface *texture = SDL_LoadBMP("stone.bmp");

    glUniform1i(glGetUniformLocation(this->program_id, "u_texture"), 0);
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &this->tex);
    glBindTexture(GL_TEXTURE_2D, this->tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    GLfloat aniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

    GLenum format = 0;
    switch(texture->format->BytesPerPixel)
    {
        case 3:
            format = texture->format->Rmask == 0xFF ? GL_RGB : GL_BGR;
            break;
        case 4:
            format = texture->format->Rmask == 0xFF ? GL_RGBA : GL_BGRA;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, format,
                 GL_UNSIGNED_BYTE, texture->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(texture);

    // normal map

    SDL_Surface *nmap = SDL_LoadBMP("four_NM_height.bmp");

    glUniform1i(glGetUniformLocation(this->program_id, "u_nmap"), 1);
    glActiveTexture(GL_TEXTURE1);

    glGenTextures(1, &this->nmap);
    glBindTexture(GL_TEXTURE_2D, this->nmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

    format = 0;
    switch(nmap->format->BytesPerPixel)
    {
        case 3:
            format = nmap->format->Rmask == 0xFF ? GL_RGB : GL_BGR;
            break;
        case 4:
            format = nmap->format->Rmask == 0xFF ? GL_RGBA : GL_BGRA;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nmap->w, nmap->h, 0, format,
                 GL_UNSIGNED_BYTE, nmap->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(nmap);

    // gloss map

    SDL_Surface *glossmap = SDL_LoadBMP("stone_gloss.bmp");

    glUniform1i(glGetUniformLocation(this->program_id, "u_glossmap"), 2);
    glActiveTexture(GL_TEXTURE2);

    glGenTextures(1, &this->glossmap);
    glBindTexture(GL_TEXTURE_2D, this->glossmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

    format = 0;
    switch(glossmap->format->BytesPerPixel)
    {
        case 3:
            format = glossmap->format->Rmask == 0xFF ? GL_RGB : GL_BGR;
            break;
        case 4:
            format = glossmap->format->Rmask == 0xFF ? GL_RGBA : GL_BGRA;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glossmap->w, glossmap->h, 0, format,
                 GL_UNSIGNED_BYTE, glossmap->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(glossmap);

    return true;
}

bool Game::HandleSDL(SDL_Event *e)
{
    switch(e->type)
    {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
            // remove key if already in pressed_keys
            for(std::vector<SDL_Keycode>::iterator i=this->pressed_keys.begin();
                i!=this->pressed_keys.end();
                ++i)
            {
                if(*i == e->key.keysym.sym)
                {
                    this->pressed_keys.erase(i);
                    break;
                }
            }

            this->pressed_keys.push_back(e->key.keysym.sym);
            break;
        case SDL_KEYUP:
            // remove key if in pressed_keys
            for(std::vector<SDL_Keycode>::iterator i=this->pressed_keys.begin();
                i!=this->pressed_keys.end(); ++i)
            {
                if(*i == e->key.keysym.sym)
                {
                    this->pressed_keys.erase(i);
                    break;
                }
            }
            break;
    }

    return true;
}

bool Game::Update(float seconds)
{
    const float movement_speed = 1.5f;
    const float rotation_speed = 70.0f;

    for(std::vector<SDL_Keycode>::iterator i=this->pressed_keys.begin();
        i!=this->pressed_keys.end(); ++i)
    {
        switch(*i)
        {
            case SDLK_ESCAPE:
                return false;
            case SDLK_w:
                this->camera = glm::translate(this->matIdentity, glm::vec3(0, 0, movement_speed * seconds)) * this->camera;
                break;
            case SDLK_s:
                this->camera = glm::translate(this->matIdentity, glm::vec3(0, 0, -movement_speed * seconds)) * this->camera;
                break;
            case SDLK_a:
                this->camera = glm::translate(this->matIdentity, glm::vec3(movement_speed * seconds, 0, 0)) * this->camera;
                break;
            case SDLK_d:
                this->camera = glm::translate(this->matIdentity, glm::vec3(-movement_speed * seconds, 0, 0)) * this->camera;
                break;
            case SDLK_SPACE:
                this->camera = glm::translate(this->matIdentity, glm::vec3(0, -movement_speed * seconds, 0)) * this->camera;
                break;
            case SDLK_c:
                this->camera = glm::translate(this->matIdentity, glm::vec3(0, movement_speed * seconds, 0)) * this->camera;
                break;
            case SDLK_UP:
                this->camera = glm::rotate(this->matIdentity, -rotation_speed * seconds, glm::vec3(1, 0, 0)) * this->camera;
                break;
            case SDLK_DOWN:
                this->camera = glm::rotate(this->matIdentity, rotation_speed * seconds, glm::vec3(1, 0, 0)) * this->camera;
                break;
            case SDLK_LEFT:
                this->camera = glm::rotate(this->matIdentity, -rotation_speed * seconds, glm::vec3(0, 1, 0)) * this->camera;
                break;
            case SDLK_RIGHT:
                this->camera = glm::rotate(this->matIdentity, rotation_speed * seconds, glm::vec3(0, 1, 0)) * this->camera;
                break;
            case SDLK_q:
                this->camera = glm::rotate(this->matIdentity, -rotation_speed * seconds, glm::vec3(0, 0, 1)) * this->camera;
                break;
            case SDLK_e:
                this->camera = glm::rotate(this->matIdentity, rotation_speed * seconds, glm::vec3(0, 0, 1)) * this->camera;
                break;
        }
    }

    return true;
}

bool Game::Draw(void)
{
    glClearColor(0.6f, 0.65f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 matProjection = glm::perspective(35.0f, 1280.0f / 720.0f, 0.01f, 100.0f);
    glm::mat4 matModelView = this->camera;
    glm::mat4 matObjectModelView = matModelView;

    GLint u_matProjection = glGetUniformLocation(this->program_id, "u_matProjection");
    GLint u_matModelView = glGetUniformLocation(this->program_id, "u_matModelView");
    GLint u_matObjectModelView = glGetUniformLocation(this->program_id, "u_matObjectModelView");

    glUniformMatrix4fv(u_matProjection, 1, GL_FALSE, glm::value_ptr(matProjection));
    glUniformMatrix4fv(u_matModelView, 1, GL_FALSE, glm::value_ptr(matModelView));
    glUniformMatrix4fv(u_matObjectModelView, 1, GL_FALSE, glm::value_ptr(matObjectModelView));

    this->cube.Draw();

    SDL_GL_SwapWindow(this->wnd);

    return true;
}

bool Game::Destroy(void)
{
    if(!this->DestroySDL()) return false;
    return true;
}

int Game::Run(int argc, const char **argv)
{
    if(!this->Init()) return 1;

    Uint32 current_time = SDL_GetTicks();
    Uint32 previous_time;
    float accumulator = 0.0f;
    const float timestep = 1.0f / 120.0f;

    SDL_Event e;
    for(this->running=true; this->running;)
    {
        previous_time = current_time;
        current_time = SDL_GetTicks();
        accumulator += (current_time - previous_time) / 1000.0f;

        for(;accumulator >= timestep; accumulator -= timestep)
        {
            if(!this->Update(timestep))
            {
                this->running = false;
                break;
            }
        }

        if(this->running)
        {
            if(!this->Draw()) this->running = false;
        }

        while(SDL_PollEvent(&e))
        {
            if(!this->HandleSDL(&e))
            {
                this->running = false;
                break;
            }
        }
    }

    if(!this->Destroy()) return 1;

    return 0;
}
