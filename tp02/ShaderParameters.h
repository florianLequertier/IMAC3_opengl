#pragma once

#include <glm/glm.hpp>
#include <glew/glew.h>
#include "EditorGUI.h"
#include "ISerializable.h"

// Helper Functions to push data to gpu
namespace GlHelper {

template<typename T>
void pushParameterToGPU(GLuint uniformId, const T& value)
{
	assert(0 && "invalid parameter type.");
}

template<>
void pushParameterToGPU<int>(GLuint uniformId, const int& value)
{
	glUniform1i(uniformId, value);
}

template<>
void pushParameterToGPU<float>(GLuint uniformId, const float& value)
{
	glUniform1f(uniformId, value);
}

template<>
void pushParameterToGPU<glm::vec2>(GLuint uniformId, const glm::vec2& value)
{
	glUniform2f(uniformId, value.x, value.y);
}

template<>
void pushParameterToGPU<glm::ivec2>(GLuint uniformId, const glm::ivec2& value)
{
	glUniform2i(uniformId, value.x, value.y);
}

template<>
void pushParameterToGPU<glm::vec3>(GLuint uniformId, const glm::vec3& value)
{
	glUniform3f(uniformId, value.x, value.y, value.z);
}

template<>
void pushParameterToGPU<glm::ivec3>(GLuint uniformId, const glm::ivec3& value)
{
	glUniform3i(uniformId, value.x, value.y, value.z);
}

template<>
void pushParameterToGPU<glm::mat4>(GLuint uniformId, const glm::mat4& value)
{
	glUniformMatrix4fv(uniformId, 1, false,  glm::value_ptr(value));
}

//array version (experimental) : 

template<typename T>
void pushParametersToGPU(GLuint uniformId, int count, const std::vector<T>& values)
{
	assert(0 && "invalid parameter type.");
}


template<>
void pushParametersToGPU<float>(GLuint uniformId, int count, const std::vector<float>& values)
{
	glUniform1fv(uniformId, count, &values[0]);
}

//TODO : complete

}


namespace ShaderParameter {

//valid types for a shader parameter.
enum ShaderParameterType {
	INT,
	INT2,
	INT3,
	FLOAT,
	FLOAT2,
	FLOAT3,
	TEXTURE,
	CUBE_TEXTURE,
	//arrays :
	ARRAY_INT,
	ARRAY_INT2,
	ARRAY_INT3,
	ARRAY_FLOAT,
	ARRAY_FLOAT2,
	ARRAY_FLOAT3,
	ARRAY_TEXTURE,
	ARRAY_CUBE_TEXTURE,
	TYPE_COUNT
};

//these types will appear with these names in .glProg files.
static std::vector<std::string> LiteralShaderParameterType{
	"int",
	"int2",
	"int3",
	"float",
	"float2",
	"float2",
	"texture",
	"cubeTexture",
	//arrays :
	"int[]",
	"int2[]",
	"int3[]",
	"float[]",
	"float2[]",
	"float2[]",
	"texture[]",
	"cubeTexture[]",
};

struct IsArray {};
struct IsNotArray {};

}


//////////////////////////////////////////////////////////////////////////////////////////
////////// BEGIN : internal parameters

//The ShaderParameter interface. 
//A ShaderParameter is an abstraction of a data which can be send to gpu.
class InternalShaderParameterBase : public ISerializable
{
protected:
	std::string m_name;

public:
	InternalShaderParameterBase(const std::string& name)
		: m_name(name)
	{}
	virtual ~InternalShaderParameterBase(){}
	virtual void init(GLuint glProgramId) = 0;
	virtual void drawUI() = 0;
	virtual void pushToGPU(int& boundTextureCount) = 0;
	const std::string& getName() const
	{
		return m_name;
	}
};


template<typename T, typename U>
class InternalShaderParameter : public InternalShaderParameterBase
{
	//UNDEFINED FOR U != (IsArray | IsNotArray) !!!
};

template<typename T>
class InternalShaderParameter<T, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	T m_data;
	GLuint m_uniformId;
	bool m_isEditable;

public:
	InternalShaderParameter(const std::string& name, bool isEditable = true)
		: InternalShaderParameterBase(name)
		, m_isEditable(isEditable)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void drawUI() override
	{
		EditorGUI::ValueField<T>(m_name, m_data);
	}

	void pushToGPU(int& boundTextureCount) override
	{
		GlHelper::pushParameterToGPU<T>(m_uniformId, m_data);
	}

	void save(Json::Value & entityRoot) const override
	{
		jsonSaveValue<T>(entityRoot, "data", m_data);
	}
	void load(Json::Value & entityRoot) override
	{
		jsonLoadValue<T>(entityRoot, "data", m_data);
	}
};

