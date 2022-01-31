#include <exe/rendering/shaders.h>
#include <glad/glad.h>
#include <iostream>

using GBEmulatorExe::Shader;

Shader::Shader(const char* vertexPrg, const char* fragmentPrg)
{
    CreateShaders(vertexPrg, fragmentPrg);
}

Shader::~Shader()
{
    if (m_programId != 0)
        glDeleteProgram(m_programId);

    m_programId = 0;
}

void Shader::CreateShaders(const char* vertexPrg, const char* fragmentPrg)
{
    unsigned vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexPrg, nullptr);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        m_programId = 0;
        glDeleteShader(vertex);
        return;
    };
    
    // similiar for Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentPrg, nullptr);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        m_programId = 0;
        return;
    };
    
    // shader Program
    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertex);
    glAttachShader(m_programId, fragment);
    glLinkProgram(m_programId);
    // print linking errors if any
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(m_programId);
        m_programId = 0;
        return;
    }
    
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::UseProgram()
{
    glUseProgram(m_programId);
}