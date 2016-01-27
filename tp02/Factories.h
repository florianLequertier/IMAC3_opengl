#pragma once

#include <map>

#include "stb/stb_image.h"

#include "Mesh.h"
#include "Materials.h"
#include "Texture.h"


class ProgramFactory
{

private:
	std::map<std::string, GLuint> m_programs;

public:
	void add(const std::string& name, GLuint programId);
	GLuint get(const std::string& name);
	bool contains(const std::string& name);
	void drawUI();

	// singleton implementation :
private:
	ProgramFactory() {

	}

public:
	inline static ProgramFactory& get()
	{
		static ProgramFactory instance;

		return instance;
	}


	ProgramFactory(const ProgramFactory& other) = delete;
	void operator=(const ProgramFactory& other) = delete;
};


//////////////////////////////////////

class TextureFactory
{

private:
	std::map<std::string, Texture*> m_textures;

	//for UI : 
	char name[20];
	char path[50];

public:
	void add(const std::string& name, const std::string& path);
	void add(const std::string& name, Texture* textureId);
	Texture* get(const std::string& name);
	bool contains(const std::string& name);
	void drawUI();

	// singleton implementation :
private:
	TextureFactory();

public:
	inline static TextureFactory& get()
	{
		static TextureFactory instance;

		return instance;
	}


	TextureFactory(const TextureFactory& other) = delete;
	void operator=(const TextureFactory& other) = delete;
};


///////////////////////////////

class MeshFactory
{

private:
	std::map<std::string, Mesh*> m_meshes;

	//for UI : 
	char name[20];
	char path[50];

public:
	void add(const std::string& name, Mesh* mesh);
	Mesh* get(const std::string& name);
	bool contains(const std::string& name);
	void drawUI();

	// singleton implementation :
private:
	MeshFactory() {

	}

public:
	inline static MeshFactory& get()
	{
		static MeshFactory instance;

		return instance;
	}


	MeshFactory(const MeshFactory& other) = delete;
	void operator=(const MeshFactory& other) = delete;
};

////////////////////////////////

class MaterialFactory
{

private:
	std::map<std::string, Material*> m_materials;

	//for UI : 
	char name[20];

public:
	void add(const std::string& name, Material* material);
	Material* get(const std::string& name);
	bool contains(const std::string& name);
	void drawUI();

	// singleton implementation :
private:
	MaterialFactory() {

	}

public:
	inline static MaterialFactory& get()
	{
		static MaterialFactory instance;

		return instance;
	}


	MaterialFactory(const MaterialFactory& other) = delete;
	void operator=(const MaterialFactory& other) = delete;
};