//array version isn't editable for now
//array version, multiple uniforms and data. By convention the name of uniform[i] is m_name[i]
template<typename T>
class InternalShaderParameter<T, ShaderParameter::IsArray> : public InternalShaderParameterBase
{
private:
	std::vector<GLuint> m_uniformId;
	std::vector<T> m_datas;

public:
	InternalShaderParameter(const std::string& name)
		: InternalShaderParameterBase(name)
	{}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		for (int i = 0; i < m_data.size(); i++)
			m_uniformIds[i] = glGetUniformLocation(glProgram, (m_name+"[" + std::to_string(i) + "]").c_str());
	}

	void drawUI() override
	{
		//Not editable for now. TODO
		//EditorGUI::ValueField<T>(m_name, m_data);
	}

	void pushToGPU(int& boundTextureCount) override
	{
		for (int i = 0; i < m_datas.size(); i++)
		{
			GlHelper::pushParameterToGPU<T>(m_uniformIds[i], m_datas[i]);
		}
	}

	void save(Json::Value & entityRoot) const override
	{
		entityRoot["count"] = m_datas.size();
		for(int i = 0; i < m_datas.size(); i ++)
			jsonSaveValue<T>(entityRoot, "data["+i+"]", m_datas[i]);
	}
	void load(Json::Value & entityRoot) override
	{
		int count = entityRoot["count"];
		m_datas.resize(count);
		for (int i = 0; i < m_datas.size(); i++)
			jsonLoadValue<T>(entityRoot, "data["+i+"]", m_datas[i]);
	}
};

//Array version not allowed for now
template<>
class InternalShaderParameter<Texture, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	ResourcePtr<Texture> m_data;
	GLuint m_uniformId;

public:
	InternalShaderParameter(const std::string& name)
		: InternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void drawUI() override
	{
		EditorGUI::ValueField<Texture>(m_name, m_data);
	}

	void pushToGPU(int& boundTextureCount) override
	{
		glActiveTexture(GL_TEXTURE0 + boundTextureCount);
		glBindTexture(GL_TEXTURE_2D, m_data->glId);
		glUniform1i(m_uniformId, boundTextureCount);
		boundTextureCount++;
	}

	void save(Json::Value & entityRoot) const override
	{
		m_data.save(entityRoot);
	}
	void load(Json::Value & entityRoot) override
	{
		m_data.load(entityRoot);
	}
};

//Array version not allowed for now
template<>
class InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	ResourcePtr<CubeTexture> m_data;
	GLuint m_uniformId;

public:
	InternalShaderParameter(const std::string& name)
		: InternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void drawUI() override
	{
		EditorGUI::ValueField<CubeTexture>(m_name, m_data);
	}

	void pushToGPU(int& boundTextureCount) override
	{
		glActiveTexture(GL_TEXTURE0 + boundTextureCount);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_data->glId);
		glUniform1i(m_uniformId, boundTextureCount);
		boundTextureCount++;
	}

	void save(Json::Value & entityRoot) const override
	{
		m_data.save(entityRoot);
	}
	void load(Json::Value & entityRoot) override
	{
		m_data.load(entityRoot);
	}
};

//utility function to make a shader from its type
std::shared_ptr<InternalShaderParameterBase> MakeNewInternalShaderParameter(const std::string& literaltype, std::string& name)
{
	ShaderParameter::ShaderParameterType parameterType = ShaderParameter::ShaderParameterType::TYPE_COUNT;
	auto& foundTypeIt = std::find(ShaderParameter::LiteralShaderParameterType.begin(), ShaderParameter::LiteralShaderParameterType.end(), literaltype);
	if (foundTypeIt != ShaderParameter::LiteralShaderParameterType.end())
		parameterType = (ShaderParameter::ShaderParameterType)std::distance(ShaderParameter::LiteralShaderParameterType.begin(), foundTypeIt);
	else
		return nullptr;


	switch (parameterType)
	{
	case ShaderParameter::ShaderParameterType::INT:
		return std::make_shared<InternalShaderParameter<int, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT2:
		return std::make_shared<InternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT3:
		return std::make_shared<InternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT:
		return std::make_shared<InternalShaderParameter<float, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT2:
		return std::make_shared<InternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT3:
		return std::make_shared<InternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::TEXTURE:
		return std::make_shared<InternalShaderParameter<Texture, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::CUBE_TEXTURE:
		return std::make_shared<InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>(name);
		break;
	default:
		return nullptr;
		break;
	}
}


////////// END : internal parameters
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
////////// BEGIN : external parameters

