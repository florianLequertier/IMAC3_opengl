#include "Project.h"
#include "Materials.h"
#include "RenderBatch.h"
#include "ShaderParameters.h"
#include "ShaderProgram.h"
#include "Factories.h"
#include "ResourceTree.h"
#include "Editor.h"
#include "MaterialVisualScripting.h"


ShaderProgram::ShaderProgram()
	: m_materialType(Rendering::MaterialType::INTERNAL)
	, m_usedWithReflections(false)
	, m_usedWithSkeleton(false)
	, m_pipeline(Rendering::PipelineType::CUSTOM_PIPELINE)
	, m_usage(Rendering::MaterialUsage::MESH)
	//:id(0)
{
}

ShaderProgram::ShaderProgram(const FileHandler::CompletePath& path)
	: Resource(path)
	, m_materialType(Rendering::MaterialType::INTERNAL)
	, m_usedWithReflections(false)
	, m_usedWithSkeleton(false)
	, m_pipeline(Rendering::PipelineType::CUSTOM_PIPELINE)
	, m_usage(Rendering::MaterialUsage::MESH)
	//, id(0)
{
}

//ShaderProgram::ShaderProgram(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath)
//	: id(0)
//{
//	load(vertexShaderPath, fragmentShaderPath);
//}
//
//
//ShaderProgram::ShaderProgram(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath, const FileHandler::CompletePath& geometryShaderPath)
//	: id(0)
//{
//	load(vertexShaderPath, fragmentShaderPath, geometryShaderPath);
//}

const std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>>& ShaderProgram::getAggregations() const
{
	return m_aggregations;
}

const std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>>& ShaderProgram::getPerInstanceAggregation() const
{
	return m_perInstanceAggregations;
}

GLuint ShaderProgram::getProgramId() const
{
	return m_programId;
}

void ShaderProgram::compile()
{
	GLuint newProgramId = makeGLProgram();

	m_perInstanceAggregations.clear();
	m_aggregations.clear();

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		auto aggregation = std::make_shared<MaterialAggregationForward>();
		aggregation->initParameters(newProgramId);
		m_aggregations["forward"] = aggregation;
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		// NOTHING
	}

	if (m_usedWithSkeleton)
	{
		auto aggregation = std::make_shared<MaterialAggregationWithSkeleton>();
		aggregation->initParameters(newProgramId);
		m_perInstanceAggregations["skeleton"] = aggregation;
	}
	else
	{
		//TODO
	}

	if (m_usedWithReflections)
	{
		//TODO
	}
	else
	{
		//TODO
	}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		auto aggregation = std::make_shared<MaterialAggregationMesh>();
		aggregation->initParameters(newProgramId);
		m_perInstanceAggregations["mesh"] = aggregation;
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		auto aggregation = std::make_shared<MaterialAggregationBillboard>();
		aggregation->initParameters(newProgramId);
		m_perInstanceAggregations["billboard"] = aggregation;
	}
	if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		auto aggregation = std::make_shared<MaterialAggregationParticles>();
		aggregation->initParameters(newProgramId);
		m_perInstanceAggregations["particles"] = aggregation;
	}
}

void ShaderProgram::init(const FileHandler::CompletePath& path, const ID& id)
{
	Resource::init(path, id);

	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);

	load(absolutePath);
}

