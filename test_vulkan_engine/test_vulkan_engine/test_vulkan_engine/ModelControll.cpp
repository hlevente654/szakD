#include "ModelControll.h"

void ModelControll::keyControl(bool* keys, float deltaTime, float angleSpeed)
{

    if (keys[GLFW_KEY_LEFT]) { // Bal nyíl
        *angle -= angleSpeed * deltaTime;  // Forgatás balra
    }
    if (keys[GLFW_KEY_RIGHT]) { // Jobb nyíl
        *angle += angleSpeed * deltaTime;  // Forgatás jobbra
    }
    if (keys[GLFW_KEY_UP]) { // Fel nyíl
        *angleX -= angleSpeed * deltaTime;  // X tengely körüli forgatás
    }
    if (keys[GLFW_KEY_DOWN]) { // Le nyíl
        *angleX += angleSpeed * deltaTime;  // X tengely körüli forgatás
    }

    /*
    // A forgatás mátrix alkalmazása a modellre
    glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(*angle), glm::vec3(0.0f, 1.0f, 0.0f));  // Y tengely körüli forgatás
    rotationMat = glm::rotate(rotationMat, glm::radians(*angleX), glm::vec3(1.0f, 0.0f, 0.0f));  // X tengely körüli forgatás

    // Alkalmazd a forgatás mátrixot a modellre
    this->setModel(rotationMat);
    */
}
