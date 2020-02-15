#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <switch.h>
#include "shader.h"

class Sprite {
public:
	float posX, posY;
	float scaleX, scaleY;
	int texWidth, texHeight;
	float r, g, b;
	
	GLuint s_vao, s_vbo, s_tex;
	
	Sprite();
	
	void Init(float posX, float posY, float scaleX, float scaleY, const char *filepath);
	void SetColor(float red, float green, float blue);
	void Resize(float Xscale, float Yscale);
	void SetTexture(const char *filepath);
	void Draw();
};

#endif