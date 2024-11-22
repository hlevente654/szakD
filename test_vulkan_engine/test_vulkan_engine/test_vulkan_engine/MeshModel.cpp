#include "MeshModel.h"



MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> newMeshList, bool isControlable, glm::vec3 startPos, glm::vec3 lookAt)
{
    meshList = newMeshList;
    model = glm::mat4(1.0f);
    controlable = isControlable;
    position = startPos;

    glm::vec3 direction = glm::normalize(lookAt - startPos);

    angleY = 0.0f;
    angleX = 0.0f;
    angleY = angleY + glm::degrees(atan2(direction.x, direction.z));

    this->setModel(glm::translate(glm::mat4(1.0f), position));
}

MeshModel::MeshModel(std::vector<Mesh> newMeshList, bool isControlable, glm::vec3 startPos)
{
	meshList = newMeshList;
	model = glm::mat4(1.0f);
	controlable = isControlable;
	position = startPos;

	this->setModel(glm::translate(glm::mat4(1.0f), position));
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

glm::vec3 MeshModel::getPosition()
{
    return position;
}

glm::vec3 MeshModel::getDirection() {
    glm::vec3 forward = glm::normalize(glm::vec3(model[2]));
    return forward;
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

        // Set texture coordinates (if they exist)
        if (mesh->mTextureCoords[0]) {
            vertices[i].tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else {
            vertices[i].tex = { 0.0f, 0.0f };
        }

        // Set color (default to white)
        vertices[i].col = { 1.0f, 1.0f, 1.0f };

        // Initialize normal vector with default value
        vertices[i].norm = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    // Copy indices from mesh
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Ha a modell nem tartalmaz normálokat, akkor számoljuk ki õket
    if (!mesh->HasNormals()) {
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            // Háromszög három pontja
            glm::vec3 v0 = vertices[i0].pos;
            glm::vec3 v1 = vertices[i1].pos;
            glm::vec3 v2 = vertices[i2].pos;

            // Normál számítása
            glm::vec3 normal = calculateNorm(v0, v1, v2);

            // Normál hozzáadása a három vertexhez
            vertices[i0].norm += normal;
            vertices[i1].norm += normal;
            vertices[i2].norm += normal;
        }

        // Végsõ normalizálás minden vertex normáljára
        for (auto& vertex : vertices) {
            vertex.norm = glm::normalize(vertex.norm);
        }
    }
    else {
        // Ha a modell tartalmaz normálokat, másoljuk át õket
        for (size_t i = 0; i < mesh->mNumVertices; i++) {
            vertices[i].norm = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
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
        // Irányvektorok számítása
        glm::vec3 forward = getDirection(); // Az aktuális irány
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))); // Jobbra mutató vektor
        glm::vec3 up = glm::normalize(glm::cross(right, forward)); // Felfelé mutató vektor

        // Mozgás elõre/hátra
        if (keys[GLFW_KEY_UP]) {
            position += forward * moveSpeed * deltaTime; // Elõre
        }
        if (keys[GLFW_KEY_DOWN]) {
            position -= forward * moveSpeed * deltaTime; // Hátra
        }

        // Mozgás balra/jobbra
        if (keys[GLFW_KEY_LEFT]) {
            position -= right * moveSpeed * deltaTime; // Balra
        }
        if (keys[GLFW_KEY_RIGHT]) {
            position += right * moveSpeed * deltaTime; // Jobbra
        }

        // Felfelé/lefelé mozgás (ha Ctrl nyomva van)
        bool isCtrlPressed = keys[GLFW_KEY_RIGHT_CONTROL] || keys[GLFW_KEY_LEFT_CONTROL];
        if (isCtrlPressed) {
            if (keys[GLFW_KEY_UP]) {
                position += up * moveSpeed * deltaTime; // Felfelé
            }
            if (keys[GLFW_KEY_DOWN]) {
                position -= up * moveSpeed * deltaTime; // Lefelé
            }
        }

        // Forgatás a saját tengelyek körül
        if (keys[GLFW_KEY_KP_8]) {
            angleX -= angleSpeed * deltaTime; // Fel (X tengely körül)
        }
        if (keys[GLFW_KEY_KP_2]) {
            angleX += angleSpeed * deltaTime; // Le (X tengely körül)
        }
        if (keys[GLFW_KEY_KP_4]) {
            angleY -= angleSpeed * deltaTime; // Balra (Y tengely körül)
        }
        if (keys[GLFW_KEY_KP_6]) {
            angleY += angleSpeed * deltaTime; // Jobbra (Y tengely körül)
        }

        // Modell mátrix frissítése
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position); // Pozíció alkalmazása
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f)); // X tengely körüli forgatás (fel/le)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f)); // Y tengely körüli forgatás (balra/jobbra)

        this->setModel(modelMatrix); // Frissített mátrix beállítása
    }
}





MeshModel::~MeshModel()
{
}

glm::vec3 MeshModel::calculateNorm(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    // Két élt számolunk a háromszögbõl
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    // Keresztszorzatot alkalmazunk, hogy meghatározzuk a normált
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    return normal;
}