void ShaderProgram::load(const FileHandler::CompletePath& path)
{
	std::ifstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load shader program at path : " << path.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	// Pipeline handling type
	//m_pipelineHandlingTypes.clear();
	//Json::Value pipelineHandlingTypeJson = root["pipelineHandling"];
	//if (pipelineHandlingTypeJson.isArray())
	//{
	//	for (int i = 0; i < pipelineHandlingTypeJson.size(); i++)
	//	{
	//		auto foundItPipelineType = std::find(Rendering::PipelineTypesToString.begin(), Rendering::PipelineTypesToString.end(), pipelineHandlingTypeJson[i].asString());
	//		assert(foundItPipelineType != Rendering::PipelineTypesToString.end());
	//		int foundIdxPipelineType = foundItPipelineType - Rendering::PipelineTypesToString.begin();
	//		m_pipelineHandlingTypes.push_back((Rendering::PipelineType)foundIdxPipelineType);
	//	}
	//}
	//else
	//{
	//	auto foundItPipelineType = std::find(Rendering::PipelineTypesToString.begin(), Rendering::PipelineTypesToString.end(), root.get("pipelineHandling", "").asString()/*pipelineHandlingTypeJson.asString()*/);
	//	assert(foundItPipelineType != Rendering::PipelineTypesToString.end());
	//	int foundIdxPipelineType = foundItPipelineType - Rendering::PipelineTypesToString.begin();
	//	m_pipelineHandlingTypes.push_back((Rendering::PipelineType)foundIdxPipelineType);
	//}

	// Base Material
	//auto foundItBaseMaterial = std::find(Rendering::BaseMaterialTypeToString.begin(), Rendering::BaseMaterialTypeToString.end(), root.get("baseMaterial", "").asString());
	//assert(foundItBaseMaterial != Rendering::BaseMaterialTypeToString.end());
	//int foundIdxBaseMaterial = foundItBaseMaterial - Rendering::BaseMaterialTypeToString.begin();
	//m_baseMaterialType = (Rendering::BaseMaterialType)foundIdxBaseMaterial;

	// Material Type
	auto foundItMaterialType = std::find(Rendering::MaterialTypeToString.begin(), Rendering::MaterialTypeToString.end(), root.get("materialType", "").asString());
	assert(foundItMaterialType != Rendering::MaterialTypeToString.end());
	int foundIdxMaterialType = foundItMaterialType - Rendering::MaterialTypeToString.begin();
	m_materialType = (Rendering::MaterialType)foundIdxMaterialType;

	m_usage = (Rendering::MaterialUsage)root.get("materialUsage", 0).asInt();
	m_pipeline = (Rendering::PipelineType)root.get("materialPipeline", 0).asInt();
	m_usedWithReflections = root.get("usedWithReflection", false).asBool();
	m_usedWithSkeleton = root.get("usedWithSkeleton", false).asBool();

	//if (std::find(m_pipelineHandlingTypes.begin(), m_pipelineHandlingTypes.end(), Rendering::PipelineType::DEFERRED_PIPILINE) != m_pipelineHandlingTypes.end()
	//	&& std::find(m_pipelineHandlingTypes.begin(), m_pipelineHandlingTypes.end(), Rendering::PipelineType::FORWARD_PIPELINE) != m_pipelineHandlingTypes.end())
	//{
	//	Json::Value deferredJson = root.get("deferred", "");
	//	assert(!deferredJson.empty());
	//	Json::Value forwardJson = root.get("forward", "");
	//	assert(!forwardJson.empty());

	//	// Deferred : 
	//	std::string vertexShaderName = deferredJson.get("vertex", "").asString();
	//	std::string fragmentShaderName = deferredJson.get("fragment", "").asString();
	//	std::string geometryShaderName = deferredJson.get("geometry", "").asString();

	//	load(Rendering::PipelineType::DEFERRED_PIPILINE, path, vertexShaderName, fragmentShaderName, geometryShaderName);

	//	// Forward : 
	//	vertexShaderName = forwardJson.get("vertex", "").asString();
	//	fragmentShaderName = forwardJson.get("fragment", "").asString();
	//	geometryShaderName = forwardJson.get("geometry", "").asString();

	//	load(Rendering::PipelineType::FORWARD_PIPELINE, path, vertexShaderName, fragmentShaderName, geometryShaderName);
	//}

	if (m_materialType == Rendering::MaterialType::DEFAULT)
	{
		m_programId = makeGLProgram();
	}
	else if (m_materialType == Rendering::MaterialType::INTERNAL)
	{
		const std::string vertexShaderName = root.get("vertex", "").asString();
		const std::string fragmentShaderName = root.get("fragment", "").asString();
		const std::string geometryShaderName = root.get("geometry", "").asString();

		m_programId = makeGLProgramForInternal(path, vertexShaderName, fragmentShaderName, geometryShaderName);
	}
	else
		assert(false && "Wrong material type !");

	// Internal parameters
	Json::Value internalShaderParameters = root["internalShaderParameters"];
	int inernalShaderParameterCount = internalShaderParameters.size();
	
	m_internalShaderParameters.clear();
	for (int i = 0; i < inernalShaderParameterCount; i++)
	{
		assert(internalShaderParameters.isValidIndex(i));

		auto newParameter = MakeNewInternalShaderParameter(internalShaderParameters[i]);
		//newParameter->init(id); //don't forget to init the parameter to get the uniforms
		m_internalShaderParameters.push_back(newParameter);
	}

	// External parameters
	/*Json::Value externalShaderParameters = root["externalShaderParameters"];
	int externalShaderParameterCount = externalShaderParameters.size();

	m_externalShaderParameters.clear();
	for (int i = 0; i < externalShaderParameterCount; i++)
	{
		assert(externalShaderParameters.isValidIndex(i));

		std::string parameterType = externalShaderParameters[i].get("type", "").asString();
		std::string parameterName = externalShaderParameters[i].get("name", "").asString();

		m_externalShaderParameters.push_back(MakeNewExternalShaderParameter(parameterType, parameterName));
	}*/

	PRINT_ERROR("error in shader program initialization.");
}

