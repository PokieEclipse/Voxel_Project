#pragma once
#include <fstream>
#include <string>
#include "sstream"

namespace Utility
{

	class Shader {
	public:

		Shader();

		Shader(const char* vertexPath, const char* fragmentPath);
		~Shader();

		bool BuildFiles(const char* vertexPath, const char* fragmentPath);

		unsigned int GetID() const { return id; }

		void Bind();

	private:
		unsigned int id;

	};

	std::string ReadFromFile(const char* filePath);

}