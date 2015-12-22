/*******************************************************************
** This code is based on Breakout from learnopengl.com.
** Added support for texture atlas.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "sprite_renderer.h"
#include "defs.h"

// Render state
static Shader shader;
static GLuint VAO;
static GLuint VBO;
static GLuint EBO;

// Initializes and configures the quad's buffer and vertex attributes
static void initRenderData();

void initSpriteRenderer(Shader &shaderToUse)
{
    shader = shaderToUse;
    initRenderData();
}

void deleteSpriteVertexArrays() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
}

/**
* Draw sprite from texture atlas.
*
*/
void drawSprite(Texture2D &texture, glm::vec2 uvSize, GLuint atlasColumn, GLuint atlasRow, glm::uvec2 windowPosition, glm::uvec2 size, GLfloat rotate, glm::vec3 color, float clipY)
{
    // Prepare transformations
    shader.Use();
    glm::mat4 model;

    // First translate 
    // (transformations are: scale happens first, then rotation and then final translation happens; reversed order)
    model = glm::translate(model, glm::vec3(windowPosition, 0.0f));

    // Move origin of rotation to center of quad.
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    // Then rotate.
    model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f));
    // Move origin back.
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    // Last scale.
    model = glm::scale(model, glm::vec3(size, 1.0f));

    shader.SetMatrix4("model", model);

    // Render textured quad.
    shader.SetVector3f("spriteColor", color);
    shader.SetFloat("clipY", HEIGHT - clipY);

    // Parameters for texture atlas (width, height, column, row)
    glm::vec4 atlasParams = glm::vec4(uvSize.x, uvSize.y, static_cast<float>(atlasColumn), static_cast<float>(atlasRow));
    shader.SetVector4f("atlasParams", atlasParams);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void initRenderData()
{
    GLfloat vertices[] = {
        // Top Right
        1.0f, 0.0f, 1.0f, 0.0f,
        // Bottom Right
        1.0f, 1.0f, 1.0f, 1.0f,
        // Bottom Left
        0.0f, 1.0f, 0.0f, 1.0f,
        // Top Left 
        0.0f, 0.0f, 0.0f, 0.0f
    };
    GLuint indices[] = {
        2, 0, 3,
        2, 1, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind VAO (NOT the EBO)
    glBindVertexArray(0);
}