#include "stdafx.h"
#include "ImportedModel.h"

ImportedModel::ImportedModel()
{	
}

ImportedModel::~ImportedModel()
{
}

void ImportedModel::Create(GLchar* path)
{
	this->loadModel(path);
}

void ImportedModel::Draw()
{
	//abstract
}

void ImportedModel::Draw(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
	for (unsigned int i = 0; i < this->meshes.size(); i++)
	{
		this->meshes[i]->Draw(projection_matrix, view_matrix);
	}
}

void ImportedModel::Update()
{
	for (unsigned int i = 0; i < this->meshes.size(); i++)
	{
		this->meshes[i]->Update();
	}
}

void ImportedModel::SetProgram(GLuint program)
{
	this->program = program;
}

GLuint ImportedModel::GetVao() const
{
	return NULL;
}

const std::vector<GLuint> ImportedModel::GetVbos() const
{
	std::vector<GLuint> nullvec;
	return nullvec;
}

const GLuint ImportedModel::GetTexture(std::string textureName) const
{
	return NULL;
}

void ImportedModel::SetTexture(std::string textureName, GLuint texture)
{
	//abstract
}

void ImportedModel::Destroy()
{
	for (auto& m : meshes)
	{
		m->Destroy();
	}
}

#pragma region DataLoad
void ImportedModel::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	this->directory = path.substr(0, path.find_last_of('/'));

	this->processNode(scene->mRootNode, scene);
}

void ImportedModel::processNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh* processedMesh = this->processMesh(mesh, scene);
		processedMesh->SetProgram(this->program);
		this->meshes.push_back(processedMesh);
	}
	// Then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}
}

Mesh* ImportedModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<VertexFormat> vertices;
	std::vector<GLuint> indices;
	std::vector<TextureWrap> textures;

	//Process position, normals, etc.
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		VertexFormat vertex;
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		/*vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normals = vector;*/

		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texture = vec;
		}
		else
			vertex.texture = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// Process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<TextureWrap> diffuseMaps = this->loadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<TextureWrap> specularMaps = this->loadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return new Mesh(vertices, indices, textures);
}

std::vector<TextureWrap> ImportedModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<TextureWrap> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string filename = excludeFilename(str).C_Str();
		std::string path = excludePath(this->directory).C_Str();
		aiString fullPath;
		fullPath.Set(path + filename);
		GLboolean skip = false;
		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == fullPath)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			TextureWrap texture;
			int width, height;
			texture.id = filename.find(".dds") != std::string::npos ? TextureLoader::loadDDS(fullPath.C_Str()) : TextureLoader::LoadTexture(fullPath.C_Str(), width, height);
			texture.type = typeName;
			texture.path = fullPath;
			textures.push_back(texture);
			this->textures_loaded.push_back(texture);  // Add to loaded textures
		}
	}
	return textures;
}

aiString ImportedModel::excludeFilename(aiString name)
{
	std::string strName = name.C_Str();
	aiString result;
	std::string tempStr = "";
	for (int i = strName.length() - 1; i >= 0; i--)
	{
		if (strName[i] == '\\')
			break;
		else
		{
			tempStr = strName[i] + tempStr;
		}
	}
	result.Set(tempStr);
	return result;
}

aiString ImportedModel::excludePath(std::string name)
{
	aiString result;
	std::string tempStr = "";
	bool skip = true;
	for (int i = name.length() - 1; i >= 0; i--)
	{
		if (name[i] == '\\')
			skip = false;
		if (!skip)
		{
			tempStr = name[i] + tempStr;
		}
	}
	result.Set(tempStr);
	return result;
}
#pragma endregion This is loading data - it should actually work