void ShaderProgram::save(const FileHandler::CompletePath & path)
{
	// Can't save internal materials
	assert(m_materialType == Rendering::MaterialType::DEFAULT);

	std::ofstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load shader program at path : " << path.toString() << std::endl;
		return;
	}
	Json::Value root;

	// Material Type
	root["materialType"] = Rendering::MaterialTypeToString[(int)m_materialType];
	root["materialUsage"] = Rendering::MaterialUsageToString[(int)m_usage];
	root["materialPipeline"] = Rendering::PipelineTypesToString[(int)m_pipeline];
	root["usedWithReflection"] = m_usedWithReflections;
	root["usedWithSkeleton"] = m_usedWithSkeleton;

	if (m_materialType == Rendering::MaterialType::DEFAULT)
	{
		// TODO : save visual scripting infos
	}
	/*else if (m_materialType == Rendering::MaterialType::INTERNAL)
	{
		root["vertex"] = m_vertexShaderName;
		root.get["fragment"] = m_fragmentShaderName;
		root.get["geometry"] = m_geometryShaderName;
	}*/
	else
		assert(false && "Wrong material type !");

	// Internal parameters
	//root["internalShaderParameters"] = Json::Value(Json::arrayValue);
	//for (int i = 0; i < m_internalShaderParameters.size(); i++)
	//{
	//	root["internalShaderParameters"][i] = Json::Value(Json::objectValue);
	//	SaveInternalShaderParameter(internalShaderParameters[i], root["internalShaderParameters"][i]);
	//	//newParameter->init(id); //don't forget to init the parameter to get the uniforms
	//}

	stream << root;
}
//
//void ShaderProgram::load(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath)
//{
//	bool hasVertShader = !vertexShaderPath.empty();
//	bool hasFragShader = !fragmentShaderPath.empty();
//
//	GLuint vertShaderId = 0;
//	GLuint fragShaderId = 0;
//
//	if (hasVertShader)
//		vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, vertexShaderPath.c_str());
//	if (hasFragShader)
//		fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, fragmentShaderPath.c_str());
//
//	GLuint programObject = glCreateProgram();
//	if (hasVertShader)
//		glAttachShader(programObject, vertShaderId);
//	if (hasFragShader)
//		glAttachShader(programObject, fragShaderId);
//
//	glLinkProgram(programObject);
//	if (check_link_error(programObject) < 0)
//		exit(1);
//
//	//check uniform errors : 
//	if (!checkError("Uniforms"))
//		exit(1);
//
//	id = programObject;
//}
//

