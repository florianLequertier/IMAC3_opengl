#include "Animator.h"
//forwards : 
#include "Scene.h"
#include "Factories.h"


Animator::Animator(): Component(ComponentType::ANIMATOR), m_skeleton(nullptr), m_currentAnimName(""), m_currentSkeletonName(""), m_isPlaying(false)
{
}


Animator::~Animator()
{
}

void Animator::setSkeleton(Skeleton * skeleton)
{
	m_skeleton = skeleton;
}

void Animator::addAnimation(SkeletalAnimation * animation)
{
	m_animations[animation->getName()] = animation;
}

void Animator::removeAnimation(SkeletalAnimation * animation)
{
	auto findIt = std::find_if(m_animations.begin(), m_animations.end(), [&animation](const std::pair<std::string, SkeletalAnimation*>& key) { return key.second == animation; });
	m_animations.erase(findIt);
	//m_animations.erase(std::remove(m_animations.begin(), m_animations.end(), animation), m_animations.end());
}

void Animator::updateAnimations(float timeInSecond)
{
	if (m_isPlaying && m_skeleton != nullptr && m_animations.find(m_currentAnimName) != m_animations.end())
		m_skeleton->playAnimationStep(timeInSecond, *m_animations[m_currentAnimName]);
}

void Animator::play()
{
	m_isPlaying = true;
}

void Animator::play(const std::string & animationName)
{
	m_isPlaying = true;
	m_currentAnimName = animationName;
}

void Animator::drawUI(Scene & scene)
{
	char tmpSkeletalName[20];
	m_skeletonName.copy(tmpSkeletalName, m_skeletonName.size());
	tmpSkeletalName[m_skeletonName.size()] = '\0';

	if (ImGui::InputText("skeleton/mesh name", tmpSkeletalName, 20)) {

		m_skeletonName = tmpSkeletalName;

		if (MeshFactory::get().contains(m_skeletonName)) {
			Mesh* tmpMesh = MeshFactory::get().get(m_skeletonName);
			if (tmpMesh != nullptr) {
				m_currentSkeletonName = m_skeletonName;
				m_skeleton = tmpMesh->getSkeleton();
			}
		}
	}


	char tmpAnimationName[60];
	m_animationName.copy(tmpAnimationName, m_animationName.size());
	tmpAnimationName[m_animationName.size()] = '\0';

	if (ImGui::InputText("animation name", tmpAnimationName, 60))
		m_animationName = tmpAnimationName;
	ImGui::SameLine();
	if (ImGui::Button("add")){
		if (SkeletalAnimationFactory::get().contains(m_currentSkeletonName, m_animationName)) {
			SkeletalAnimation* tmpAnim = SkeletalAnimationFactory::get().get(m_currentSkeletonName, m_animationName);
			if (tmpAnim != nullptr) {
				if (m_animations.find(m_currentAnimName) == m_animations.end())
					m_currentAnimName = m_animationName;
				m_animations[m_animationName] = tmpAnim;
			}
		}
	}

	int imguiId = 0;
	for (auto& it = m_animations.begin(); it != m_animations.end();) {

		ImGui::PushID(imguiId);

		ImGui::Text(it->first.c_str());
		ImGui::SameLine();
		if (ImGui::Button("remove")) {
			if (m_currentAnimName == it->first)
				m_currentAnimName = m_animations.size() > 0 ? m_animations.begin()->first : "";
			it = m_animations.erase(it);
		}
		else
			it++;

		ImGui::PopID();
		imguiId++;
	}
}

void Animator::eraseFromScene(Scene & scene)
{
	scene.erase(this);
}

void Animator::addToScene(Scene & scene)
{
	scene.add(this);
}

void Animator::eraseFromEntity(Entity & entity)
{
	entity.erase(this);
}

void Animator::addToEntity(Entity & entity)
{
	entity.add(this);
}

Component * Animator::clone(Entity * entity)
{
	Animator* newAnimator= new Animator(*this);

	newAnimator->attachToEntity(entity);

	return newAnimator;
}

void Animator::save(Json::Value & componentRoot) const
{
	Component::save(componentRoot);

	componentRoot["skeletonName"] = m_currentSkeletonName;

	componentRoot["animationCount"] = m_animations.size();
	int animationId = 0;
	for (auto it = m_animations.begin(); it != m_animations.end(); it++)
		componentRoot["animationName"][animationId] = it->first;

	componentRoot["currentAnimName"] = m_currentAnimName;
}

void Animator::load(Json::Value & componentRoot)
{
	Component::load(componentRoot);

	m_currentSkeletonName = componentRoot.get("skeletonName", "").asString();
	m_skeletonName = m_currentSkeletonName;
	auto mesh = MeshFactory::get().get(m_currentSkeletonName);
	if (mesh != nullptr) {
		m_skeleton = mesh->getSkeleton();
	}

	int animationCount = componentRoot.get("animationCount", 0).asInt();

	for (int i = 0; i < animationCount; i++) {

		std::string animationName = componentRoot["animationName"][i].asString();
		if (SkeletalAnimationFactory::get().contains(m_currentSkeletonName, animationName)) {
			SkeletalAnimation* tmpAnim = SkeletalAnimationFactory::get().get(m_currentSkeletonName, animationName);
			if (tmpAnim != nullptr) {
				if (m_animations.find(m_currentAnimName) == m_animations.end())
					m_currentAnimName = animationName;
				m_animations[animationName] = tmpAnim;
			}
		}
	}

	m_currentAnimName = componentRoot.get("currentAnimName", "").asString();
}
