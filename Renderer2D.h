#ifndef RENDERER2D_H
#define RENDERER2D_H

#include <glad/glad.h>
#include <string>
#include <iostream>
#include "Texture.h"

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 800;

// helper: print shader compile/link errors
static void checkShaderCompile(unsigned int shader, const char* name) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, NULL, info);
        std::cerr << "ERROR: Shader Compile failed (" << name << "):\n" << info << std::endl;
    }
}
static void checkProgramLink(unsigned int prog) {
    int success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(prog, 1024, NULL, info);
        std::cerr << "ERROR: Program Link failed:\n" << info << std::endl;
    }
}

// Simple 2D Renderer for textured quads
class Renderer2D {
private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int shaderProgram = 0;
    
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        uniform mat4 transform;
        void main() {
            gl_Position = transform * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D texture1;
        uniform vec4 color;
        uniform bool useTexture;
        void main() {
            if (useTexture) {
                vec4 t = texture(texture1, TexCoord);
                FragColor = t * color;
            } else {
                FragColor = color;
            }
        }
    )";
    
public:
    Renderer2D() {
        // Setup quad vertices (-0.5..0.5 centered) and tex coords
        float vertices[] = {
            // positions        // tex coords (flipped V so images appear right-side up)
            -0.5f, -0.5f,       0.0f, 1.0f,
             0.5f, -0.5f,       1.0f, 1.0f,
             0.5f,  0.5f,       1.0f, 0.0f,
            -0.5f,  0.5f,       0.0f, 0.0f
        };
        unsigned int indices[] = {0, 1, 2, 2, 3, 0};
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texcoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Create shader
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexShaderSource, NULL);
        glCompileShader(vertex);
        checkShaderCompile(vertex, "vertex");
        
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragment);
        checkShaderCompile(fragment, "fragment");
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertex);
        glAttachShader(shaderProgram, fragment);
        glLinkProgram(shaderProgram);
        checkProgramLink(shaderProgram);
        
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    void drawQuad(float x, float y, float width, float height, 
                  Texture* tex = nullptr, float r = 1, float g = 1, float b = 1, float a = 1) {
        glUseProgram(shaderProgram);
        
        // Transform: scale to pixel size, then translate to screen position
        // We use a column-major matrix for OpenGL
        // Scale: (width, height) in pixels → NDC scale
        // Translate: (x, y) top-left in pixels → NDC position
        // Y is flipped: y=0 at top, y=SCREEN_HEIGHT at bottom
        // NDC: x=-1..1, y=-1(bottom)..1(top)
        // translate_x = -1 + ( (x + width/2) * 2 / SCREEN_WIDTH )
        // translate_y = 1 - ( (y + height/2) * 2 / SCREEN_HEIGHT )
        float sx = (width * 2.0f) / float(SCREEN_WIDTH);
        float sy = -(height * 2.0f) / float(SCREEN_HEIGHT); // flip Y
        float tx = ((x + width * 0.5f) * 2.0f) / float(SCREEN_WIDTH) - 1.0f;
        float ty = 1.0f - ((y + height * 0.5f) * 2.0f) / float(SCREEN_HEIGHT);
        
        // Column-major matrix for OpenGL:
        float transform[16] = {
            sx, 0.0f, 0.0f, 0.0f,
            0.0f, sy, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            tx,  ty,  0.0f, 1.0f
        };
        
        int tloc = glGetUniformLocation(shaderProgram, "transform");
        if (tloc >= 0) glUniformMatrix4fv(tloc, 1, GL_FALSE, transform);
        int cloc = glGetUniformLocation(shaderProgram, "color");
        if (cloc >= 0) glUniform4f(cloc, r, g, b, a);
        
        if (tex) {
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
            glActiveTexture(GL_TEXTURE0);
            tex->bind();
        } else {
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
        }
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    // Draw a pixel for bitmap font
    void drawPixel(float x, float y, float pixelSize, float r, float g, float b) {
        drawQuad(x, y, pixelSize, pixelSize, nullptr, r, g, b, 1.0f);
    }
    
    // Draw a character using bitmap patterns
    void drawChar(char c, float x, float y, float size, float r, float g, float b) {
        float pixelSize = size / 7.0f;
        
        // 5x7 bitmap font patterns
        if (c == 'A') {
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 4*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'B') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'C') {
            for (int i = 1; i < 6; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'D') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'E') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            for (int i = 0; i < 4; i++) {
                drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
        }
        else if (c == 'F') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            for (int i = 0; i < 4; i++) {
                drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            }
        }
        else if (c == 'G') {
            for (int i = 1; i < 6; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            for (int i = 1; i < 4; i++) {
                drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + 3*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'H') {
            for (int i = 0; i < 7; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'I') {
            for (int i = 0; i < 7; i++) drawPixel(x + pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            for (int i = 0; i < 3; i++) {
                drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
        }
        else if (c == 'J') {
            for (int i = 0; i < 7; i++) drawPixel(x + 2*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'K') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'L') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            for (int i = 1; i < 4; i++) drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'M') {
            for (int i = 0; i < 7; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 4*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'N') {
            for (int i = 0; i < 7; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'O') {
            for (int i = 1; i < 6; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'P') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'Q') {
            for (int i = 1; i < 6; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'R') {
            for (int i = 0; i < 7; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'S') {
            for (int i = 1; i < 4; i++) drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            for (int i = 0; i < 3; i++) drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'T') {
            for (int i = 0; i < 5; i++) drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
            for (int i = 1; i < 7; i++) drawPixel(x + 2*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'U') {
            for (int i = 0; i < 6; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'V') {
            for (int i = 0; i < 5; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'W') {
            for (int i = 0; i < 7; i++) {
                drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 4*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'X') {
            drawPixel(x, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 6*pixelSize, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'Y') {
            drawPixel(x, y, pixelSize, r, g, b);
            drawPixel(x + 3*pixelSize, y, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + pixelSize, pixelSize, r, g, b);
            for (int i = 2; i < 7; i++) drawPixel(x + pixelSize, y + i*pixelSize, pixelSize, r, g, b);
        }
        else if (c == 'Z') {
            for (int i = 0; i < 4; i++) {
                drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
            drawPixel(x + 2*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            drawPixel(x + pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
            drawPixel(x, y + 5*pixelSize, pixelSize, r, g, b);
        }
        else if (c >= '0' && c <= '9') {
            int digit = c - '0';
            // Numbers 0-9 patterns
            if (digit == 0) {
                for (int i = 1; i < 6; i++) {
                    drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                    drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
                }
                drawPixel(x + pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 1) {
                for (int i = 0; i < 7; i++) drawPixel(x + pixelSize, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x, y + pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 2) {
                drawPixel(x, y, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
                drawPixel(x, y + 5*pixelSize, pixelSize, r, g, b);
                for (int i = 0; i < 4; i++) drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 3) {
                for (int i = 0; i < 4; i++) {
                    drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
                }
                drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 4) {
                for (int i = 0; i < 4; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                for (int i = 0; i < 7; i++) drawPixel(x + 2*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 5) {
                for (int i = 0; i < 4; i++) drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x, y + pixelSize, pixelSize, r, g, b);
                drawPixel(x, y + 2*pixelSize, pixelSize, r, g, b);
                for (int i = 0; i < 3; i++) drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
                for (int i = 0; i < 3; i++) drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 6) {
                for (int i = 1; i < 6; i++) drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y, pixelSize, r, g, b);
                for (int i = 0; i < 3; i++) drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 4*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + 5*pixelSize, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 7) {
                for (int i = 0; i < 4; i++) drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                drawPixel(x + 3*pixelSize, y + pixelSize, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y + 2*pixelSize, pixelSize, r, g, b);
                for (int i = 3; i < 7; i++) drawPixel(x + pixelSize, y + i*pixelSize, pixelSize, r, g, b);
            }
            else if (digit == 8) {
                for (int i = 1; i < 6; i++) {
                    drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                    drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
                }
                for (int i = 1; i < 3; i++) {
                    drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
                }
            }
            else if (digit == 9) {
                for (int i = 1; i < 4; i++) {
                    drawPixel(x, y + i*pixelSize, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y, pixelSize, r, g, b);
                    drawPixel(x + i*pixelSize, y + 3*pixelSize, pixelSize, r, g, b);
                }
                for (int i = 1; i < 6; i++) drawPixel(x + 3*pixelSize, y + i*pixelSize, pixelSize, r, g, b);
                drawPixel(x + pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
                drawPixel(x + 2*pixelSize, y + 6*pixelSize, pixelSize, r, g, b);
            }
        }
    }
    
    void drawText(const std::string& text, float x, float y, float size, float r = 1, float g = 1, float b = 1) {
        float charWidth = size * 0.7f;
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (c == ' ') {
                x += charWidth * 0.5f;
                continue;
            }
            drawChar(c, x, y, size, r, g, b);
            x += charWidth;
        }
    }
};

#endif
