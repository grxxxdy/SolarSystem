#ifndef PLANET_H
#define PLANET_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>

#include "Figures.h"
#include "Shader.h"
#include "Texture.h"

class Planet {
public:

	Planet(float radius, int sectorCount, int stackCount, float sunDistance, float sunRotationSpeed, float selfRotationSpeed, 
		const char* vertexShaderPath, const char* fragmentShaderPath, 
		const char* texturePath, const char* cloudTexturePath = nullptr, 
		const char* orbitTexturePath = nullptr, int orbitDencity = 0,
		bool isLightSource = false)
		: body(radius, sectorCount, stackCount),
		shaderProgram(vertexShaderPath, fragmentShaderPath),
		sunOrbit(sunDistance, 0.02f, 64, 64),
		sunOrbitShaderProgram("ShaderData/SunOrbits/vertex_shader.txt", "ShaderData/SunOrbits/fragment_shader.txt")
	{
		setupBody(texturePath);

		if (orbitTexturePath != nullptr) 
			setupOrbit(orbitTexturePath, 0.05f, radius + 1.0f, 64, 64, orbitDencity);

		if (cloudTexturePath != nullptr)
		{
			setupClouds(cloudTexturePath);
			hasClouds = true;
		}
		else
			hasClouds = false;

		setupSunOrbit();

		isLight = isLightSource;

		distanceFromSun = sunDistance;
		rotationAroundSunSpeed = sunRotationSpeed;
		rotationAroundSelfSpeed = selfRotationSpeed;
	}

