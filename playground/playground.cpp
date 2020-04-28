#include <cstdio>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "common/shader.hpp"
#include "common/controls.hpp"

#include "common/texture.hpp"

using namespace glm;

GLFWwindow* window;

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1024;
    int height = 768;

    window = glfwCreateWindow(width, height, "GL Playground", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    /* ================================================ */

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f,  1.0f, // A
         1.0f, -1.0f,  1.0f, // D
        -1.0f,  1.0f,  1.0f, // E
         1.0f,  1.0f,  1.0f, // H

        -1.0f, -1.0f, -1.0f, // B
         1.0f, -1.0f, -1.0f, // C
        -1.0f,  1.0f, -1.0f, // F
         1.0f,  1.0f, -1.0f, // G

        -1.0f, -1.0f, -1.0f, // B
        -1.0f, -1.0f,  1.0f, // A
        -1.0f,  1.0f, -1.0f, // F
        -1.0f,  1.0f,  1.0f, // E

         1.0f, -1.0f,  1.0f, // D
         1.0f, -1.0f, -1.0f, // C
         1.0f,  1.0f,  1.0f, // H
         1.0f,  1.0f, -1.0f, // G

        -1.0f, -1.0f,  1.0f, // A
         1.0f, -1.0f,  1.0f, // D
        -1.0f, -1.0f, -1.0f, // B
         1.0f, -1.0f, -1.0f, // C

        -1.0f,  1.0f,  1.0f, // E
         1.0f,  1.0f,  1.0f, // H
        -1.0f,  1.0f, -1.0f, // F
         1.0f,  1.0f, -1.0f, // G
    };

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    static const GLfloat g_uv_buffer_data[] = {
        0.0f, 0.0f, // A
        1.0f, 0.0f, // D
        0.0f, 1.0f, // E
        1.0f, 1.0f, // H

        0.0f, 0.0f, // B
        1.0f, 0.0f, // C
        0.0f, 1.0f, // F
        1.0f, 1.0f, // G

        0.0f, 0.0f, // B
        1.0f, 0.0f, // A
        0.0f, 1.0f, // F
        1.0f, 1.0f, // E

        0.0f, 0.0f, // D
        1.0f, 0.0f, // C
        0.0f, 1.0f, // H
        1.0f, 1.0f, // G

        0.0f, 0.0f, // A
        1.0f, 0.0f, // D
        0.0f, 1.0f, // B
        1.0f, 1.0f, // C

        0.0f, 0.0f, // E
        1.0f, 0.0f, // H
        0.0f, 1.0f, // F
        1.0f, 1.0f, // G
    };
    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


    static const GLuint g_index_buffer_data[] = {
        0, 2, 3,
        0, 3, 1,

        4, 6, 7,
        4, 7, 5,

        8, 10, 11,
        8, 11, 9,

        12, 14, 15,
        12, 15, 13,

        16, 18, 19,
        16, 19, 17,

        20, 22, 23,
        20, 23, 21,
    };
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), &g_index_buffer_data[0], GL_STATIC_DRAW);

    GLuint shaderProgramID = LoadShaders(
            "/home/oma/Code/CPP-Workspace/ogl/playground/vertex-shader.glsl",
            "/home/oma/Code/CPP-Workspace/ogl/playground/fragment-shader.glsl"
    );
    GLuint matrixID = glGetUniformLocation(shaderProgramID, "MVP");


    // Load the texture using any two methods
    GLuint Texture = loadBMP_custom("/home/oma/Code/CPP-Workspace/ogl/playground/res/tile.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(shaderProgramID, "myTextureSampler");

    /* ================================================ */

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    do {
        /* ==== UPDATE =================================== */

        computeMatricesFromInputs();
        mat4 projection = getProjectionMatrix();
        mat4 view = getViewMatrix();
        mat4 model = mat4(1.0f);
        mat4 mvp = projection * view * model;

        /* =============================================== */


        /* ==== DRAW ===================================== */

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgramID);
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);


        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*) nullptr
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(
                1,
                2,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void*) nullptr
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*) 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
        }

        glfwSwapBuffers(window);

        /* ============================================== */

        glfwPollEvents();
    }
    while (
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
        && glfwWindowShouldClose(window) == 0
    );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteProgram(shaderProgramID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

