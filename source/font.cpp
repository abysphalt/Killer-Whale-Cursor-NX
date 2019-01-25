#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    float position[3];
    float texcoord[2];
} Vertex;

Font::Font(const char *filepath) {
	FT_Library ft;
	FT_Init_FreeType(&ft);

	FT_Face face;
	FT_New_Face(ft, filepath, 0, &face);
	
	FT_Set_Pixel_Sizes(face, 0, 48);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
  
	for (GLubyte c = 0; c < 128; c++) {
		// Load character glyph 
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Now store character for later use
		Character character = {
			texture, 
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(GLuint)face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	FT_Done_Face(face);
    FT_Done_FreeType(ft);
	
	glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    glBindVertexArray(s_vao);
	
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, NULL, GL_DYNAMIC_DRAW);
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
	glEnableVertexAttribArray(1);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	SetColor(1.0f, 1.0f, 1.0f);
}

void Font::SetColor(float red, float green, float blue) {
	r = red;
	g = green;
	b = blue;
}

void Font::DrawText(GLfloat x, GLfloat y, GLfloat scale, const char *text, ...) {
	glUniform1f(glGetUniformLocation(s_program, "isText"), 1.0);
	glUniform3f(glGetUniformLocation(s_program, "diffuse"), r, g, b);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(s_vao);
	
	char formattedText[256];
	
	va_list args;
    va_start(args, text);
	vsprintf(formattedText, text, args);
	va_end(args);
	
	glm::mat4 modelMtx{1.0};
	modelMtx = glm::translate(modelMtx, glm::vec3{ 0.0f, 0.0f, 0.0f });
	glUniformMatrix4fv(glGetUniformLocation(s_program, "mdlvMtx"), 1, GL_FALSE, glm::value_ptr(modelMtx));

    // Iterate through all characters
    for (int c = 0; c < int(strlen(formattedText)); c++) {
        Character ch = Characters[formattedText[c]];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y + (Characters['T'].Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
		Vertex vertices[] = {
			{ {xpos,     ypos + h, +0.0f}, {0.0f, 1.0f} },
			{ {xpos,     ypos,     +0.0f}, {0.0f, 0.0f} },
			{ {xpos + w, ypos,     +0.0f}, {1.0f, 0.0f} },
			
			{ {xpos + w, ypos,     +0.0f}, {1.0f, 0.0f} },
			{ {xpos + w, ypos + h, +0.0f}, {1.0f, 1.0f} },
			{ {xpos,     ypos + h, +0.0f}, {0.0f, 1.0f} },
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0]));
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
	glDisable(GL_BLEND);
}