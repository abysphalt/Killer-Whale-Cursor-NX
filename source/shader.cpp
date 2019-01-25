#include "shader.h"

GLuint s_program;

/*
Vert/frag shaders from devkitpro's textured_cube sample, slightly modified to remove unused properties:
https://github.com/devkitPro/switch-examples/blob/master/graphics/opengl/textured_cube/source/main.cpp
*/
static const char* const vertexShaderSource = R"text(
    #version 320 es
    precision mediump float;

    layout (location = 0) in vec3 inPos;
    layout (location = 1) in vec2 inTexCoord;

    out vec2 vtxTexCoord;
    out vec3 vtxView;

    uniform mat4 mdlvMtx;
    uniform mat4 projMtx;

    void main() {
        // Calculate position
        vec4 pos = mdlvMtx * vec4(inPos, 1.0);
        vtxView = -pos.xyz;
        gl_Position = projMtx * pos;

        // Calculate texcoord
        vtxTexCoord = inTexCoord;
    }
)text";

static const char* const fragmentShaderSource = R"text(
    #version 320 es
    precision mediump float;

    in vec2 vtxTexCoord;
    in vec3 vtxView;

    out vec4 fragColor;
	
	uniform vec3 diffuse;
    uniform sampler2D tex_diffuse;
	uniform float isText;

    void main() {
		if(isText == 0.0) {
			vec4 color = texture(tex_diffuse, vtxTexCoord);
		
			if(color.a == 0.0)
				discard;
		
			fragColor = color * vec4(diffuse, 1.0);
		}
		if(isText == 1.0) {
			float texRed = texture(tex_diffuse, vtxTexCoord).r;
			vec4 texColor = vec4(texRed, texRed, texRed, 1.0);
			if(texRed < 0.8)
				discard;
			fragColor = texColor * vec4(diffuse, 1.0);
		}
    }
)text";

GLuint createAndCompileShader(GLenum type, const char* source) {
	GLint success;
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    return handle;
}

void initShaders() {
	GLint success;
	
	GLint vsh = createAndCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLint fsh = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	
    s_program = glCreateProgram();
    glAttachShader(s_program, vsh);
    glAttachShader(s_program, fsh);
    glLinkProgram(s_program);

    glGetProgramiv(s_program, GL_LINK_STATUS, &success);
    glDeleteShader(vsh);
    glDeleteShader(fsh);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	
	glUseProgram(s_program);
	
	auto projMtx = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(s_program, "projMtx"), 1, GL_FALSE, glm::value_ptr(projMtx));
    glUniform1i(glGetUniformLocation(s_program, "tex_diffuse"), 0);
}

void deinitShaders() {
	glDeleteProgram(s_program);
}