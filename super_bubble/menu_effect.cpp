#include "menu_effect.h"
#include "defs.h"
#include "shader.h"
#include "resource_manager.h"

// Render state
static Shader shader;
static GLuint VAO;
static GLuint VBO;
static GLuint EBO;

// Initializes and configures the quad's buffer and vertex attributes
static void initRenderData();

void initEffectRenderer()
{
    shader = ResourceManager::LoadShader("../shaders/effect.vs", "../shaders/effect.frag", nullptr, "effect");
    initRenderData();
}

void drawMenuEffect(const double secondsSinceLastUpdate)
{
    static GLfloat time = secondsSinceLastUpdate;    
    shader.SetFloat("time", time, true);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    time += secondsSinceLastUpdate;
}

void deleteEffectVertexArrays() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
}

void initRenderData()
{
    GLfloat vertices[] = {        
         1.0f, -1.0f, 0.0f,        
         1.0f,  1.0f, 0.0f,        
        -1.0f,  1.0f, 0.0f,        
        -1.0f, -1.0f, 0.0f,
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind VAO (NOT the EBO)
    glBindVertexArray(0);
}