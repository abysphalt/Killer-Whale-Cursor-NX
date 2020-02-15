#ifndef __SHADER_H__
#define __SHADER_H__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>

#define GLM_FORCE_PURE
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern GLuint s_program;

void initShaders();
void deinitShaders();

#endif
