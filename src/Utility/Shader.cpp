#include "Shader.h"

#include "glew.h"

#include <iostream>

Utility::Shader::Shader()
{
}

Utility::Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	BuildFiles(vertexPath, fragmentPath);
}

Utility::Shader::~Shader()
{
	glDeleteProgram(id);
}

bool Utility::Shader::BuildFiles(const char* vertexPath, const char* fragmentPath)
{
	// Get shader code from file

	std::string vShader = ReadFromFile(vertexPath);
	std::string fShader = ReadFromFile(fragmentPath);

	const char* vertexCode = vShader.c_str();
	const char* fragmentCode = fShader.c_str();

	// Compile shaders

	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexCode, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR :: VERTEX COMPILATION FAILED\n" << infoLog << std::endl;
		return false;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR :: FRAGMENT COMPILATION FAILED\n" << infoLog << std::endl;
		return false;
	}

	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "ERROR :: PROGRAM COMPILATION FAILED\n" << infoLog << std::endl;
		return false;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return true;

}

void Utility::Shader::Bind()
{
	glUseProgram(id);
}

std::string Utility::ReadFromFile(const char* filePath)
{
	std::string contents;

	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		file.open(filePath);

		std::stringstream fileStream;

		fileStream << file.rdbuf();

		file.close();

		contents = fileStream.str();

	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR FILE " << filePath << " NOT READ : " << std::endl;
	}

	return contents;
}
