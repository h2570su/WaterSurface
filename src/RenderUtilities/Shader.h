#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>



class Shader
{
public:
	GLuint Program;
	enum Type {
		NULL_SHADER = (0),
		VERTEX_SHADER = (1 << 0),
		TESS_CONTROL_SHADER = (1 << 1),
		TESS_EVALUATION_SHADER = (1 << 2),
		GEOMETRY_SHADER = (1 << 3),
		FRAGMENT_SHADER = (1 << 4),
	};
	//DEFINE_ENUM_FLAG_OPERATORS(Type);

	Type type = NULL_SHADER;
	// Constructor generates the shader on the fly
	Shader(const GLchar* vert, const GLchar* tesc, const GLchar* tese, const char* geom, const char* frag)
	{
		std::vector<GLuint> shaders;
		if (vert)
		{
			shaders.push_back(this->compileShader(GL_VERTEX_SHADER, this->readCode(vert).c_str()));
			this->type = (Shader::Type)(this->type | Type::VERTEX_SHADER);
		}
		if (tesc)
		{
			shaders.push_back(this->compileShader(GL_TESS_CONTROL_SHADER, this->readCode(tesc).c_str()));
			this->type = (Shader::Type)(this->type | Type::TESS_CONTROL_SHADER);
		}
		if (tese)
		{
			shaders.push_back(this->compileShader(GL_TESS_EVALUATION_SHADER, this->readCode(tese).c_str()));
			this->type = (Shader::Type)(this->type | Type::TESS_EVALUATION_SHADER);
		}
		if (geom)
		{
			shaders.push_back(this->compileShader(GL_GEOMETRY_SHADER, this->readCode(geom).c_str()));
			this->type = (Shader::Type)(this->type | Type::GEOMETRY_SHADER);
		}
		if (frag)
		{
			shaders.push_back(this->compileShader(GL_FRAGMENT_SHADER, this->readCode(frag).c_str()));
			this->type = (Shader::Type)(this->type | Type::FRAGMENT_SHADER);
		}
		// Shader Program
		GLint success;
		GLchar infoLog[512];
		this->Program = glCreateProgram();

		for (GLuint shader : shaders)
			glAttachShader(this->Program, shader);

		glLinkProgram(this->Program);
		// Print linking errors if any
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		for (GLuint shader : shaders)
			glDeleteShader(shader);
	}
	// Uses the current shader
	void Use()
	{
		glUseProgram(this->Program);
	}
	//From learnopengl.com
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(this->Program, name.c_str()), (int)value);
	}
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(this->Program, name.c_str()), value);
	}
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(this->Program, name.c_str()), value);
	}
	 void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(this->Program, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(this->Program, name.c_str()), x, y); 
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(this->Program, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(this->Program, name.c_str()), x, y, z); 
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(this->Program, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) 
    { 
        glUniform4f(glGetUniformLocation(this->Program, name.c_str()), x, y, z, w); 
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(this->Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(this->Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(this->Program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
	//End
private:
	std::string readCode(const GLchar* path)
	{
		std::string code;
		std::ifstream shader_file;
		// ensures ifstream objects can throw exceptions:
		shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// Open files
			shader_file.open(path);
			//if (!vShaderFile)std::cout << vertexPath << " fail to open" << std::endl;
			std::stringstream shader_stream;
			// Read file's buffer contents into streams
			shader_stream << shader_file.rdbuf();
			// close file handlers
			shader_file.close();
			// Convert stream into string
			code = shader_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			std::cout << path << std::endl;
		}
		return code;
	}
	GLuint compileShader(GLenum shader_type, const char* code)
	{
		GLuint shader_number;
		GLint success;
		GLchar infoLog[512];
		// Vertex Shader
		shader_number = glCreateShader(shader_type);
		glShaderSource(shader_number, 1, &code, NULL);
		glCompileShader(shader_number);
		// Print compile errors if any
		glGetShaderiv(shader_number, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader_number, 512, NULL, infoLog);
			if (shader_type == GL_VERTEX_SHADER)
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if (shader_type == GL_TESS_CONTROL_SHADER)
				std::cout << "ERROR::SHADER::TESS_CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if (shader_type == GL_TESS_EVALUATION_SHADER)
				std::cout << "ERROR::SHADER::TESS_EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if (shader_type == GL_GEOMETRY_SHADER)
				std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
			else if (shader_type == GL_FRAGMENT_SHADER)
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		return shader_number;
	}
};

#endif