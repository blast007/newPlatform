#pragma once

#include <GL/glew.h>
#include "BzfPlatform.h"

class GLHelloWorld
{
public:
    GLHelloWorld(const char* fragShader, int width, int height);
    ~GLHelloWorld();
    char* readFile(const char *filename);
    char* readShader(const char *filename);
    GLuint compileShader(GLenum type, const char* source);
    void resize(int width, int height);
    void setPosition(double curX, double curY, double clickX, double clickY);
    void drawFrame(double abstime);
private:
    GLuint vtx, frag;

    GLuint shader_program;
    GLint attrib_position;
    GLint sampler_channel[4];
    GLint uniform_cres;
    GLint uniform_ctime;
    GLint uniform_date;
    GLint uniform_time;
    GLint uniform_mouse;
    GLint uniform_res;
    GLint uniform_srate;
};
