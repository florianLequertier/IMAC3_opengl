#include "stdafx.h"

#include "Texture.h"
#include "Utils.h"
#include "ErrorHandler.h"

#include "jsoncpp/json/json.h"


Texture::Texture() 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, comp(3)
	, pixels(0)
	, w(1)
	, h(1)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{

}

Texture::Texture(int width, int height) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, comp(3)
	, pixels(0)
	, w(width)
	, h(height)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	//pixels = new unsigned char[3*width*height];
	//for (int i = 0; i < 3*width*height; i++) {
	//	pixels[i] = 255;
	//}
}

Texture::Texture(unsigned char * _pixels, int width, int height, int _comp) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	comp = _comp;
	w = width;
	h = height;

	pixels = _pixels;
}

Texture::Texture(char r, char g, char b) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	comp = 4;
	w = 1;
	h = 1;
	pixels = new unsigned char[3];
	pixels[0] = r;
	pixels[1] = g;
	pixels[2] = b;
}


Texture::Texture(const FileHandler::CompletePath& _path, bool alphaChannel) 
	: Resource(_path)
	, glId(0)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	if (!alphaChannel)
	{
		internalFormat = GL_RGB;
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		pixels = stbi_load(_path.c_str(), &w, &h, &comp, 3);
	}
	else
	{
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		pixels = stbi_load(_path.c_str(), &w, &h, &comp, 4);
	}
}

Texture::Texture(int width, int height, const glm::vec4 & color) 
	: w(width)
	, h(height)
	, glId(0)
	, internalFormat(GL_RGBA)
	, format(GL_RGBA)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	comp = 4;
	pixels = new unsigned char[4*width*height];
	for (int i = 0; i < width * height * 4; i += 4)
	{
		pixels[i] = color.r;
		pixels[i + 1] = color.g;
		pixels[i + 2] = color.b;
		pixels[i + 3] = color.a;
	}
}

Texture::Texture(int width, int height, const glm::vec3 & color) 
	: w(width)
	, h(height)
	, glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
	, textureWrapping_u(GL_REPEAT)
	, textureWrapping_v(GL_REPEAT)
	, minFilter(GL_LINEAR)
	, magFilter(GL_LINEAR)
{
	comp = 3;
	pixels = new unsigned char[3 * width*height];
	for (int i = 0; i < width * height * 3; i += 3)
	{
		pixels[i] = color.r;
		pixels[i + 1] = color.g;
		pixels[i + 2] = color.b;
	}
}

void Texture::init(const FileHandler::CompletePath& path)
{
	Resource::init(path);

	internalFormat = GL_RGB;
	format = GL_RGB;
	type = GL_UNSIGNED_BYTE;
	pixels = stbi_load(path.c_str(), &w, &h, &comp, 3);
}

Texture::~Texture()
{
	delete[] pixels;
}

void Texture::setTextureParameters(GLint _internalFormat, GLenum _format, GLenum _type, bool _generateMipMap)
{
	internalFormat = _internalFormat;
	format = _format;
	type = _type;
	generateMipMap = _generateMipMap;
}

void Texture::initGL()
{

	//m_textureUseCounts++;

	//if (m_textureUseCounts == 1)
	if(glId <= 0)
	{
		glGenTextures(1, &glId);

		glBindTexture(GL_TEXTURE_2D, glId);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapping_u);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapping_v);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		if (generateMipMap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

		if (!checkError("Uniforms"))
			PRINT_ERROR("error in texture initialization.")
	}
}

void Texture::freeGL()
{
	//m_textureUseCounts--;

	//if (m_textureUseCounts <= 0)
	if(glId > 0){
		glDeleteTextures(1, &glId);
	}
}

//////////////////////////////////////////

CubeTexture::CubeTexture()
	:CubeTexture(0, 0, 0)
{

}

CubeTexture::CubeTexture(char r, char g, char b) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
{
	comp = 4;
	w = 1;
	h = 1;

	for (int i = 0; i < 6; i++)
	{
		pixels[i] = new unsigned char[3];
		pixels[i][0] = r;
		pixels[i][1] = g;
		pixels[i][2] = b;
	}
}

CubeTexture::CubeTexture(const std::vector<FileHandler::CompletePath>& _paths) 
	: glId(0)
	, internalFormat(GL_RGB)
	, format(GL_RGB)
	, type(GL_UNSIGNED_BYTE)
	, generateMipMap(true)
	, m_textureUseCounts(0)
{
	for (int i = 0; i < 6; i++)
	{
		paths[i] = _paths[i];
		pixels[i] = stbi_load(paths[i].c_str(), &w, &h, &comp, 3);
	}
}


CubeTexture::~CubeTexture()
{
	for (int i = 0; i < 6; i++)
	{
		delete[] pixels[i];
	}
}

void CubeTexture::setTextureParameters(GLint _internalFormat, GLenum _format, GLenum _type, bool _generateMipMap)
{
	internalFormat = _internalFormat;
	format = _format;
	type = _type;
	generateMipMap = _generateMipMap;
}

void CubeTexture::initGL()
{
	if (glId <= 0){

		glGenTextures(1, &glId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glId);

		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, w, h, 0, format, type, pixels[i]);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		

		if (generateMipMap)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}
		else
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}

void CubeTexture::freeGL()
{
	if (glId > 0){
			glDeleteTextures(1, &glId);
	}
}

void CubeTexture::init(const FileHandler::CompletePath & path)
{
	Resource::init(path);

	load(path);
}

void CubeTexture::load(const FileHandler::CompletePath & path)
{
	std::ifstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load cube texture at path : " << path.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	for (int i = 0; i < 6; i++)
	{
		const std::string strPath = root[i]["path"].asString();
		paths[i] = FileHandler::CompletePath(strPath);
	}

	internalFormat = root["internalFormat"].asInt();
	format = root["format"].asInt();
	type = root["type"].asInt();
	generateMipMap = root["generateMipMap"].asBool();
}

void CubeTexture::save(const FileHandler::CompletePath & path) const
{
	Json::Value root;

	for (int i = 0; i < 6; i++)
	{
		root[i]["path"] = paths[i].toString();
	}

	root["internalFormat"] = (int)internalFormat;
	root["format"] = (int)format;
	root["type"] = (int)type;
	root["generateMipMap"] = generateMipMap;

	std::ofstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load cube texture at path : " << path.toString() << std::endl;
		return;
	}
	stream << root;
}
