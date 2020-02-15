#ifndef __FONT_H__
#define __FONT_H__

#include <switch.h>
#include <map>
#include "shader.h"

struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class Font {
public:
	std::map<GLchar, Character> Characters;
	GLuint s_vao, s_vbo;
	float r, g, b;
	
	Font(const char *filepath);
	void SetColor(float red, float green, float blue);
	void DrawText(GLfloat x, GLfloat y, GLfloat scale, const char *text, ...);
};

#endif
