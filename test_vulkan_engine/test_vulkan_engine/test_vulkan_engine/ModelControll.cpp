#include "ModelControll.h"

void ModelControll::keyControl(bool* keys, float deltaTime, float angleSpeed)
{

    if (keys[GLFW_KEY_LEFT]) { // Bal ny�l
        *angle -= angleSpeed * deltaTime;  // Forgat�s balra
    }
    if (keys[GLFW_KEY_RIGHT]) { // Jobb ny�l
        *angle += angleSpeed * deltaTime;  // Forgat�s jobbra
    }
    if (keys[GLFW_KEY_UP]) { // Fel ny�l
        *angleX -= angleSpeed * deltaTime;  // X tengely k�r�li forgat�s
    }
    if (keys[GLFW_KEY_DOWN]) { // Le ny�l
        *angleX += angleSpeed * deltaTime;  // X tengely k�r�li forgat�s
    }

    /*
    // A forgat�s m�trix alkalmaz�sa a modellre
    glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(*angle), glm::vec3(0.0f, 1.0f, 0.0f));  // Y tengely k�r�li forgat�s
    rotationMat = glm::rotate(rotationMat, glm::radians(*angleX), glm::vec3(1.0f, 0.0f, 0.0f));  // X tengely k�r�li forgat�s

    // Alkalmazd a forgat�s m�trixot a modellre
    this->setModel(rotationMat);
    */
}