//The ShaderParameter interface. 
//A ShaderParameter is an abstraction of a data which can be send to gpu.
class ExternalShaderParameterBase
{
protected:
	std::string m_name;
public:
	ExternalShaderParameterBase(const std::string& name)
		: m_name(name)
	{}
	virtual ~ExternalShaderParameterBase(){}
	virtual void init(GLuint glProgramId) = 0;
	const std::string& getName() const
	{
		return m_name;
	}
	virtual GLuint getUniformId() const { return 0; }
	virtual void getUniformIds(std::vector<GLuint>& outUniforms) const { return; }
};


template<typename T, typename U = ShaderParameter::IsNotArray>
class ExternalShaderParameter : public ExternalShaderParameterBase
{
	//UNDEFINED FOR U != (IsArray | IsNotArray) !!!
};

template<typename T>
class ExternalShaderParameter<T, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(T& externalData)
	{
		GlHelper::pushParameterToGPU<T>(m_uniformId, externalData);
	}

	GLuint getUniformId() const override
	{ 
		return m_uniformId;
	}

};

//array version isn't editable for now
//array version, multiple uniforms and data. By convention the name of uniform[i] is m_name[i]
template<typename T>
class ExternalShaderParameter<T, ShaderParameter::IsArray> : public ExternalShaderParameterBase
{
private:
	std::vector<GLuint> m_uniformIds;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
	{}

	int getUniformCount() const
	{
		return m_uniformIds.size();
	}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		for (int i = 0; i < m_data.size(); i++)
			m_uniformIds[i] = glGetUniformLocation(glProgram, (m_name + "[" + std::to_string(i) + "]").c_str());
	}

	void pushToGPU(std::vector<T&>& externalDatas) override
	{
		for (int i = 0; i < m_datas.size(); i++)
		{
			GlHelper::pushParameterToGPU<T>(m_uniformIds[i], externalDatas[i]);
		}
	}

	void pushToGPU(T& externalData, int uniformIdx) override
	{
		assert(uniformIdx < m_uniformIds.size());
		GlHelper::pushParameterToGPU<T>(m_uniformIds[uniformIdx], externalData[uniformIdx]);
	}

	void getUniformIds(std::vector<GLuint>& outUniforms) const override
	{
		outUniforms = m_uniformIds;
	}

};

//Array version not allowed for now
template<>
class ExternalShaderParameter<Texture, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(GLint textureUnit)
	{
		glUniform1i(m_uniformId, textureUnit);
	}

	GLuint getUniformId() const override
	{
		return m_uniformId;
	}
};

//Array version not allowed for now
template<>
class ExternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(GLint textureUnit)
	{
		glUniform1i(m_uniformId, textureUnit);
	}

	GLuint getUniformId() const override
	{
		return m_uniformId;
	}
};

//utility function to make a shader from its type
std::shared_ptr<ExternalShaderParameterBase> MakeNewExternalShaderParameter(const std::string& literaltype, std::string& name)
{
	ShaderParameter::ShaderParameterType parameterType = ShaderParameter::ShaderParameterType::TYPE_COUNT;
	auto& foundTypeIt = std::find(ShaderParameter::LiteralShaderParameterType.begin(), ShaderParameter::LiteralShaderParameterType.end(), literaltype);
	if (foundTypeIt != ShaderParameter::LiteralShaderParameterType.end())
		parameterType = (ShaderParameter::ShaderParameterType)std::distance(ShaderParameter::LiteralShaderParameterType.begin(), foundTypeIt);
	else
		return nullptr;


	switch (parameterType)
	{
	case ShaderParameter::ShaderParameterType::INT:
		return std::make_shared<ExternalShaderParameter<int, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT2:
		return std::make_shared<ExternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT3:
		return std::make_shared<ExternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT:
		return std::make_shared<ExternalShaderParameter<float, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT2:
		return std::make_shared<ExternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT3:
		return std::make_shared<ExternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::TEXTURE:
		return std::make_shared<ExternalShaderParameter<Texture, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::CUBE_TEXTURE:
		return std::make_shared<ExternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>(name);
		break;
//array versions :
	case ShaderParameter::ShaderParameterType::ARRAY_INT:
		return std::make_shared<ExternalShaderParameter<int, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_INT2:
		return std::make_shared<ExternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_INT3:
		return std::make_shared<ExternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT:
		return std::make_shared<ExternalShaderParameter<float, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT2:
		return std::make_shared<ExternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT3:
		return std::make_shared<ExternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name);
		break;
	default:
		return nullptr;
		break;
	}
}

////////// END : internal parameters
//////////////////////////////////////////////////////////////////////////////////////////


