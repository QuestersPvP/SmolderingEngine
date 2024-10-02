#include "Engine/Source/Public/Object/ObjectManager.h"

// Project Includes
#include "Engine/Source/Public/Object/Object.h"
#include "Engine/Source/Public/Object/GameObject.h"

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
