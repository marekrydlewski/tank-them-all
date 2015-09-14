#pragma once
#include "SceneManager.h"
#include "InitGLUT.h"
#include "Tank.h"

namespace BasicEngine
{
	class Engine
	{

	public:
		Engine();
		~Engine();

		//OpenGL and manager init
		bool Init();

		//Loop
		void Run();

		//Getters
		Managers::Scene_Manager*  GetScene_Manager()  const;
		Managers::Shader_Manager* GetShader_Manager() const;
		Managers::Models_Manager* GetModels_Manager() const;

		TextureLoader* GetTexture_Loader() const;
		void BindTank(std::string) const;
	private:
		Managers::Scene_Manager*  m_scene_manager;
		Managers::Shader_Manager* m_shader_manager;
		Managers::Models_Manager* m_models_manager;

		TextureLoader* m_texture_loader;
	};
}