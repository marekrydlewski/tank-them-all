#include "stdafx.h"
#include "Terrain.h"
#include <SOIL.h>

using namespace BasicEngine::Rendering;
using namespace BasicEngine::Rendering::Models;

GLfloat LightPosition[] = { 0.0, 10.0, 0.0 };

Terrain::Terrain()
{
}


Terrain::~Terrain()
{
}

void Terrain::Create(char *filename)
{
	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	unsigned char *data = SOIL_load_image(filename, &width, &height, &channels, SOIL_LOAD_L);
	centerX = width / 2;
	centerZ = height / 2;
	scaleMap = 1.0;

	heights = new float*[height];
	for (int i = 0; i < height; ++i)
		heights[i] = new float[width];

	vertexNormals = new glm::vec3*[height];
	for (int i = 0; i < height; ++i)
	{
		vertexNormals[i] = new glm::vec3[width];
		for (int j = 0; j < width; ++j)
			vertexNormals[i][j] = glm::vec3(0, 0, 0);
	}

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			heights[i][j] = ((float)data[i*width + j] / 32.0) - 4;
		}

	ComputeNormals();

	SOIL_free_image_data(data);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	std::vector<VertexFormat> vertices;
	std::vector<GLuint> indices;

	for (int i = 0; i < height - 1; i++)
	{
		for (int j = 0; j < width - 1; j++)
		{
			vertices.push_back(VertexFormat(glm::vec3((j - centerX) / scaleMap, heights[i][j], (i - centerZ) / scaleMap), glm::vec2(0, 0), vertexNormals[i][j]));
			vertices.push_back(VertexFormat(glm::vec3((j - centerX) / scaleMap, heights[i + 1][j], (i + 1 - centerZ) / scaleMap), glm::vec2(0, 1), vertexNormals[i + 1][j]));
			vertices.push_back(VertexFormat(glm::vec3((j + 1 - centerX) / scaleMap, heights[i][j + 1], (i - centerZ) / scaleMap), glm::vec2(1, 0), vertexNormals[i][j + 1]));
			vertices.push_back(VertexFormat(glm::vec3((j + 1 - centerX) / scaleMap, heights[i + 1][j + 1], (i + 1 - centerZ) / scaleMap), glm::vec2(1, 1), vertexNormals[i + 1][j + 1]));
		}
	}

	for (int i = 0; i < (width-1)*(height-1)*4; i = i + 4)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i + 3);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexFormat), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (GLvoid*)(offsetof(VertexFormat, VertexFormat::texture)));
	glBindVertexArray(0);

	this->vao = vao;
	this->vbos.push_back(vbo);
	this->vbos.push_back(ibo);

	this->model_matrix = glm::mat4(1.0);

	int texw, texh;
	GLuint grass;
	grass = TextureLoader::LoadTexture("textures\\grass.jpg", texw, texh);
	SetTexture("grass", grass);
}

void Terrain::ComputeNormals()
{
	for (int i = 0; i < height - 1; i++)
	{
		for (int j = 0; j < width - 1; j++)
		{
			float A, B, C;
			if (j % 2 == 0)
			{
				A = heights[i][j];
				B = heights[i + 1][j];
				C = heights[i][j + 1];
				glm::vec3 crossProduct = glm::cross(glm::vec3(0, A - B, -1 / scaleMap), glm::vec3(1 / scaleMap, C - B, -1 / scaleMap));
				vertexNormals[i][j] += crossProduct;
				vertexNormals[i][j + 1] += crossProduct;
				vertexNormals[i + 1][j] += crossProduct;
				faceNormals.push_back(crossProduct);
			}
			else
			{
				A = heights[i][j];
				B = heights[i + 1][j];
				C = heights[i + 1][j - 1];
				glm::vec3 crossProduct = glm::cross(glm::vec3(-1 / scaleMap, C - A, 1 / scaleMap), glm::vec3(0, B - A, 1 / scaleMap));
				vertexNormals[i][j] += crossProduct;
				vertexNormals[i + 1][j - 1] += crossProduct;
				vertexNormals[i + 1][j] += crossProduct;
				faceNormals.push_back(crossProduct);
			}
		}
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			vertexNormals[i][j] = glm::normalize(vertexNormals[i][j]);
		}
	}

	for (int i = 0; i < faceNormals.size(); i++)
	{
		faceNormals[i] = glm::normalize(faceNormals[i]);
	}
}

void Terrain::Draw()
{
	
}

void Terrain::Draw(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	GLint location = glGetUniformLocation(program, "texture_grass");
	glUniform1i(location, 0);
	glBindTexture(GL_TEXTURE_2D, this->textures["grass"]);
	location = glGetUniformLocation(program, "light_source_1");
	glUniform3fv(location, 1, LightPosition);
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, false, &model_matrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"), 1, false, &view_matrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "projection_matrix"), 1, false, &projection_matrix[0][0]);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (width-1)*(height-1)*6, GL_UNSIGNED_INT, 0);
}

void Terrain::Update()
{

}

void Terrain::SetProgram(GLuint program)
{
	if (program == 0) return;
	this->program = program;
}

GLuint Terrain::GetVao() const
{
	return vao;
}

const std::vector<GLuint> Terrain::GetVbos() const
{
	return vbos;
}

const GLuint Terrain::GetTexture(std::string textureName) const
{
	return 0;
}

void Terrain::SetTexture(std::string textureName, GLuint texture)
{
	if (texture == 0) return;
	textures[textureName] = texture;
}

void Terrain::Destroy()
{
	delete heights;
	delete vertexNormals;
	faceNormals.clear();
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(vbos.size(), &vbos[0]);
	vbos.clear();
}