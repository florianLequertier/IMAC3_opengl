#pragma once

#include <math.h>

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"

struct BaseCamera
{
	enum CameraMode { PERSPECTIVE, ORTHOGRAPHIC };

	BaseCamera();

	//virtual void setTranslationLocal(glm::vec3 pos) = 0;
	//virtual void translateLocal(glm::vec3 pos) = 0;
	//virtual void setDirection(glm::vec3 dir) = 0;
	//virtual void setRotation(float phi, float theta) = 0;
	//virtual void setTranslation(glm::vec3 pos) = 0;
	//virtual void translate(glm::vec3 pos) = 0;
	//virtual void rotate(float deltaX, float deltaY) = 0;
	//virtual void updateTransform() = 0;

	virtual void updateScreenSize(float screenWidth, float screenHeight) = 0;
	virtual void setPerspectiveInfos(float fovy, float aspect, float near = 0.1f, float far = 100.f) = 0;
	virtual void setOrthographicInfos(float left, float right, float bottom, float top, float zNear, float zFar) = 0;
	virtual void setCameraMode(CameraMode cameraMode) = 0;

	virtual glm::mat4 getViewMatrix() const = 0;
	virtual glm::mat4 getProjectionMatrix() const = 0;
	virtual glm::vec3 getCameraPosition() const = 0;
	virtual glm::vec3 getCameraForward() const = 0;

	virtual void setFOV(float fov) = 0;
	virtual void setNear(float near) = 0;
	virtual void setFar(float far) = 0;
	virtual void setAspect(float aspect) = 0;

	virtual float getFOV() const = 0;
	virtual float getNear() const = 0;
	virtual float getFar() const = 0;
	virtual float getAspect() const = 0;
};

class Camera : public Component, public BaseCamera
{

private :
	CameraMode m_cameraMode;

	glm::vec3 m_lookPosition;
	glm::vec3 m_position;

	glm::vec3 m_up;
	glm::vec3 m_forward;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	float m_fovy;
	float m_aspect;
	float m_zNear;
	float m_zFar;
	float m_left;
	float m_top;
	float m_right;
	float m_bottom;

public:

	Camera();

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;

	virtual void drawUI(Scene& scene) override;
	virtual void eraseFromScene(Scene & scene) override;
	virtual void addToScene(Scene & scene) override;
	virtual Component * clone(Entity * entity) override;
	virtual void eraseFromEntity(Entity& entity) override;
	virtual void addToEntity(Entity& entity) override;
	
	virtual void updateScreenSize(float screenWidth, float screenHeight) override;
	virtual void setPerspectiveInfos(float fovy, float aspect, float zNear = 0.1f, float zFar = 100.f) override;
	virtual void setOrthographicInfos(float left, float right, float bottom, float top, float zNear, float zFar) override;
	virtual void setCameraMode(CameraMode cameraMode) override;
	virtual glm::mat4 getViewMatrix() const override;
	virtual glm::mat4 getProjectionMatrix() const override;
	virtual glm::vec3 getCameraPosition() const override;
	virtual glm::vec3 getCameraForward() const override;

	virtual void setFOV(float fov) override;
	virtual void setNear(float near) override;
	virtual void setFar(float far) override;
	virtual void setAspect(float aspect) override;
	virtual float getFOV() const override;
	virtual float getNear() const override;
	virtual float getFar() const override;
	virtual float getAspect() const override;

	void updateProjection();

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(Json::Value& rootComponent) override;
};


class CameraEditor : public BaseCamera
{
private:
	float radius;
	float theta;
	float phi;
	glm::vec3 o;
	glm::vec3 eye;

	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 right;

	bool isFPSMode;

	float m_fovy;
	float m_aspect;
	float m_zNear;
	float m_zFar;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

public:
	CameraEditor();

	void setTranslationLocal(glm::vec3 pos);
	void translateLocal(glm::vec3 pos);
	void setDirection(glm::vec3 dir);
	void setRotation(float phi, float theta);
	void setTranslation(glm::vec3 pos);
	void translate(glm::vec3 pos);
	void rotate(float deltaX, float deltaY);
	void updateTransform();
	void setFPSMode(bool fpsMode);

	void updateProjection();

	virtual void updateScreenSize(float screenWidth, float screenHeight) override;
	virtual void setPerspectiveInfos(float fovy, float aspect, float near = 0.1f, float = 100.f) override;
	virtual void setOrthographicInfos(float left, float right, float bottom, float top, float zNear, float zFar) override;
	virtual void setCameraMode(CameraMode cameraMode) override;
	virtual glm::mat4 getViewMatrix() const override;
	virtual glm::mat4 getProjectionMatrix() const override;
	virtual glm::vec3 getCameraPosition() const override;
	virtual glm::vec3 getCameraForward() const override;

	// H�rit� via BaseCamera
	virtual void setFOV(float fov) override;
	virtual void setNear(float near) override;
	virtual void setFar(float far) override;
	virtual void setAspect(float aspect) override;
	virtual float getFOV() const override;
	virtual float getNear() const override;
	virtual float getFar() const override;
	virtual float getAspect() const override;

};


/*
struct CameraFPS : public Camera
{

CameraFPS();
CameraFPS(const Camera& cam);

virtual void setTranslationLocal(glm::vec3 pos) override;
virtual void translateLocal(glm::vec3 pos) override;
virtual void setDirection(glm::vec3 dir) override;
virtual void setRotation(float phi, float theta) override;
virtual void setTranslation(glm::vec3 pos) override;
virtual void translate(glm::vec3 pos) override;
virtual void rotate(float deltaX, float deltaY) override;
virtual void updateTransform() override;
void switchFromCameraEditor(const Camera& other); // make the transition between camera editor and camera fps

virtual Component * clone(Entity * entity) override;

};
*/

/*
void camera_defaults(Camera & c);
void camera_zoom(Camera & c, float factor);
void camera_turn(Camera & c, float phi, float theta);
void camera_pan(Camera & c, float x, float y);
void camera_rotate(Camera& c, float phi, float theta);
void camera_translate(Camera& c, float x, float y, float z);
void toogleCamera(Camera& c);
*/