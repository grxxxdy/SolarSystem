#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture {
public:
	static void loadTexture(const char* path, unsigned int &textureID)
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Set texture wrapping and filtering options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load image using stb_image
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		if (data)
		{
			// Determine image format
			GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;

			// Load texture data into OpenGL
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cerr << "Failed to load texture at path: " << path << std::endl;
		}
		stbi_image_free(data);
	}

	static unsigned int loadCubemap(std::vector<std::string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height,
				&nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
					width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap failed to load at path: " << faces[i]
					<< std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
			GL_CLAMP_TO_EDGE);
		return textureID;

	}

	static std::vector<glm::vec4> sampleTextureColors(const char* texturePath, int numSegments)
	{
		int width, height, channels;
		unsigned char* data = stbi_load(texturePath, &width, &height, &channels, 0);
		if (!data)
		{
			throw std::runtime_error("Failed to load texture.");
		}

		std::vector<glm::vec4> colors(numSegments);
		for (int i = 0; i < numSegments; ++i)
		{
			// Calculate sampling position
			float u = ((float)i) / (float)numSegments;       // Fraction of the texture
			int x = static_cast<int>(u * width);          // Corresponding pixel x-coordinate

			// Read pixel data
			int index = (x * channels);                   // Index in texture data
			float r = data[index + 0] / 255.0f;           // Red channel
			float g = data[index + 1] / 255.0f;           // Green channel
			float b = data[index + 2] / 255.0f;           // Blue channel
			float a = (channels > 3 ? data[index + 3] : 255) / 255.0f; // Alpha channel

			colors[i] = glm::vec4(r, g, b, a);
		}

		stbi_image_free(data);

		/*for (const auto& color : colors) {
			std::cout << "Color: " << color.r << ", " << color.g << ", " << color.b << ", " << color.a << "\n";
		}*/

		return colors;
	}
};

#endif