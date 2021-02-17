//
// Created by FoLoKe on 05.02.2021.
//

#include <jni.h>
#include <string>
#include <android/log.h>
#include "external/stb_image.h"
#include <android/asset_manager_jni.h>
#include <cmath>
#include <sstream>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/OBJ_Loader.h"

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_foloke_gltest_MainActivity_test(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

GLuint gProgram;
GLuint gvPositionHandle;

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

auto gVertexShader =
        "#version 300 es\n"
        "layout (location=0) in vec3 aPos;\n"
        "out vec4 vertexColor;\n"

        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"

        "void main()\n"
        "{\n"
            "gl_Position = projection * view * model * vec4(aPos,1.0f); \n"
        "}";


auto gFragmentShader =

        "#version 300 es\n"
        "precision highp float;"
        "out vec4 FragColor;\n"

        "uniform vec4 uniformColor;\n"

        "void main()\n"
        "{\n"
            "FragColor = uniformColor;\n"
        "}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) { // Load and Bind Fragments to Progam and link program then return result
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!fragmentShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, nullptr, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}



bool setupGraphics(int w, int h) {
    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    //gvPositionHandle = glGetAttribLocation(gProgram, "aPos");

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;
}

GLfloat vertices[] = {
        0.5f,  0.5f, 0.0f,  // верхняя правая
        0.5f, -0.5f, 0.0f,  // нижняя правая
        -0.5f, -0.5f, 0.0f,  // нижняя левая
        -0.5f,  0.5f, 0.0f   // верхняя левая
};

GLuint indices[] = {
       0, 1, 3,
       1, 2, 3
};

GLuint VAO;

void initBuffers()
{
    //GENERATING
    GLuint VBO;
    GLuint EBO;

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    //BINDING
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    //UNBINDING
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void getModel(AAssetManager *mgr)
{

//    glGenTextures(1 , &mTexture);
//    glBindTexture(GL_TEXTURE_2D, mTexture);// Bind our 2D texture so that following set up will be applied
//
//    //Set texture wrapping parameter
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
//
//    //Set texture Filtering parameter
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
//
//    //Load the image
//    int picWidth = 128,picHeight = 64;

    //FUCK IT! JUST USE LIBRARY FOR IMPORT OR PROPER 3D ENGINE
    if(mgr == nullptr)
        LOGE("MyNative %s", "AAssetManager NULL");
    AAsset * asset = AAssetManager_open(mgr, "test.obj", AASSET_MODE_STREAMING);

    if (asset == nullptr)
        LOGE("_ASSET_NOT_FOUND_ %s", "explosion.png");

    const char* buffer = (const char*)AAsset_getBuffer(asset);
    char * token = strtok(strdup(buffer), "\n");

    std::vector<glm::vec3> verts;

    while(token) {

        //std::string str;
        //str = line;
        LOGI("%s", token);
        char * line = (char *)malloc((strlen(token) + 1));
        strcpy(line, token);
        std::string str(line);
        if(str.substr(0, 2) == "v ") {
            std::istringstream v(str.substr(2));
            glm::vec3  vert;
            double x, y, z;
            v>>x;
            v>>y;
            v>>z;
            vert = glm::vec3(x,y,z);
            LOGI("%f", vert.x);
            verts.push_back(vert);
        }

        token  = strtok(nullptr, "\n");
    }
    //printf("%s", buf);
//
//    //Generate the image
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , picWidth , picHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
//    glGenerateMipmap(GL_TEXTURE_2D);
//
//    glBindTexture(GL_TEXTURE_2D,0); //Unbind 2D textures

}

float color = 0;

void renderFrame() {
    color = color + 0.01f;
    if(color > 1) {
        color = 0;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-55.0f), glm::vec3(1.0, 0.0, 0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5, 0.5, 0.5));

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glClearColor(1, 1-0.03f, 1, 1);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GLint colorLocation = glGetUniformLocation(gProgram, "uniformColor");
    GLint modelLocation = glGetUniformLocation(gProgram, "model");
    GLint viewLocation = glGetUniformLocation(gProgram, "view");
    GLint projectionLocation = glGetUniformLocation(gProgram, "projection");

    glEnable(GL_DEPTH_TEST);
    glUseProgram(gProgram);

    glUniform4f(colorLocation, 0, color, 0, 1);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    checkGlError("render");
    glBindVertexArray(0);
}



extern "C" JNIEXPORT void JNICALL Java_com_foloke_gltest_MainActivity_init(JNIEnv * env, jclass obj,  jint width, jint height)
{
    setupGraphics(width, height);
    initBuffers();
}

extern "C" JNIEXPORT void JNICALL Java_com_foloke_gltest_MainActivity_step(JNIEnv* env,
                                                                           jclass clazz)
{
    renderFrame();
}

extern "C" JNIEXPORT void JNICALL Java_com_foloke_gltest_MainActivity_load(JNIEnv* env, jclass clazz, jobject assetManager) {
    getModel(AAssetManager_fromJava(env, assetManager));
}

