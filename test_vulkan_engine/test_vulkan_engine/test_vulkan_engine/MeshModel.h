#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>

#include "Mesh.h"

class MeshModel
{
public:
	MeshModel();
	MeshModel(std::vector<Mesh> newMeshList);

	size_t getMeshCount();
	Mesh* getMesh(size_t index);

	glm::mat4 getModel();
	glm::mat4* getModelRef();

	void setModel(glm::mat4 newModel);

	void destroyMeshModel();

	void keyControl(bool* keys, float deltaTime, float angleSpeed);

	static std::vector<std::string> LoadMaterials(const aiScene* scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiNode* node, const aiScene* scene, std::vector<int> matToTex);
	static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);

	~MeshModel();

private:
	std::vector<Mesh> meshList;
	glm::mat4 model;


	float angleY = 0.0f; // Y tengely körüli forgatás
	float angleX = 0.0f; // X tengely körüli forgatás
	glm::mat4 modelMatrix = glm::mat4(1.0f); // Modell mátrix
};