GLuint ShaderProgram::makeGLProgramForInternal(const FileHandler::CompletePath& shaderFolderPath, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& geometryShaderName)
{
	bool hasVertShader = !vertexShaderName.empty();
	bool hasFragShader = !fragmentShaderName.empty();
	bool hasGeomShader = !geometryShaderName.empty();

	GLuint vertShaderId = 0;
	GLuint fragShaderId = 0;
	GLuint geomShaderId = 0;

	if (hasVertShader)
	{
		const FileHandler::CompletePath& vertexAbsolutePath(shaderFolderPath.getPath().toString() + "/" + vertexShaderName);
		vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, vertexAbsolutePath.c_str());
	}
	if (hasFragShader)
	{
		const FileHandler::CompletePath& fragmentAbsolutePath(shaderFolderPath.getPath().toString() + "/" + fragmentShaderName);
		fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, fragmentAbsolutePath.c_str());
	}
	if (hasGeomShader)
	{
		const FileHandler::CompletePath& geometryAbsolutePath(shaderFolderPath.getPath().toString() + "/" + geometryShaderName);
		geomShaderId = compile_shader_from_file(GL_GEOMETRY_SHADER, geometryAbsolutePath.c_str());
	}

	GLuint programObject = glCreateProgram();
	if (hasVertShader)
		glAttachShader(programObject, vertShaderId);
	if (hasFragShader)
		glAttachShader(programObject, fragShaderId);
	if (hasGeomShader)
		glAttachShader(programObject, geomShaderId);

	glLinkProgram(programObject);
	if (check_link_error(programObject) < 0)
 		exit(1);

	//check uniform errors : 
	ASSERT_GL_ERROR("error in shader program compilation.");

	return programObject;
}

GLuint ShaderProgram::makeGLProgramForDefault(const FileHandler::CompletePath& shaderFolderPath, const std::string& fragmentShaderName)
{
	///////////////////

	// Get path to compute shader params functions
	bool hasFragShader = !fragmentShaderName.empty();

	std::string computeParamsVertexShaderPath;
	std::string computeParamsFragmentShaderPath;

	if (hasFragShader)
	{
		const FileHandler::CompletePath& fragmentAbsolutePath(shaderFolderPath.getPath().toString() + "/" + fragmentShaderName);
		computeParamsFragmentShaderPath = fragmentAbsolutePath.toString();
	}

	///////////////////

	// Fragment shader : 
	std::stringstream vertexShaderStream;
	std::stringstream fragmentShaderStream;

	std::string internalShaderFolderPath(FileHandler::Path(Project::getShaderFolderPath().toString() + "/internals/").toString());

	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header.frag");

	fillShaderStream(fragmentShaderStream, computeParamsFragmentShaderPath);

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "forward.frag");
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "deferred.frag");
	}

	// Vertex shader :
	fillShaderStream(vertexShaderStream, internalShaderFolderPath + "header.vert");

	if (m_usedWithSkeleton)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withSkeleton.vert");
	}
	else
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutSkeleton.vert");
	}

	if (m_usedWithReflections)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withReflection.vert");
	}
	else
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutReflection.vert");
	}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "mesh.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "billboard.vert");
	}
	if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "particles.vert");
	}

	return makeGLProgram(vertexShaderStream.str(), fragmentShaderStream.str());
}

GLuint ShaderProgram::makeGLProgram()
{
	std::stringstream vertexShaderStream;
	std::stringstream fragmentShaderStream;

	std::string internalShaderFolderPath( FileHandler::Path(Project::getShaderFolderPath().toString() + "/internals/").toString() );

	// Fragment shader : 
	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header.frag");

	///// Temporary and hard coded, must be replaced by shader visual scripting : 
	const FileHandler::CompletePath& fragmentAbsolutePath(internalShaderFolderPath + "/" + "defaultComputeShaderParams.frag");
	std::string computeParamsFragmentShaderPath = fragmentAbsolutePath.toString();
	fillShaderStream(fragmentShaderStream, computeParamsFragmentShaderPath);
	///////////

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "forward.frag");
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "deferred.frag");
	}

	// Vertex shader : 
	fillShaderStream(vertexShaderStream, internalShaderFolderPath + "header.vert");
	if (m_usedWithSkeleton)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withSkeleton.vert");
	}
	else
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutSkeleton.vert");
	}

	if (m_usedWithReflections)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withReflection.vert");
	}
	else
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutReflection.vert");
	}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "mesh.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "billboard.vert");
	}
	if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "particles.vert");
	}

	return makeGLProgram(vertexShaderStream.str(), fragmentShaderStream.str());
}

