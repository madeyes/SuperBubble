/*******************************************************************
** This code is based on Breakout from learnopengl.com.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "shader.h"

void initSpriteRenderer(Shader &shaderToUse);
void deleteSpriteVertexArrays();

// Renders a defined quad textured with given sprite
void drawSprite(Texture2D &texture, glm::vec2 uvSize, GLuint atlasColumn, GLuint atlasRow, glm::uvec2 windowPosition, glm::uvec2 size = glm::uvec2(10, 10), GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f), float clipY = 0.0f);

#endif