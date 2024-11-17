#include "MeshModel.h"



MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> newMeshList, bool isControlable, glm::vec3 startPos)
{
	meshList = newMeshList;
	model = glm::mat4(1.0f);
	controlable = isControlable;

	modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, startPos);
	position = startPos;
	this->setModel(modelMatrix);
}

size_t MeshModel::getMeshCount()
{
	return meshList.size();
}

Mesh* MeshModel::getMesh(size_t index)
{
	if (index >= meshList.size())
	{
		throw std::runtime_error("Attempted to access invalid Mesh index!");
	}

	return &meshList[index];
}

glm::mat4 MeshModel::getModel()
{
	return model;
}

glm::mat4* MeshModel::getModelRef()
{
	return &model;
}

bool MeshModel::getControlable()
{
	return this->controlable;
}

void MeshModel::setModel(glm::mat4 newModel)
{
	model = newModel;
}

void MeshModel::destroyMeshModel()
{
	for (auto& mesh : meshList)
	{
		mesh.destroyBuffers();
	}
}


std::vector<std::string> MeshModel::LoadMaterials(const aiScene* scene)
{
	// Create 1:1 sized list of textures
	std::vector<std::string> textureList(scene->mNumMaterials);

	// Go through each material and copy its texture file name (if it exists)
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		// Get the material
		aiMaterial* material = scene->mMaterials[i];

		// Initialise the texture to empty string (will be replaced if texture exists)
		textureList[i] = "";

		// Check for a Diffuse Texture (standard detail texture)
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			// Get the path of the texture file
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				// Cut off any directory information aleady present
				int idx = std::string(path.data).rfind("\\");
				std::string fileName = std::string(path.data).substr(idx + 1);

				textureList[i] = fileName;
			}
		}
	}

	return textureList;
}

std::vector<Mesh> MeshModel::LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiNode* node, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<Mesh> meshList;

	// Go through each mesh at this node and create it, then add it to our meshList
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		meshList.push_back(
			LoadMesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, scene->mMeshes[node->mMeshes[i]], scene, matToTex)
		);
	}

	// Go through each node attached to this node and load it, then append their meshes to this node's mesh list
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		std::vector<Mesh> newList = LoadNode(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, node->mChildren[i], scene, matToTex);
		meshList.insert(meshList.end(), newList.begin(), newList.end());
	}

	return meshList;
}

Mesh MeshModel::LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// Resize vertex list to hold all vertices for mesh
	vertices.resize(mesh->mNumVertices);

	// Go through each vertex and copy it across to our vertices
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Set position
		vertices[i].pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		// Set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].tex = { 0.0f, 0.0f };
		}

		// Set colour (just use white for now)
		vertices[i].col = { 1.0f, 1.0f, 1.0f };
	}

	// Iterate over indices through faces and copy across
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// Get a face
		aiFace face = mesh->mFaces[i];

		// Go through face's indices and add to list
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create new mesh with details and return it
	Mesh newMesh = Mesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, &vertices, &indices, matToTex[mesh->mMaterialIndex]);

	return newMesh;
}

void MeshModel::keyControl(bool* keys, float deltaTime, float moveSpeed, float angleSpeed)
{
    if (this->controlable)
    {
        // Mozg�s jobbra/balra (X tengely ment�n)
        if (keys[GLFW_KEY_LEFT]) {
            position.x -= moveSpeed * deltaTime;  // Mozg�s balra
        }
        if (keys[GLFW_KEY_RIGHT]) {
            position.x += moveSpeed * deltaTime;  // Mozg�s jobbra
        }

        // Mozg�s fel/le (Y tengely ment�n)
        if (keys[GLFW_KEY_UP]) {
            position.y += moveSpeed * deltaTime;  // Mozg�s felfel�
        }
        if (keys[GLFW_KEY_DOWN]) {
            position.y -= moveSpeed * deltaTime;  // Mozg�s lefel�
        }

        // Forgat�s a saj�t tengely k�r�l (numpad 8/2)
        if (keys[GLFW_KEY_KP_8]) {  // Numpad 8 (fel)
            angleZ -= angleSpeed * deltaTime;  // Forgat�s fel
        }
        if (keys[GLFW_KEY_KP_2]) {  // Numpad 2 (le)
            angleZ += angleSpeed * deltaTime;  // Forgat�s le
        }

        // Forgat�s balra/jobbra (numpad 4/6)
        if (keys[GLFW_KEY_KP_4]) {  // Numpad 4 (balra)
            angleY -= angleSpeed * deltaTime;  // Forgat�s balra
        }
        if (keys[GLFW_KEY_KP_6]) {  // Numpad 6 (jobbra)
            angleY += angleSpeed * deltaTime;  // Forgat�s jobbra
        }

        // Modell m�trix friss�t�se
        modelMatrix = glm::mat4(1.0f);  // Alap m�trix
        modelMatrix = glm::translate(modelMatrix, position);  // Poz�ci� alkalmaz�sa
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));  // Y tengely k�r�li forgat�s
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angleZ), glm::vec3(0.0f, 0.0f, 1.0f));  // z tengely k�r�li forgat�s

        this->setModel(modelMatrix);  // Friss�tett modell m�trix be�ll�t�sa
    }
}


MeshModel::~MeshModel()
{
}