GLuint ShaderProgram::makeGLProgram(const std::string & vertexShaderContent, const std::string & fragmentShaderContent)
{
	GLuint vertexShaderId = compile_shader_from_string(GL_VERTEX_SHADER, vertexShaderContent);
	GLuint fragmentShaderId = compile_shader_from_string(GL_FRAGMENT_SHADER, fragmentShaderContent);


	GLuint programObject = glCreateProgram();
	glAttachShader(programObject, vertexShaderId);
	glAttachShader(programObject, fragmentShaderId);

	glLinkProgram(programObject);
	if (check_link_error(programObject) < 0)
		exit(1);

	//check uniform errors : 
	ASSERT_GL_ERROR("error in shader program compilation.");

	// Avoid memory leaks
	glDetachShader(programObject, vertexShaderId);
	glDetachShader(programObject, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programObject;
}

//bool ShaderProgram::implementPipeline(Rendering::PipelineType pipelineType) const
//{
//	return ids.find(pipelineType) != ids.end();
//}
//
//int ShaderProgram::getImplementedPipelineCount() const
//{
//	return m_pipelineHandlingTypes.size();
//}
//
//Rendering::PipelineType ShaderProgram::getImplementedPipeline(int idx) const
//{
//	assert(idx >= 0 && idx < m_pipelineHandlingTypes.size());
//	return m_pipelineHandlingTypes[idx];
//}
//
//GLuint ShaderProgram::getProgramId(Rendering::PipelineType pipelineType) const
//{
//	assert(ids.find(pipelineType) != ids.end());
//	return ids.find(pipelineType)->second;
//}

void ShaderProgram::LoadMaterialInstance(Material* material) const
{
	//Rendering::PipelineType materialPipelineType = material->getPipelineType();
	//auto& foundProgramId = ids.find(materialPipelineType);
	//assert(foundProgramId != ids.end());

	material->loadFromShaderProgramDatas(m_programId/*foundProgramId->second*/, m_internalShaderParameters/*, m_externalShaderParameters*/);
}

void ShaderProgram::addMaterialRef(Material* ref)
{
	if (std::find(m_materialRefs.begin(), m_materialRefs.end(), ref) != m_materialRefs.end())
		m_materialRefs.push_back(ref);
}

void ShaderProgram::removeMaterialRef(Material* ref)
{
	m_materialRefs.erase(std::remove(m_materialRefs.begin(), m_materialRefs.end(), ref), m_materialRefs.end());
}

const std::vector<std::shared_ptr<InternalShaderParameterBase>>& ShaderProgram::getInternalParameters() const
{
	return m_internalShaderParameters;
}

//const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& ShaderProgram::getExternalParameters() const
//{
//	return m_externalShaderParameters;
//}

Rendering::MaterialType ShaderProgram::getMaterialType() const
{
	return m_materialType;
}

bool getMaterialUsageItem()
{

}

void ShaderProgram::drawInInspector(Scene & scene)
{
	bool dirty = false;

	if (ImGui::Combo("material usage", (int*)&m_usage, [](void* data, int idx, const char** out_text) {
		if (Rendering::MaterialUsageToString.size() > idx) {
			*out_text = Rendering::MaterialUsageToString[idx].c_str(); return true;
		}
		else return false;
	}, nullptr, (int)Rendering::MaterialUsageToString.size()))
	{
		m_usage = (Rendering::MaterialUsage)m_usage;
		dirty = true;
	}

	if (ImGui::Combo("material pipeline", (int*)&m_pipeline, [](void* data, int idx, const char** out_text) {
		if (Rendering::PipelineTypesToString.size() > idx) {
			*out_text = Rendering::PipelineTypesToString[idx].c_str(); return true;
		}
		else return false;
	}, nullptr, (int)Rendering::PipelineTypesToString.size()))
	{
		m_pipeline = (Rendering::PipelineType)m_usage;
		dirty = true;
	}

	if (ImGui::Checkbox("used with reflections", &m_usedWithReflections))
	{
		dirty = true;
	}

	if (ImGui::Checkbox("used with skeleton", &m_usedWithSkeleton))
	{
		dirty = true;
	}

	if (ImGui::Button("Edit"))
	{
		// If the edit window isn't opened
		if (!m_nodeManagerRef)
		{
			// Node manager creation
			m_nodeManagerRef = std::make_shared<MVS::NodeManager>();

			// Node manager loading
			std::ifstream stream;
			stream.open(m_completePath.toString());
			if (!stream.is_open())
			{
				std::cout << "error, can't load shader program at path : " << m_completePath.toString() << std::endl;
				return;
			}
			Json::Value root;
			stream >> root;
			m_nodeManagerRef->load(root["MVS"]);

			// We open the window which will display the node manager
			m_nodeManagerRef = std::make_shared<MVS::NodeManager>();
			Editor::instance().getWindowManager()->addWindow(m_nodeManagerRef);
		}
	}

	if (dirty)
	{
		if (ImGui::Button("Compile"))
		{
			compile();
		}
	}
}

void ShaderProgram::drawRightClicContextMenu()
{
	if (ImGui::Button("create material from this program."))
	{
		ImGui::OpenPopup("AddMaterialPopUp");
	}
}
//
//Material* ShaderProgram::makeNewMaterialInstance(const FileHandler::CompletePath& completePath)
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return new Material3DObject(*this, completePath);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return new MaterialBillboard(*this, completePath);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return new MaterialParticlesCPU(*this, completePath);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return new MaterialReflection(*this, completePath);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//std::shared_ptr<Material> ShaderProgram::makeSharedMaterialInstance(const FileHandler::CompletePath& completePath)
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return std::make_shared<Material3DObject>(*this, completePath);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return std::make_shared<MaterialBillboard>(*this, completePath);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return std::make_shared<MaterialParticlesCPU>(*this, completePath);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return std::make_shared<MaterialReflection>(*this, completePath);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//
//Material* ShaderProgram::makeNewMaterialInstance()
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return new Material3DObject(*this);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return new MaterialBillboard(*this);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return new MaterialParticlesCPU(*this);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return new MaterialReflection(*this);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//std::shared_ptr<Material> ShaderProgram::makeSharedMaterialInstance()
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return std::make_shared<Material3DObject>(*this);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return std::make_shared<MaterialBillboard>(*this);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return std::make_shared<MaterialParticlesCPU>(*this);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return std::make_shared<MaterialReflection>(*this);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//Material* makeNewMaterialInstance(const FileHandler::CompletePath& path)
//{
//	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
//	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);
//
//	std::ifstream stream;
//	stream.open(absolutePath.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
//		return nullptr;
//	}
//	Json::Value root;
//	stream >> root;
//
//	ResourcePtr<ShaderProgram> programPtr;
//	programPtr.load(root["shaderProgramName"]);
//	assert(programPtr.isValid());
//	Rendering::BaseMaterialType baseMaterialType = programPtr->getBaseMaterialType();
//
//	//std::string shaderProgramName = root.get("shaderProgramName", "").asString();
//	//assert(shaderProgramName != "");
//	//ShaderProgram* program = getProgramFactory().get(shaderProgramName);
//	//assert(program != nullptr);
//	//Rendering::BaseMaterialType baseMaterialType = program->getBaseMaterialType();
//
//	switch (baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//	{
//		Material* newMaterial = new Material3DObject();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::BILLBOARD:
//	{
//		Material* newMaterial = new MaterialBillboard();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::PARTICLE:
//	{
//		Material* newMaterial = new MaterialParticlesCPU();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::REFLECTION:
//	{
//		Material* newMaterial = new MaterialReflection();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}