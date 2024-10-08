#include "Engine/Source/Public/Object/ObjectManager.h"

// Project Includes
#include "Engine/Source/EngineManager.h"

#include "Engine/Source/Public/Object/Object.h"
#include "Engine/Source/Public/Object/GameObject.h"
#include "Engine/Source/Public/Rendering/MeshModel.h"

#include "Engine/Source/Public/Rendering/Renderer.h"

GameObject* ObjectManager::CreateGameObject(ObjectData _objectData, Mesh* _mesh, MeshModel* _meshModel)
{
    int id;
    if (!reusableIDs.empty()) 
    {
        // Get the smallest ID from the reusable ID set
        id = *reusableIDs.begin();
        reusableIDs.erase(reusableIDs.begin());
    }
    else 
    {
        // Assign the next sequential ID
        id = nextID++;
    }

    GameObject* newObj = new GameObject(_objectData, _mesh, _meshModel);
    newObj->SetUseTexture(1);
    newObj->SetModel(_objectData.objectMatrix);
    gameObjects.push_back(newObj);
    return newObj;
}

void ObjectManager::DestroyAllGameObjects()
{
    if (seEngineManager == nullptr)
        seEngineManager = EngineManager::GetEngineManager();

    // Wait until queues and all operations are done before cleaning up
    vkDeviceWaitIdle(seEngineManager->GetRenderer()->GetLogicalDevice());

    for (int i = 0; i < gameObjects.size(); i++)
    {
        MeshModel* tempModel = gameObjects[i]->objectMeshModel;
        tempModel->DestroyMeshModel();
        delete gameObjects[i];
    }

    gameObjects.clear();
}
