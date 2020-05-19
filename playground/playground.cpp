#include <cstdio>
#include <iostream>
#include <vector>
#include <exception>
#include <map>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/vboindexer.hpp"

#include <ft2build.h>
#include <glm/gtc/matrix_transform.hpp>
#include FT_FREETYPE_H

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Model.hpp"

using namespace glm;

GLFWwindow* window;

struct Character {
    unsigned int TextureId;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    FT_Pos       Advance;
};

std::map<char, Character> loadChars(FT_Face face) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    std::map<char, Character> characters;
    for (unsigned char c = 0; c < 128; ++c){
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)){
            fprintf(stderr, "Failed to load Glyph.");
            continue;
        }
        
        unsigned int texture;
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
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        
        characters.insert(std::pair<char, Character>(c, character));
    }
    
    return characters;
}

std::map<char, Character> loadFonts() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init FreeType library.");
    }
    
    FT_Face face;
    if (FT_New_Face(ft, "/home/oma/Code/CPP-Workspace/ogl/playground/res/fonts/arial.ttf", 0, &face)) {
        fprintf(stderr, "Could not load the font.");
    }
    
    FT_Set_Pixel_Sizes(face, 0, 48);
    
    std::map<char, Character> characters = loadChars(face);
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    return characters;
}

void renderText(
    GLuint textVao,
    GLuint textVbo,
    GLuint shaderId,
    std::map<char, Character> characters,
    std::string text,
    glm::vec2 position,
    float scale,
    glm::vec3 color
)
{
    glUseProgram(shaderId);
    
    glActiveTexture(GL_TEXTURE0);
    
    glBindVertexArray(textVao);
    glBindBuffer(GL_ARRAY_BUFFER, textVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        4,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*) nullptr
    );
    
    glUniform1i(glGetUniformLocation(shaderId, "text"), 0);
    glUniform3f(glGetUniformLocation(shaderId, "textColor"), color.x, color.y, color.z);
    
    std::string::const_iterator it;
    float x = position.x;
    float y = position.y;
    for (it = text.begin(); it != text.end(); ++it){
        Character c = characters[*it];
        
        float xpos = x + c.Bearing.x * scale;
        float ypos = y - (c.Size.y - c.Bearing.y) * scale;
        
        float w = c.Size.x * scale;
        float h = c.Size.y * scale;
        
        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            
            { xpos    , ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos    , 1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };
        
        glBindTexture(GL_TEXTURE_2D, c.TextureId);
    
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (c.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam
)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    
    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;
    
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;
    
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

int init(int width, int height) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
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
    
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    
    
    return 0;
}

