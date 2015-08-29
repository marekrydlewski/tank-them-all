#include "stdafx.h"
#include "ModelsManager.h"


ModelsManager::ModelsManager()
{
	//triangle game object
	Triangle* triangle = new Triangle();
	triangle->SetProgram(ShaderManager::GetShader("colorShader"));
	triangle->Create();
	gameModelList["triangle"] = triangle;

}

ModelsManager::~ModelsManager()
{
	//auto -it's a map iterator
	for (auto& model : gameModelList)
	{
		delete model.second;
	}
	gameModelList.clear();
}

void ModelsManager::DeleteModel(const std::string& gameModelName)
{
	IGameObject* model = gameModelList[gameModelName];
	model->Destroy();
	gameModelList.erase(gameModelName);
}

const IGameObject& ModelsManager::GetModel(const std::string& gameModelName) const
{
	return (*gameModelList.at(gameModelName));
}

void ModelsManager::Update()
{
	//auto -it's a map iterator
	for (auto& model : gameModelList)
	{
		model.second->Update();
	}
}

void ModelsManager::Draw()
{
	//auto -it's a map iterator
	for (auto& model : gameModelList)
	{
		model.second->Draw();
	}
}