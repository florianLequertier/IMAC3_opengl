#include "stdafx.h"

#include "Billboard.h"
//forwards : 
#include "Factories.h"
#include "Scene.h"
#include "SerializeUtils.h"
#include "EditorGUI.h"


Billboard::Billboard(): Component(ComponentType::BILLBOARD), m_translation(0,0,0), m_scale(1,1), m_textureName("default"), m_color(1,1,1,1)
{
	m_quadMesh = getMeshFactory().getDefault("plane");
	m_billboardMaterial = getMaterialFactory().getDefault("billboard");
	m_texture = getTextureFactory().getDefault("default");
	m_texture->initGL();
}


Billboard::~Billboard()
{
	
}

void Billboard::render(const glm::mat4 & projection, const glm::mat4 & view)
{
	glm::mat4 MVP = projection * view;
	//glm::mat4 NormalMatrix = glm::transpose(glm::inverse(glm::mat4(1)));
	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);

	MaterialBillboard* castedBillboardMaterial = static_cast<MaterialBillboard*>(m_billboardMaterial.get()); //TODO essayer d'enlever �a avec l'upgrade du pipeline de rendu

	castedBillboardMaterial->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture->glId);

	castedBillboardMaterial->setUniformMVP(MVP);
	castedBillboardMaterial->setUniformCameraRight(CameraRight);
	castedBillboardMaterial->setUniformCameraUp(CameraUp);
	castedBillboardMaterial->setUniformScale(m_scale);
	castedBillboardMaterial->setUniformTexture(0);
	castedBillboardMaterial->setUniformTranslation(m_translation);
	castedBillboardMaterial->setUniformColor(m_color);

	m_quadMesh->draw();
}

void Billboard::setTranslation(const glm::vec3 & translation)
{
	m_translation = translation;
}

void Billboard::setScale(const glm::vec2 & scale)
{
	m_scale = scale;
}

void Billboard::setColor(const glm::vec4 & color)
{
	m_color = color;
}

glm::vec3 Billboard::getTranslation() const
{
	return glm::vec3();
}

glm::vec2 Billboard::getScale() const
{
	return glm::vec2();
}

glm::vec4 Billboard::getColor() const
{
	return m_color;
}

void Billboard::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	m_scale.x = scale.x;
	m_scale.y = scale.y;

	m_translation = translation;
}

void Billboard::drawUI(Scene & scene)
{
	char texName[100];
	int stringLength = std::min((int)m_textureName.size(), 100);
	m_textureName.copy(texName, stringLength);
	texName[stringLength] = '\0';
	//%NOCOMMIT%
	//if (ImGui::InputText("textureName", texName, 20))
	//{
	//	m_textureName = texName;

	//	if (getTextureFactory().contains(m_textureName))
	//	{
	//		m_texture = getTextureFactory().get(m_textureName);
	//	}
	//}
	EditorGUI::ResourceField(m_texture, "textureName", texName, 100);

	ImGui::ColorEdit4("Color", glm::value_ptr(m_color));
}

void Billboard::eraseFromScene(Scene & scene)
{
	scene.erase(this);
}

Component* Billboard::clone(Entity* entity)
{
	Billboard* billboard = new Billboard(*this);

	billboard->attachToEntity(entity);

	return billboard;
}

void Billboard::addToEntity(Entity& entity)
{
	entity.add(this);
}

void Billboard::eraseFromEntity(Entity& entity)
{
	entity.erase(this);
}

void Billboard::save(Json::Value& rootComponent) const
{
	Component::save(rootComponent);

	rootComponent["translation"] = toJsonValue(m_translation);
	rootComponent["scale"] = toJsonValue(m_scale);
	rootComponent["textureName"] = m_texture->name;
	rootComponent["color"] = toJsonValue(m_color);
}

void Billboard::load(Json::Value& rootComponent)
{
	Component::load(rootComponent);

	m_translation = fromJsonValue<glm::vec3>(rootComponent["translation"], glm::vec3(0, 0, 0));
	m_scale = fromJsonValue<glm::vec2>(rootComponent["scale"], glm::vec2(0, 0));

	m_textureName = rootComponent.get("textureName", "").asString();
	m_texture = getTextureFactory().get(m_textureName);

	m_color = fromJsonValue<glm::vec4>(rootComponent["color"], glm::vec4(1, 1, 1, 1));
}

void Billboard::addToScene(Scene& scene)
{
	scene.add(this);
}