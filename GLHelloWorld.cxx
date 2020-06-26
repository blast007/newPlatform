#include "GLHelloWorld.h"

#include <string.h>

// Based on https://github.com/cmcsun/esshader

/*
MIT License

esshader

Copyright (c) 2016 Christopher McFee <cmcfee@cryptolab.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

GLHelloWorld::GLHelloWorld(const char* filename, int width, int height)
{

    const char* fragShader = readShader(filename);

    const GLchar* vertexSource = R"glsl(
		#version 100
		precision highp float;

		attribute vec4 iPosition;
		void main(){
			gl_Position = iPosition;
		}
	)glsl";

    const GLchar* fragmentTemplate = R"glsl(
		#version 100
		precision highp float;

		uniform vec3 iResolution;
		uniform float iTime;
		uniform float iChannelTime[4];
		uniform vec4 iMouse;
		uniform vec4 iDate;
		uniform float iSampleRate;
		uniform vec3 iChannelResolution[4];
		uniform sampler2D iChannel0;
		uniform sampler2D iChannel1;
		uniform sampler2D iChannel2;
		uniform sampler2D iChannel3;

		%s

		void main() {
			mainImage(gl_FragColor, gl_FragCoord.xy);
		}
	)glsl";

    GLchar* fragmentSource = (GLchar*)malloc(strlen(fragmentTemplate) + strlen(fragShader) + 1);

    sprintf(fragmentSource, fragmentTemplate, fragShader);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    GLint success;

    vtx = compileShader(GL_VERTEX_SHADER, vertexSource);
    frag = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vtx);
    glAttachShader(shader_program, frag);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
        exit(-4);

    glDeleteShader(vtx);
    glDeleteShader(frag);
    glReleaseShaderCompiler();

    glUseProgram(shader_program);
    glValidateProgram(shader_program);

    attrib_position = glGetAttribLocation(shader_program, "iPosition");
    sampler_channel[0] = glGetUniformLocation(shader_program, "iChannel0");
    sampler_channel[1] = glGetUniformLocation(shader_program, "iChannel1");
    sampler_channel[2] = glGetUniformLocation(shader_program, "iChannel2");
    sampler_channel[3] = glGetUniformLocation(shader_program, "iChannel3");
    uniform_cres = glGetUniformLocation(shader_program, "iChannelResolution");
    uniform_ctime = glGetUniformLocation(shader_program, "iChannelTime");
    uniform_date = glGetUniformLocation(shader_program, "iDate");
    uniform_time = glGetUniformLocation(shader_program, "iTime");
    uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
    uniform_res = glGetUniformLocation(shader_program, "iResolution");
    uniform_srate = glGetUniformLocation(shader_program, "iSampleRate");

    resize(width, height);
}

GLHelloWorld::~GLHelloWorld()
{
}

char* GLHelloWorld::readFile(const char *filename)
{
    long length = 0;
    char *result = nullptr;
    FILE *file = fopen(filename, "r");
    if (file)
    {
        int status = fseek(file, 0, SEEK_END);
        if (status != 0)
        {
            fclose(file);
            return nullptr;
        }
        length = ftell(file);
        status = fseek(file, 0, SEEK_SET);
        if (status != 0)
        {
            fclose(file);
            return nullptr;
        }
        result = (char*)malloc((length + 1) * sizeof(char));
        if (result)
        {
            size_t actual_length = fread(result, sizeof(char), length, file);
            result[actual_length++] = '\0';
        }
        fclose(file);
        return result;
    }
    return nullptr;
}

char* GLHelloWorld::readShader(const char *filename)
{
    std::string path = "shaders/" + std::string(filename);
    char* shader = readFile(path.c_str());
    if (shader == nullptr)
    {
        path = "../" + path;
        shader = readFile(path.c_str());
        if (shader == nullptr)
        {
            path = "../" + path;
            shader = readFile(path.c_str());
            if (shader == nullptr)
                exit(-10);
        }
    }
    return shader;
}

GLuint GLHelloWorld::compileShader(GLenum type, const char* source)
{
    GLuint shader;
    GLint success, len;
    char *log;


    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1)
        {
            log = (char*)malloc(len);
            glGetShaderInfoLog(shader, len, nullptr, log);
            fprintf(stderr, "\n\n%s\n\n", log);
            free(log);
        }
        exit(-5);
    }

    return shader;
}

void GLHelloWorld::resize(int width, int height)
{
    glUniform3f(uniform_res, (float)width, (float)height, 0.0f);
    glViewport(0, 0, width, height);
    setPosition(width / 2, height / 2, width / 2, height / 2);
}

void GLHelloWorld::setPosition(double curX, double curY, double clickX, double clickY)
{
    glUniform4f(uniform_mouse, (float)curX, (float)curY, (float)clickX, (float)clickY);
}

void GLHelloWorld::drawFrame(double abstime)
{
    static const GLfloat vertices[] =
    {
        -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
        };

    if (uniform_time >= 0)
        glUniform1f(uniform_time, abstime);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
