#include "sprite.h"
#include "utils.h"

typedef struct {
	float position[3];
	float texcoord[2];
} Vertex;

Sprite::Sprite() { }

void Sprite::Init(float Xpos, float Ypos, float Xscale, float Yscale, const char *filepath) {
	posX = Xpos;
	posY = Ypos;
	scaleX = Xscale;
	scaleY = Yscale;
	
	SetColor(1.0f, 1.0f, 1.0f);
	Resize(Xscale, Yscale);
	SetTexture(filepath);
}

void Sprite::SetColor(float red, float green, float blue) {
	r = red;
	g = green;
	b = blue;
}

void Sprite::Resize(float Xscale, float Yscale) {
	Vertex vertex_list[] = {
		{ {-Xscale, +Yscale, +0.0f}, {0.0f, 0.0f} },
		{ {-Xscale, -Yscale, +0.0f}, {0.0f, 1.0f} },
		{ {+Xscale, -Yscale, +0.0f}, {1.0f, 1.0f} },
		
		{ {+Xscale, -Yscale, +0.0f}, {1.0f, 1.0f} },
		{ {+Xscale, +Yscale, +0.0f}, {1.0f, 0.0f} },
		{ {-Xscale, +Yscale, +0.0f}, {0.0f, 0.0f} },
	};
	
	glGenVertexArrays(1, &s_vao);
	glGenBuffers(1, &s_vbo);
	glBindVertexArray(s_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Sprite::SetTexture(const char *filepath) {
	s_tex = png_texture_load(filepath, &texWidth, &texHeight);
}

void Sprite::Draw() {
	glUniform1f(glGetUniformLocation(s_program, "isText"), 0.0);
	glUniform3f(glGetUniformLocation(s_program, "diffuse"), r, g, b);
	
	glm::mat4 modelMtx{1.0};
	modelMtx = glm::translate(modelMtx, glm::vec3{ posX, posY, 0.0f });
	glUniformMatrix4fv(glGetUniformLocation(s_program, "mdlvMtx"), 1, GL_FALSE, glm::value_ptr(modelMtx));
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_tex);
	glBindVertexArray(s_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
