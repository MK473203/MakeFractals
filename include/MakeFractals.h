#ifndef FREMAKE
#define FREMAKE

// The code for combining everything together into an interacive application

#define BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>
void boost::throw_exception(std::exception const& e) {
    throw 11;
}
void boost::throw_exception(std::exception const& e, boost::source_location const& loc) {
    throw 11;
}


#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Fractal_Draw.h"
#include "Fractal_Maths.h"
#include "Fractal_Utils.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

const char* renderVertShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(aPos, 0.0f, 1.0f);
    TexCoord = aTexCoord;
}
)";

const char* renderFragShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)";

const char* screenVertShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0f, 1.0f);
    TexCoord = aTexCoord;
}
)";

const char* screenFragShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}
)";

#endif // !FREMAKE