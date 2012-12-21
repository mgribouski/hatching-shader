
#include "Shaders.h"

static unsigned long getFileLength(std::ifstream &file)
{
    if(!file.good()) return 0;
    
    file.seekg(0, std::ios::end);
    unsigned long len = file.tellg();
    file.seekg(std::ios::beg);
    
    return len;
}

//-----

Shader *Shader::Create(std::string filename)
{
    return Shader::LoadAgnostic((char *)filename.c_str());
}

//-----

Shader::Shader()
{
}

Shader::~Shader()
{
    glDeleteShader(m_object);
    free(m_source);
}

int Shader::Load(char* filename)
{
    m_filename = std::string(filename);
    
    std::cout << "in load";
    
    std::ifstream file;
    std::cout << filename;
	file.open(filename, std::ios::in);
    if (!file)
    {
        char err[512];
        sprintf(err, "Failed to load shader: %s", filename);
        return -1;
    }
    
    unsigned long len = getFileLength(file);
    
    if (len == 0) return -2;
    
    m_source = (char *)malloc(len + 1);
    
    std::cout << " after malloc";
    
    m_source[len] = 0;  // len isn't always strlen because some characters are stripped in ASCII read
    // it is important to null-terminate the real length later; len is just max possible value
    unsigned int i=0;
    while (file.good())
    {
        m_source[i] = file.get();
        if (!file.eof())
            i++;
    }
    
    m_source[i] = 0;
    std::cout << "m_source: ";
    std::cout << m_source[1];
    
    file.close();
    
    return 0;
}

void Shader::Compile()
{
    GLint	length = (GLint) strlen(m_source);
    glShaderSource(m_object, 1, (const GLcharARB **)&m_source, &length);
    
    glCompileShader(m_object);
    
    int compile = 0;
    glGetShaderiv(m_object, GL_COMPILE_STATUS, &compile);
    if (compile == GL_FALSE)
    {
        char log[2048];
        glGetShaderInfoLog(m_object, 2048, NULL, log);
        printf("Failed to compile shader (%s). Log follows:\n%s\n", m_filename.c_str(), log);
    }
}

void Shader::Attach(GLuint program)
{
    glAttachShader(program, m_object);
}

Shader *Shader::LoadAgnostic(char *filename)
{
    char ext[16];
    strcpy(ext, strchr(filename, '.') + sizeof(char));
    
    if (!strcmp(ext, "frag"))
        return new glFragmentShader(filename);
    else if (!strcmp(ext, "vert"))
        return new glVertexShader(filename);
    else if (!strcmp(ext, "geom"))
        return new glGeometryShader(filename);
    else
        return NULL;
}

//-----

glVertexShader::glVertexShader()
{
    m_object = glCreateShader(GL_VERTEX_SHADER);
}

glVertexShader::glVertexShader(char *filename)
{
    m_object = glCreateShader(GL_VERTEX_SHADER);
    std::cout << "in vertexshader create!";
    Load(filename);
    Compile();
}

glVertexShader::~glVertexShader()
{
}

glGeometryShader::glGeometryShader()
{
    m_object = glCreateShader(GL_GEOMETRY_SHADER_EXT);
}

glGeometryShader::glGeometryShader(char *filename)
{
    m_object = glCreateShader(GL_GEOMETRY_SHADER_EXT);
    Load(filename);
    Compile();
}

glGeometryShader::~glGeometryShader()
{
}

glFragmentShader::glFragmentShader()
{
    m_object = glCreateShader(GL_FRAGMENT_SHADER);
}

glFragmentShader::glFragmentShader(char *filename)
{
    m_object = glCreateShader(GL_FRAGMENT_SHADER);
    Load(filename);
    Compile();
}

glFragmentShader::~glFragmentShader()
{
}

//-----

ShaderProgram::ShaderProgram()
{
    m_filename = "";
    m_object = glCreateProgram();
}

ShaderProgram::ShaderProgram(std::string vertex, std::string fragment)
{
    m_filename = "";
    m_object = glCreateProgram();
    
    AddShader(Shader::Create(vertex));
    AddShader(Shader::Create(fragment));
    
    Link();
}

ShaderProgram::ShaderProgram(Shader *vertex, Shader *fragment)
{
    m_filename = "";
    m_object = glCreateProgram();
    
    AddShader(Shader::Create(vertex->m_filename));
    AddShader(Shader::Create(fragment->m_filename));
    
    Link();
}

ShaderProgram::~ShaderProgram()
{
    for (size_t i = 0; i < m_shaders.size(); i++)
    {
        glDetachShader(m_object, m_shaders[i]->m_object);
        delete m_shaders[i];
    }
    m_shaders.clear();
    
    glDeleteProgram(m_object);
}

GLuint ShaderProgram::GetObject()
{
    return m_object;
}

void ShaderProgram::AddShader(std::string filename)
{
    AddShader(Shader::Create(filename));
}

void ShaderProgram::AddShader(Shader *s)
{
    if (m_filename.empty())
        m_filename.append(s->m_filename);
    else
        m_filename.append("," + s->m_filename);
    
    m_shaders.push_back(s);
    s->Attach(m_object);
}

void ShaderProgram::Link()
{
    glUseProgram(0);
    glLinkProgram(m_object);
    
    int link = 0;
    glGetProgramiv(m_object, GL_LINK_STATUS, &link);
    if (link == GL_FALSE)
    {
        char log[2048];
        glGetProgramInfoLog(m_object, 2048, NULL, log);
        printf("Failed to link shader program (%s). Log follows:\n%s\n", m_filename.c_str(), log);
    }
}

void ShaderProgram::Use()
{
    glUseProgram(m_object);
    char log[2048];
    glGetProgramInfoLog(m_object, 2048, NULL, log);
    printf(log);
    
}

void ShaderProgram::setUniform1f(GLuint u, GLfloat value)
{
    glUniform1f(u, value);
}

void ShaderProgram::setUniform3f(GLuint u, GLfloat val1, GLfloat val2, GLfloat val3)
{
    glUniform3f(u, val1, val2, val3);
}

void ShaderProgram::setUniform4f(GLuint u, GLfloat val1, GLfloat val2, GLfloat val3, GLfloat val4)
{
    glUniform4f(u, val1, val2, val3, val4);
}

void ShaderProgram::setUniform4fv(GLuint u, GLsizei count, GLfloat *value)
{
    glUniform4fv(u, count, value);
}

void ShaderProgram::setUniform1i(GLuint u, GLuint value)
{
    glUniform1i(u, value);
}

void ShaderProgram::setUniformMatrix4fv(GLuint u, GLsizei count, GLboolean transpose, GLfloat *value)
{
    glUniformMatrix4fv(u, count, transpose, value);
}

GLuint ShaderProgram::getUniform(std::string name)
{
    return glGetUniformLocation(m_object, name.c_str());
}

GLuint ShaderProgram::getAttribute(std::string name)
{
    return glGetAttribLocation(m_object, name.c_str());
}
