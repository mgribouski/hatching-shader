
#ifndef SHADERS_H
#define SHADERS_H

#include <OpenGL/GL.h>

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

// Basically, create a shader program by calling
// ShaderProgram(vert_shader_filename, frag_shader_filename)
// and then when you want it to be active, call Use()
// on it. To go back to the default shader, call
// glUseProgram(0).

class Shader
{
protected:
    GLuint   m_object;
    char    *m_source;
    
public:
    Shader();
    virtual ~Shader();
    
    std::string m_filename;
    
    int Load(char *filename);
    void Compile();
    void Attach(GLuint program);
    
    static Shader *Create(std::string filename);
    
    static Shader *LoadAgnostic(char *filename);
    
    friend class ShaderProgram;
};

class glVertexShader: public Shader
{
public:
    glVertexShader();
    glVertexShader(char *filename);
    ~glVertexShader();
};

class glGeometryShader: public Shader
{
public:
    glGeometryShader();
    glGeometryShader(char *filename);
    ~glGeometryShader();
};

class glFragmentShader: public Shader
{
public:
    glFragmentShader();
    glFragmentShader(char *filename);
    ~glFragmentShader();
};

//-----

class ShaderProgram
{
protected:
    std::vector<Shader *> m_shaders;
    
    GLuint m_object;
    
public:
    ShaderProgram();
    ShaderProgram(std::string vertex, std::string fragment);
    ShaderProgram(Shader *vertex, Shader *fragment);
    ~ShaderProgram();
    
    std::string m_filename;
    
    GLuint GetObject();
    void AddShader(std::string filename);
    void AddShader(Shader *s);
    void Link();
    void Use();
    
    void setUniform1f(GLuint u, GLfloat value);
    void setUniform3f(GLuint u, GLfloat val1, GLfloat val2, GLfloat val3);
    void setUniform4f(GLuint u, GLfloat val1, GLfloat val2, GLfloat val3, GLfloat val4);
    void setUniform4fv(GLuint u, GLsizei count, GLfloat *value);
    void setUniform1i(GLuint u, GLuint value);
    void setUniformMatrix4fv(GLuint u, GLsizei count, GLboolean transpose, GLfloat *value);
    
    GLuint getUniform(std::string name);
    GLuint getAttribute(std::string name);
};

#endif