int main() {
    int width = 1024;
    int height = 768;
    
    int initResult = init(width, height);
    if (initResult != 0) {
        return initResult;
    }
    
    std::map<char, Character> characters = loadFonts();

    /* ================================================ */

    // Load meshes
    Mesh* cubeMesh = nullptr;
    try {
        cubeMesh = new Mesh("/home/oma/Code/CPP-Workspace/ogl/playground/res/cube.obj");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[MESH LOAD ERROR]: %s", e.what());
        return -1;
    }

    Mesh* floorMesh = nullptr;
    try {
        floorMesh = new Mesh("/home/oma/Code/CPP-Workspace/ogl/playground/res/floor.obj");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[MESH LOAD ERROR]: %s", e.what());
        return -1;
    }


    // Load textures
    Texture* cubeTexture = nullptr;
    try {
        cubeTexture = new Texture("/home/oma/Code/CPP-Workspace/ogl/playground/res/tile.bmp");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[TEXTURE LOAD ERROR]: %s", e.what());
        return -1;
    }

    Texture* floorTexture = nullptr;
    try {
        floorTexture = new Texture("/home/oma/Code/CPP-Workspace/ogl/playground/res/tile_bw.bmp");
    }
    catch (LoadException &e) {
        fprintf(stderr, "[TEXTURE LOAD ERROR]: %s", e.what());
        return -1;
    }

    // Create models
    
    auto floorModel = new Model(floorMesh, floorTexture);
    
    auto models = std::vector<Model*>();
    
    models.push_back(floorModel);
    
    for (int i = 0; i < 20; ++i) {
        auto cubeModel = new Model(cubeMesh, cubeTexture);
        cubeModel->setPosition(glm::vec3(-30.0f + i * 3, 1.0f, -1.0f));
        models.push_back(cubeModel);
    }
    

    GLuint mainShaderProgramID = LoadShaders(
        "/home/oma/Code/CPP-Workspace/ogl/playground/vertex-shader.glsl",
        "/home/oma/Code/CPP-Workspace/ogl/playground/fragment-shader.glsl"
    );
    GLuint textureSampler  = glGetUniformLocation(mainShaderProgramID, "myTextureSampler");
    GLuint mvpMatrixID = glGetUniformLocation(mainShaderProgramID, "MVP");
    GLuint viewMatrixID = glGetUniformLocation(mainShaderProgramID, "V");
    GLuint modelMatrixID = glGetUniformLocation(mainShaderProgramID, "M");

    GLuint lightId = glGetUniformLocation(mainShaderProgramID, "LightPosition_worldSpace");
    GLuint lightColorId = glGetUniformLocation(mainShaderProgramID, "LightColor");
    vec3 lightPosition(0, 5, 0);
    double lightTimeCounter = 0;

    /* ================================================ */
    
    GLuint textVAO;
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    GLuint textVBO;
    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
    GLuint textShaderProgramId = LoadShaders(
        "/home/oma/Code/CPP-Workspace/ogl/playground/text-vertex-shader.glsl",
        "/home/oma/Code/CPP-Workspace/ogl/playground/text-fragment-shader.glsl"
    );
    glUseProgram(textShaderProgramId);
    glm::mat4 textProjectionMat = glm::ortho(0.0f, static_cast<float> (width), 0.0f, static_cast<float> (height));
    glUniformMatrix4fv(glGetUniformLocation(textShaderProgramId, "projection"), 1, GL_FALSE, &textProjectionMat[0][0]);//glm::value_ptr(textProjectionMat));
    
    /* ================================================ */
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


    double lastTime = glfwGetTime();
    do {
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);

        /* ==== UPDATE =================================== */

        computeMatricesFromInputs(deltaTime);
        mat4 projection = getProjectionMatrix();
        mat4 view = getViewMatrix();
        
        /* =============================================== */


        /* ==== DRAW ===================================== */

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(mainShaderProgramID);
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &view[0][0]);
        
        lightTimeCounter += deltaTime;
        lightPosition.x = 3 * cos(lightTimeCounter * 2);
        lightPosition.z = 3 * sin(lightTimeCounter * 2);
        //lightPosition.x = 30 * sin(lightTimeCounter * 0.8);
        glUniform3f(lightId, lightPosition.x, lightPosition.y, lightPosition.z);

        vec3 lightColor(1, 1, 1);
        glUniform3f(lightColorId, lightColor.r, lightColor.g, lightColor.b);


        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(textureSampler, 0);

        for (Model* m : models) {
            m->update();
            mat4 model = m->getModelMatrix();
            mat4 mvp = projection * view * model;
            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
            m->draw();
        }
        
        /* ============================================== */
        
        renderText(
            textVAO,
            textVBO,
            textShaderProgramId,
            characters,
            "Hello World!",
            glm::vec2(10.0f, 10.0f),
            1.0f,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        
        /* ============================================== */
        
        
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << "[GL][!] " << gluErrorString(error) << " (" << error << ")" << std::endl;
        }

        glfwSwapBuffers(window);

        /* ============================================== */
        
        glfwPollEvents();

        // For the next frame, the "last time" will be "now"
        lastTime = currentTime;
    }
    while (
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
        && glfwWindowShouldClose(window) == 0
    );


    // == CLEANUP ===================================== //

    for (Model* m : models) {
        delete m;
    }
    delete cubeMesh;
    delete cubeTexture;

    delete floorMesh;
    delete floorTexture;
    
    glDeleteProgram(mainShaderProgramID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