	void updatePos(float deltaTime, float parentDistance = -1)
	{
		modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, rotationAroundSunSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 moonPos;

		if (parentDistance == -1)
		{
			moonPos = glm::vec3(distanceFromSun, 0.0f, 0.0f);
		}
		else
		{
			glm::vec3 parentPos = glm::vec3(parentDistance, 0.0f, 0.0f);
			moonPos = parentPos + glm::vec3(1.0f * cos(0.5f * deltaTime), 1.0f * sin(0.5f * deltaTime), 0.0f);
		}

		modelMatrix = glm::translate(modelMatrix, moonPos);

		modelMatrix = glm::rotate(modelMatrix, rotationAroundSelfSpeed * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void render(glm::mat4 view, glm::mat4 projection, glm::vec3 lightPos, glm::vec3 lightColor, glm::vec3 viewPos, bool visibleOrbits)
	{
		// Planet
		shaderProgram.use();
		shaderProgram.setUniformMat4("model", modelMatrix);
		shaderProgram.setUniformMat4("view", view);
		shaderProgram.setUniformMat4("projection", projection);

		shaderProgram.setUniformVec3("lightPos", lightPos);
		shaderProgram.setUniformVec3("lightColor", lightColor);
		shaderProgram.setUniformVec3("viewPos", viewPos);
		shaderProgram.setUniformB("isLightSource", isLight);

		shaderProgram.setUniformB("hasClouds", hasClouds);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		shaderProgram.setUniformI("textureToSet", 0);

		if (hasClouds) 
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, cloudTextureID);
			shaderProgram.setUniformI("cloudTexture", 1);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, body.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Orbit, if the planet has one
		if (orbitVAO.size() != 0)
		{
			Shader orbitShader("ShaderData/SunOrbits/vertex_shader.txt", "ShaderData/SunOrbits/fragment_shader.txt");
			orbitShader.use();

			for (int i = 0; i < orbitVAO.size(); i++)
			{
				orbitShader.setUniformMat4("model", modelMatrix);
				orbitShader.setUniformMat4("view", view);
				orbitShader.setUniformMat4("projection", projection);

				orbitShader.setUniformVec4("colorToSet", orbitColors[i]);
				orbitShader.setUniformVec3("lightPos", lightPos);
				orbitShader.setUniformVec3("lightColor", lightColor);
				orbitShader.setUniformVec3("viewPos", viewPos);
				orbitShader.setUniformB("ignoreLights", false);

				glBindVertexArray(orbitVAO[i]);
				glDrawElements(GL_TRIANGLES, orbitIndices[i].size(), GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
		}

		// Sun orbits
		if (visibleOrbits)
		{
			glm::mat4 torusModel = glm::mat4(1.0f);
			torusModel = glm::rotate(torusModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			torusModel = glm::translate(torusModel, glm::vec3(0.0f, 0.0f, 0.0f));

			sunOrbitShaderProgram.use();
			sunOrbitShaderProgram.setUniformMat4("model", torusModel);
			sunOrbitShaderProgram.setUniformMat4("view", view);
			sunOrbitShaderProgram.setUniformMat4("projection", projection);

			sunOrbitShaderProgram.setUniformVec4("colorToSet", glm::vec4(1.0f));
			sunOrbitShaderProgram.setUniformVec3("lightPos", lightPos);
			sunOrbitShaderProgram.setUniformVec3("lightColor", lightColor);
			sunOrbitShaderProgram.setUniformVec3("viewPos", viewPos);
			sunOrbitShaderProgram.setUniformB("ignoreLights", true);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glBindVertexArray(sunOrbitVAO);
			glDrawElements(GL_TRIANGLES, sunOrbit.indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	float getDistanceFromSun()
	{
		return distanceFromSun;
	}

	~Planet() 
	{
		glDeleteVertexArrays(1, &sphereVAO);
		glDeleteBuffers(1, &sphereVBO);
		glDeleteBuffers(1, &sphereEBO);

		if (orbitVAO.size() != 0) {
			for (int i = 0; i < orbitVAO.size(); i++)
			{
				glDeleteVertexArrays(1, &orbitVAO[i]);
				glDeleteBuffers(1, &orbitVBO[i]);
				glDeleteBuffers(1, &orbitEBO[i]);
			}
		}

		glDeleteVertexArrays(1, &sunOrbitVAO);
		glDeleteBuffers(1, &sunOrbitVBO);
		glDeleteBuffers(1, &sunOrbitEBO);
	}


private:
	Sphere body;
	Torus sunOrbit;

	Shader shaderProgram;
	Shader sunOrbitShaderProgram;

	unsigned int sphereVBO, sphereVAO, sphereEBO;
	std::vector<unsigned int> orbitVBO, orbitVAO, orbitEBO;
	unsigned int sunOrbitVBO, sunOrbitVAO, sunOrbitEBO;

	unsigned int textureID, orbitTextureID = 0, cloudTextureID = 0;

	std::vector<std::vector<unsigned int>> orbitIndices;

	bool isLight, hasClouds;

	glm::mat4 modelMatrix;

	float distanceFromSun;
	float rotationAroundSunSpeed;
	float rotationAroundSelfSpeed;

	std::vector<glm::vec4> orbitColors;


	void setupBody(const char* texturePath)
	{
		glGenVertexArrays(1, &sphereVAO);
		glGenBuffers(1, &sphereVBO);
		glGenBuffers(1, &sphereEBO);

		glBindVertexArray(sphereVAO);

		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, body.vertices.size() * sizeof(float), &body.vertices[0], GL_STATIC_DRAW);

		// Element buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, body.indices.size() * sizeof(unsigned int), &body.indices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// Texture coordinate attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		Texture::loadTexture(texturePath, textureID);
	}

	void setupClouds(const char* texturePath)
	{
		Texture::loadTexture(texturePath, cloudTextureID);
	}

	void setupOrbit(const char* orbitTexturePath, float innerRadius, float outerRadius, int numSides, int numRings, int ringsCount)
	{
		for (int i = 0; i < ringsCount; i++)
		{
			Torus ring(outerRadius, innerRadius, numSides, numRings);

			unsigned int VAO, VBO, EBO;

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, ring.vertices.size() * sizeof(float), &ring.vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ring.indices.size() * sizeof(unsigned int), &ring.indices[0], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);

			orbitVAO.push_back(VAO);
			orbitVBO.push_back(VBO);
			orbitEBO.push_back(EBO);

			orbitIndices.push_back(ring.indices);

			outerRadius += innerRadius + 0.05f;
		}

		orbitColors = Texture::sampleTextureColors(orbitTexturePath, ringsCount);
	}

	void setupSunOrbit() 
	{
		glGenVertexArrays(1, &sunOrbitVAO);
		glGenBuffers(1, &sunOrbitVBO);
		glGenBuffers(1, &sunOrbitEBO);

		glBindVertexArray(sunOrbitVAO);

		glBindBuffer(GL_ARRAY_BUFFER, sunOrbitVBO);
		glBufferData(GL_ARRAY_BUFFER, sunOrbit.vertices.size() * sizeof(float), &sunOrbit.vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunOrbitEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sunOrbit.indices.size() * sizeof(unsigned int), &sunOrbit.indices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
};

#endif
