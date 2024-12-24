#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	unsigned int programID;

	Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		std::string vertexStr;
		std::string fragmentStr;

		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			// reading files
			vShaderFile.open(vertexShaderPath);
			fShaderFile.open(fragmentShaderPath);

			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			// converting streams into strings
			vertexStr = vShaderStream.str();
			fragmentStr = fShaderStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
		}

		const char* vertexCode = vertexStr.c_str();
		const char* fragmentCode = fragmentStr.c_str();

		// compiling shaders
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexCode, NULL);
		glCompileShader(vertexShader);
		checkErrors(vertexShader, "VERTEX");
		

		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
		glCompileShader(fragmentShader);
		checkErrors(vertexShader, "FRAGMENT");

		// creating a shader program
		programID = glCreateProgram();
		glAttachShader(programID, vertexShader);
		glAttachShader(programID, fragmentShader);
		glLinkProgram(programID);
		checkErrors(programID, "SHADER_PROGRAM");

		// deleting shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void use()
	{
		glUseProgram(programID);
	}

	void setUniformF(const std::string& name, float value)
	{
		glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
	}
	void setUniformI(const std::string& name, int value)
	{
		glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
	}
	void setUniformB(const std::string& name, bool value) 
	{
		glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
	}
	void setUniformVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
	}
	void setUniformVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
	}
	void setUniformMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
	}

private:
	void checkErrors(GLuint shader, std::string type)
	{
		int success;
		char infoLog[512];

		if (type == "SHADER_PROGRAM")
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" <<
					infoLog << std::endl;
			}
		}
		else
		{
			
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" <<
					infoLog << std::endl;
			}
		}
	}
};

#endif 
