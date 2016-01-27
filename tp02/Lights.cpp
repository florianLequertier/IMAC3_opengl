#include "Lights.h"
#include "Scene.h"

Light::Light(float _intensity, glm::vec3 _color) : Component(LIGHT), intensity(_intensity), color(_color)
{

}

///////////////////////////////

PointLight::PointLight(float _intensity, glm::vec3 _color, glm::vec3 _position) : Light(_intensity, _color), position(_position)
{
	m_type = POINT_LIGHT;

	updateBoundingBox();
}

void PointLight::updateBoundingBox()
{
	float lightRadius = std::sqrt(intensity / 0.01f);
	
	boundingBox.applyTranslation(position);
	boundingBox.applyScale(glm::vec3(lightRadius, lightRadius, lightRadius));
}

void PointLight::drawUI()
{
	if (ImGui::CollapsingHeader("point light"))
	{
		ImGui::SliderFloat("light intensity", &intensity, 0.f, 50.f);
		ImGui::ColorEdit3("light color", &color[0]);
		//ImGui::SliderFloat3("light position", &position[0], -10.f, 10.f);
	}
}

void PointLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	position = translation;

	updateBoundingBox();
}

void PointLight::eraseFromScene(Scene & scene)
{
	scene.erase(this);
}


Component* PointLight::clone(Entity* entity)
{
	PointLight* newPointLight = new PointLight(*this);

	newPointLight->attachToEntity(entity);

	return newPointLight;
}

void PointLight::addToScene(Scene& scene)
{
	scene.add(this);
}

void PointLight::setBoundingBoxVisual(MeshRenderer * visual)
{
	boundingBox.setVisual(visual);
}

void PointLight::renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 color)
{
	boundingBox.render(projectile, view, color);
}


////////////////////////////////

DirectionalLight::DirectionalLight(float _intensity, glm::vec3 _color, glm::vec3 _direction) :
	Light(_intensity, _color), direction(_direction)
{
	m_type = DIRECTIONAL_LIGHT;
}

void DirectionalLight::drawUI()
{
	if (ImGui::CollapsingHeader("directional light"))
	{
		ImGui::SliderFloat("light intensity", &intensity, 0.f, 10.f);
		ImGui::ColorEdit3("light color", &color[0]);
		//ImGui::SliderFloat3("light direction", &direction[0], 0.f, 1.f);
	}
}

void DirectionalLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	direction = glm::normalize( glm::mat3_cast(rotation) * glm::vec3(0, -1, 0) ); // ???
}

void DirectionalLight::eraseFromScene(Scene & scene)
{
	scene.erase(this);
}

Component* DirectionalLight::clone(Entity* entity)
{
	DirectionalLight* newDirectionalLight = new DirectionalLight(intensity, color, direction);

	newDirectionalLight->attachToEntity(entity);

	return newDirectionalLight;
}

void DirectionalLight::addToScene(Scene& scene)
{
	scene.add(this);
}

////////////////////////////////////

SpotLight::SpotLight(float _intensity, glm::vec3 _color, glm::vec3 _position, glm::vec3 _direction, float _angle) :
	Light(_intensity, _color), position(_position), direction(_direction), angle(_angle)
{
	m_type = SPOT_LIGHT;

	updateBoundingBox();
}

void SpotLight::updateBoundingBox()
{
	float lightRadius = std::sqrt(intensity / 0.01f);

	boundingBox.applyTranslation(position);
	boundingBox.applyScale(glm::vec3(lightRadius, lightRadius, lightRadius));
}

void SpotLight::drawUI()
{
	if (ImGui::CollapsingHeader("spot light"))
	{
		ImGui::SliderFloat("light intensity", &intensity, 0.f, 50.f);
		ImGui::ColorEdit3("light color", &color[0]);
		//ImGui::SliderFloat3("light position", &position[0], -10.f, 10.f);
		//ImGui::SliderFloat3("light direction", &direction[0], -1.f, 1.f);
		ImGui::SliderFloat("light angles", &angle, 0.f, glm::pi<float>());
	}
}

void SpotLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	position = translation;
	direction = glm::normalize(glm::mat3_cast(rotation) * glm::vec3(0, -1, 0));

	updateBoundingBox();
}

void SpotLight::eraseFromScene(Scene & scene)
{
	scene.erase(this);
}

Component* SpotLight::clone(Entity* entity)
{
	SpotLight* newSpotLight = new SpotLight(intensity, color, position, direction, angle);

	newSpotLight->attachToEntity(entity);

	return newSpotLight;
}

void SpotLight::addToScene(Scene& scene)
{
	scene.add(this);
}

void SpotLight::setBoundingBoxVisual(MeshRenderer * visual)
{
	boundingBox.setVisual(visual);
}

void SpotLight::renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 color)
{
	boundingBox.render(projectile, view, color);
}