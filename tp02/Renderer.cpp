#include "Renderer.h"
#include "Factories.h" //forward

Renderer::Renderer(LightManager* _lightManager, std::string programGPass_vert_path, std::string programGPass_frag_path, std::string programLightPass_vert_path, std::string programLightPass_frag_path_pointLight, std::string programLightPass_frag_path_directionalLight, std::string programLightPass_frag_path_spotLight)  : quadMesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES), 2)
{

	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();

	////////////////////// INIT QUAD MESH ////////////////////////
	quadMesh.triangleIndex = { 0, 1, 2, 2, 1, 3 };
	quadMesh.vertices = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };
	quadMesh.initGl();


	//////////////////// 3D lightPass point light shader ////////////////////////
	// Try to load and compile shaders
	GLuint vertShaderId_lightPass_point = compile_shader_from_file(GL_VERTEX_SHADER, programLightPass_vert_path.c_str());
	GLuint fragShaderId_lightPass_point = compile_shader_from_file(GL_FRAGMENT_SHADER, programLightPass_frag_path_pointLight.c_str());

	glProgram_lightPass_pointLight = glCreateProgram();
	glAttachShader(glProgram_lightPass_pointLight, vertShaderId_lightPass_point);
	glAttachShader(glProgram_lightPass_pointLight, fragShaderId_lightPass_point);

	glLinkProgram(glProgram_lightPass_pointLight);
	if (check_link_error(glProgram_lightPass_pointLight) < 0)
		exit(1);

	uniformTexturePosition[POINT] = glGetUniformLocation(glProgram_lightPass_pointLight, "ColorBuffer");
	uniformTextureNormal[POINT] = glGetUniformLocation(glProgram_lightPass_pointLight, "NormalBuffer");
	uniformTextureDepth[POINT] = glGetUniformLocation(glProgram_lightPass_pointLight, "DepthBuffer");
	unformScreenToWorld[POINT] = glGetUniformLocation(glProgram_lightPass_pointLight, "ScreenToWorld");
	uniformCameraPosition[POINT] = glGetUniformLocation(glProgram_lightPass_pointLight, "CameraPosition");

	//check uniform errors : 
	if (!checkError("Uniforms"))
		exit(1);

	//////////////////// 3D lightPass directional light shader ////////////////////////
	// Try to load and compile shaders
	GLuint vertShaderId_lightPass_dir = compile_shader_from_file(GL_VERTEX_SHADER, programLightPass_vert_path.c_str());
	GLuint fragShaderId_lightPass_dir = compile_shader_from_file(GL_FRAGMENT_SHADER, programLightPass_frag_path_directionalLight.c_str());

	glProgram_lightPass_directionalLight = glCreateProgram();
	glAttachShader(glProgram_lightPass_directionalLight, vertShaderId_lightPass_dir);
	glAttachShader(glProgram_lightPass_directionalLight, fragShaderId_lightPass_dir);

	glLinkProgram(glProgram_lightPass_directionalLight);
	if (check_link_error(glProgram_lightPass_directionalLight) < 0)
		exit(1);

	uniformTexturePosition[DIRECTIONAL] = glGetUniformLocation(glProgram_lightPass_directionalLight, "ColorBuffer");
	uniformTextureNormal[DIRECTIONAL] = glGetUniformLocation(glProgram_lightPass_directionalLight, "NormalBuffer");
	uniformTextureDepth[DIRECTIONAL] = glGetUniformLocation(glProgram_lightPass_directionalLight, "DepthBuffer");
	unformScreenToWorld[DIRECTIONAL] = glGetUniformLocation(glProgram_lightPass_directionalLight, "ScreenToWorld");
	uniformCameraPosition[DIRECTIONAL] = glGetUniformLocation(glProgram_lightPass_directionalLight, "CameraPosition");

	//check uniform errors : 
	if (!checkError("Uniforms"))
		exit(1);

	//////////////////// 3D lightPass spot light shaders ////////////////////////
	// Try to load and compile shaders
	GLuint vertShaderId_lightPass_spot = compile_shader_from_file(GL_VERTEX_SHADER, programLightPass_vert_path.c_str());
	GLuint fragShaderId_lightPass_spot = compile_shader_from_file(GL_FRAGMENT_SHADER, programLightPass_frag_path_spotLight.c_str());

	glProgram_lightPass_spotLight = glCreateProgram();
	glAttachShader(glProgram_lightPass_spotLight, vertShaderId_lightPass_spot);
	glAttachShader(glProgram_lightPass_spotLight, fragShaderId_lightPass_spot);

	glLinkProgram(glProgram_lightPass_spotLight);
	if (check_link_error(glProgram_lightPass_spotLight) < 0)
		exit(1);

	uniformTexturePosition[SPOT] = glGetUniformLocation(glProgram_lightPass_spotLight, "ColorBuffer");
	uniformTextureNormal[SPOT] = glGetUniformLocation(glProgram_lightPass_spotLight, "NormalBuffer");
	uniformTextureDepth[SPOT] = glGetUniformLocation(glProgram_lightPass_spotLight, "DepthBuffer");
	unformScreenToWorld[SPOT] = glGetUniformLocation(glProgram_lightPass_spotLight, "ScreenToWorld");
	uniformCameraPosition[SPOT] = glGetUniformLocation(glProgram_lightPass_spotLight, "CameraPosition");

	//check uniform errors : 
	if (!checkError("Uniforms"))
		exit(1);

	//////////////////// 3D Gpass shaders ////////////////////////
	// Try to load and compile shaders
	GLuint vertShaderId_gpass = compile_shader_from_file(GL_VERTEX_SHADER, programGPass_vert_path.c_str());
	GLuint fragShaderId_gpass = compile_shader_from_file(GL_FRAGMENT_SHADER, programGPass_frag_path.c_str());

	glProgram_gPass = glCreateProgram();
	glAttachShader(glProgram_gPass, vertShaderId_gpass);
	glAttachShader(glProgram_gPass, fragShaderId_gpass);

	glLinkProgram(glProgram_gPass);
	if (check_link_error(glProgram_gPass) < 0)
		exit(1);

	//check uniform errors : 
	if (!checkError("Uniforms"))
		exit(1);

	//////////////////// INITIALIZE FRAME BUFFER ///////////////////

	glGenTextures(3, gbufferTextures);
	// 2 draw buffers for color and normal
	GLuint gbufferDrawBuffers[2];

	// Create color texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create normal texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create depth texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create Framebuffer Object
	glGenFramebuffers(1, &gbufferFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
	// Initialize DrawBuffers
	gbufferDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	gbufferDrawBuffers[1] = GL_COLOR_ATTACHMENT1;
	glDrawBuffers(2, gbufferDrawBuffers);

	// Attach textures to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbufferTextures[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbufferTextures[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbufferTextures[2], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Error on building framebuffer\n");
		exit(EXIT_FAILURE);
	}

	// Back to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	////////////////////// LIGHT MANAGER /////////////////////////
	lightManager = _lightManager;
	lightManager->init(glProgram_lightPass_pointLight, glProgram_lightPass_directionalLight, glProgram_lightPass_spotLight);

}

void Renderer::onResizeWindow()
{
	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();

	// unbind texture of FBO
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//delete old textures
	glDeleteTextures(3, gbufferTextures);

	//generate new textures
	glGenTextures(3, gbufferTextures);

	// Create color texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create normal texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create depth texture
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Update Framebuffer Object with new textures
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

	// Attach textures to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbufferTextures[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbufferTextures[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbufferTextures[2], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Error on building framebuffer\n");
		exit(EXIT_FAILURE);
	}

	// Back to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initPostProcessQuad(std::string programBlit_vert_path, std::string programBlit_frag_path)
{
	//////////////////// BLIT shaders ////////////////////////
	GLuint vertShaderId_blit = compile_shader_from_file(GL_VERTEX_SHADER, programBlit_vert_path.c_str());
	GLuint fragShaderId_blit = compile_shader_from_file(GL_FRAGMENT_SHADER, programBlit_frag_path.c_str());

	glProgram_blit = glCreateProgram();
	glAttachShader(glProgram_blit, vertShaderId_blit);
	glAttachShader(glProgram_blit, fragShaderId_blit);

	glLinkProgram(glProgram_blit);
	if (check_link_error(glProgram_blit) < 0)
		exit(1);

	//check uniform errors : 
	if (!checkError("Uniforms"))
		exit(1);

	// Upload uniforms
	uniformTextureBlit = glGetUniformLocation(glProgram_blit, "Texture");
}

/*
void Renderer::render(Camera& camera, std::vector<Entity*> entities)
{

	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();


	////////////////////////// begin scene rendering 

	// Viewport 
	glViewport(0, 0, width, height);

	// Clear default buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	///////////// begin draw world

	//////// begin deferred

	////// begin matrix updates

	// update values
	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 worldToView = glm::lookAt(camera.eye, camera.o, camera.up);
	glm::mat4 vp = projection * worldToView;
	glm::mat4 screenToWorld = glm::transpose(glm::inverse(vp));

	///// end matrix updates

	////// begin G pass 

	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

	// Default states
	glEnable(GL_DEPTH_TEST);

	// Clear the front buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < entities.size(); i++)
	{
		glm::mat4 modelMatrix = entities[i]->getModelMatrix(); //get modelMatrix
		glm::mat4 mv = worldToView * modelMatrix;
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(mv));
		glm::mat4 mvp = projection * worldToView * modelMatrix;

		entities[i]->meshRenderer->material->use();
		entities[i]->meshRenderer->material->setUniform_MVP(mvp);
		entities[i]->meshRenderer->material->setUniform_normalMatrix(normalMatrix);

		entities[i]->meshRenderer->mesh->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////// end G pass

	///// begin light pass

	glUseProgram(glProgram_lightPass);

	// send screen to world matrix : 
	glUniformMatrix4fv(unformScreenToWorld, 1, false, glm::value_ptr(screenToWorld));
	glUniform3fv(uniformCameraPosition, 1, glm::value_ptr(camera.eye));

	//for lighting : 
	lightManager->renderLights();

	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

	glUniform1i(uniformTexturePosition, 0);
	glUniform1i(uniformTextureNormal, 1);
	glUniform1i(uniformTextureDepth, 2);

	// Render quad
	quadMesh.draw();

	///// end light pass

	///////// end deferred

	///////// begin forward 

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbufferFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//draw collider in forward rendering pass (no lightning)
	for (int i = 0; i < entities.size(); i++)
	{
		if (entities[i]->collider == nullptr)
			continue;

		glm::vec3 colliderColor(1, 0, 0);
		if (entities[i]->getIsSelected())
			colliderColor = glm::vec3(1, 1, 0);

		entities[i]->collider->render(projection, worldToView, colliderColor);
	}


	///////// end forward


	///////////// end draw world


	///////////// begin draw blit quad
	glDisable(GL_DEPTH_TEST);

	glUseProgram(glProgram_blit);

	for (int i = 0; i < 3; i++)
	{
		glViewport((width * i) / 3, 0, width / 3, height / 4);

		glActiveTexture(GL_TEXTURE0);
		// Bind gbuffer color texture
		glBindTexture(GL_TEXTURE_2D, gbufferTextures[i]);
		glUniform1i(uniformTextureBlit, 0);

		quadMesh.draw();
	}

	glViewport(0, 0, width, height);

	///////////// end draw blit quad



	//////////////////////// end scene rendering


}
*/

void Renderer::render(const Camera& camera, std::vector<MeshRenderer*>& meshRenderers, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, Terrain& terrain, Skybox& skybox)
{
	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();


	////////////////////////// begin scene rendering 

	// Viewport 
	glViewport(0, 0, width, height);

	// Clear default buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	///////////// begin draw world

	//////// begin deferred

	////// begin matrix updates

	// update values
	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 worldToView = glm::lookAt(camera.eye, camera.o, camera.up);
	glm::mat4 camera_mvp = projection * worldToView;
	glm::mat4 screenToWorld = glm::transpose(glm::inverse(camera_mvp));
	
	///// end matrix updates

	////// begin G pass 

	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);

	// Default states
	glEnable(GL_DEPTH_TEST);

	// Clear the front buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < meshRenderers.size(); i++)
	{
		glm::mat4 modelMatrix = meshRenderers[i]->entity()->getModelMatrix(); //get modelMatrix
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
		glm::mat4 mvp = projection * worldToView * modelMatrix;

		meshRenderers[i]->getMaterial()->use();
		meshRenderers[i]->getMaterial()->setUniform_MVP(mvp);
		meshRenderers[i]->getMaterial()->setUniform_normalMatrix(normalMatrix);

		meshRenderers[i]->getMesh()->draw();
	}

	terrain.render(projection, worldToView);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////// end G pass

	///// begin light pass
	// Disable the depth test
	glDisable(GL_DEPTH_TEST);
	// Enable blending
	glEnable(GL_BLEND);
	// Setup additive blending
	glBlendFunc(GL_ONE, GL_ONE);



	// Render quad
	//for lighting : 
	//lightManager->renderLights(pointLights, directionalLights, spotLights);
	
	//point light : 
	glUseProgram(glProgram_lightPass_pointLight);

	// send screen to world matrix : 
	glUniformMatrix4fv(unformScreenToWorld[POINT], 1, false, glm::value_ptr(screenToWorld));
	glUniform3fv(uniformCameraPosition[POINT], 1, glm::value_ptr(camera.eye));

	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

	glUniform1i(uniformTexturePosition[POINT], 0);
	glUniform1i(uniformTextureNormal[POINT], 1);
	glUniform1i(uniformTextureDepth[POINT], 2);
	for (int i = 0; i < pointLights.size(); i++)
	{
		if (passCullingTest(projection, worldToView, camera.eye, pointLights[i]->boundingBox)) // optimisation test
		{
			lightManager->uniformPointLight(*pointLights[i]);
			quadMesh.draw();
		}
	}

	//spot lights : 
	glUseProgram(glProgram_lightPass_spotLight);

	// send screen to world matrix : 
	glUniformMatrix4fv(unformScreenToWorld[SPOT], 1, false, glm::value_ptr(screenToWorld));
	glUniform3fv(uniformCameraPosition[SPOT], 1, glm::value_ptr(camera.eye));

	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

	glUniform1i(uniformTexturePosition[SPOT], 0);
	glUniform1i(uniformTextureNormal[SPOT], 1);
	glUniform1i(uniformTextureDepth[SPOT], 2);
	for (int i = 0; i < spotLights.size(); i++)
	{
		if (passCullingTest(projection, worldToView, camera.eye, spotLights[i]->boundingBox)) // optimisation test
		{
			lightManager->uniformSpotLight(*spotLights[i]);
			quadMesh.draw();
		}
	}

	//make sure that the blit quat cover all the screen : 
	quadMesh.vertices = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };
	// update in CG : 
	glBindBuffer(GL_ARRAY_BUFFER, quadMesh.vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, quadMesh.vertices.size() * sizeof(float), &(quadMesh.vertices)[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//directionals : 
	glUseProgram(glProgram_lightPass_directionalLight);
	// send screen to world matrix : 
	glUniformMatrix4fv(unformScreenToWorld[DIRECTIONAL], 1, false, glm::value_ptr(screenToWorld));
	glUniform3fv(uniformCameraPosition[DIRECTIONAL], 1, glm::value_ptr(camera.eye));

	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

	glUniform1i(uniformTexturePosition[DIRECTIONAL], 0);
	glUniform1i(uniformTextureNormal[DIRECTIONAL], 1);
	glUniform1i(uniformTextureDepth[DIRECTIONAL], 2);
	for (auto& light : directionalLights)
	{
		lightManager->uniformDirectionalLight(*light);
		quadMesh.draw();
	}

	// Disable blending
	glDisable(GL_BLEND);	
	glEnable(GL_DEPTH_TEST);

	///// end light pass

	///////// end deferred


	///////////// end draw world


	//////////////////////// end scene rendering

	///////// turn on forward rendering : 

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbufferFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	//rander skybox : 
	skybox.render(projection, worldToView);
}

void Renderer::debugDrawColliders(const Camera& camera, const std::vector<Entity*>& entities)
{
	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();

	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = glm::lookAt(camera.eye, camera.o, camera.up);
	glm::mat4 vp = projection * view;
	glm::mat4 screenToWorld = glm::transpose(glm::inverse(vp));

	//draw collider in forward rendering pass (no lightning)
	for (int i = 0; i < entities.size(); i++)
	{
		Collider* collider = static_cast<Collider*>(entities[i]->getComponent(Component::ComponentType::COLLIDER));
		if (collider == nullptr)
			continue;

		glm::vec3 colliderColor(1, 0, 0);
		if (entities[i]->getIsSelected())
			colliderColor = glm::vec3(1, 1, 0);

		collider->render(projection, view, colliderColor);
	}
}

void Renderer::debugDrawDeferred()
{
	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();

	///////////// begin draw blit quad
	glDisable(GL_DEPTH_TEST);

	glUseProgram(glProgram_blit);

	for (int i = 0; i < 3; i++)
	{
		glViewport((width * i) / 3, 0, width / 3, height / 4);

		glActiveTexture(GL_TEXTURE0);
		// Bind gbuffer color texture
		glBindTexture(GL_TEXTURE_2D, gbufferTextures[i]);
		glUniform1i(uniformTextureBlit, 0);

		quadMesh.draw();
	}

	glViewport(0, 0, width, height);

	///////////// end draw blit quad
}

void Renderer::debugDrawLights(const Camera& camera, const std::vector<PointLight*>& pointLights, const std::vector<SpotLight*>& spotLights)
{
	int width = Application::get().getWindowWidth();
	int height = Application::get().getWindowHeight();

	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = glm::lookAt(camera.eye, camera.o, camera.up);

	for (auto& light : pointLights)
	{
		light->renderBoundingBox(projection, view, glm::vec3(0, 0, 1));
	}

	for (auto& light : spotLights)
	{
		light->renderBoundingBox(projection, view, glm::vec3(0, 0, 1));
	}
}

void Renderer::updateCulling(const Camera& camera, std::vector<PointLight*>& pointLights, std::vector<SpotLight*>& spotLights)
{
	pointLightCount = 0;
	spotLightCount = 0;

	int width = Application::get().getWindowWidth();
	int height = Application::get().getWindowHeight();

	glm::mat4 projection = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = glm::lookAt(camera.eye, camera.o, camera.up);

	int lastId = pointLights.size() - 1;;
	for (int i = 0; i < pointLights.size(); i++)
	{
		
		BoxCollider& collider = pointLights[i]->boundingBox;
		glm::vec3 topRight = collider.topRight;
		glm::vec3 bottomLeft = collider.bottomLeft;


		if ( !(camera.eye.x > bottomLeft.x && camera.eye.x < topRight.x && camera.eye.y > bottomLeft.y && camera.eye.y < topRight.y && camera.eye.z > bottomLeft.z && camera.eye.z < topRight.z))
		{
			// permet au collider d'�tre toujours face � la camera. 
			//cela permet d'�viter les erreurs du au caract�re "AABB" du collider.
			//en effet, on stock le collider avec une seule diagonale, il faut qu'une fois projet� en rep�re ecrant cette diagonale soit aussi celle du carr� projet�.
			glm::vec3 camToCollider = glm::normalize(collider.translation - camera.eye);
			glm::mat4 facingCameraRotation = glm::lookAt(camToCollider, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			facingCameraRotation = glm::inverse(glm::mat4(facingCameraRotation));

			//collider.applyRotation(glm::quat(facingCameraRotation));

			glm::vec4 tmpTopRight = projection * view  /* facingCameraRotation */* glm::vec4(topRight, 1);
			glm::vec4 tmpBottomLeft = projection * view  /* facingCameraRotation */* glm::vec4(bottomLeft, 1);

			topRight = glm::vec3(tmpTopRight.x / tmpTopRight.w, tmpTopRight.y / tmpTopRight.w, tmpTopRight.z / tmpTopRight.w);
			bottomLeft = glm::vec3(tmpBottomLeft.x / tmpBottomLeft.w, tmpBottomLeft.y / tmpBottomLeft.w, tmpBottomLeft.z / tmpBottomLeft.w);

			if ((topRight.x < -1 && bottomLeft.x < -1) || (topRight.y < -1 && bottomLeft.y < -1)
				|| (topRight.x > 1 && bottomLeft.x > 1) || (topRight.y > 1 && bottomLeft.y > 1))
			{
				PointLight* tmpLight = pointLights[i];
				pointLights[i] = pointLights[lastId];
				pointLights[lastId] = tmpLight;

				lastId--;
				i--;
			}
		}

		if (i >= lastId)
		{
			pointLightCount = i + 1;
			std::cout << "nombre de point light visibles : " << pointLightCount << std::endl;
			break;
		}
	}

	lastId = spotLights.size() - 1;
	for (int i = 0; i < spotLights.size(); i++)
	{

		BoxCollider& collider = spotLights[i]->boundingBox;

		// this matrix will allow the light bounding box to allways facing the camera.
		glm::vec3 camToCollider = glm::normalize(collider.translation - camera.eye);
		glm::mat4 facingCameraRotation = glm::lookAt(camToCollider, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		facingCameraRotation = glm::inverse(glm::mat4(facingCameraRotation));

		collider.applyRotation(glm::quat(facingCameraRotation));

		glm::vec3 topRight = collider.topRight;
		glm::vec3 bottomLeft = collider.bottomLeft;

		glm::vec4 tmpTopRight = projection * view  * glm::vec4(topRight, 1);
		glm::vec4 tmpBottomLeft = projection * view  * glm::vec4(bottomLeft, 1);

		topRight = glm::vec3(tmpTopRight.x / tmpTopRight.w, tmpTopRight.y / tmpTopRight.w, tmpTopRight.z / tmpTopRight.w);
		bottomLeft = glm::vec3(tmpBottomLeft.x / tmpBottomLeft.w, tmpBottomLeft.y / tmpBottomLeft.w, tmpBottomLeft.z / tmpBottomLeft.w);

		if ((topRight.x < -1 && bottomLeft.x < -1) || (topRight.y < -1 && bottomLeft.y < -1)
			|| (topRight.x > 1 && bottomLeft.x > 1) || (topRight.y > 1 && bottomLeft.y > 1))
		{
			SpotLight* tmpLight = spotLights[i];
			spotLights[i] = spotLights[lastId];
			spotLights[lastId] = tmpLight;

			lastId--;
			i--;
		}

		if (i >= lastId)
		{
			spotLightCount = i + 1;
			std::cout << "nombre de spot light visibles : " << spotLightCount << std::endl;
			break;
		}
	}

}


bool Renderer::passCullingTest(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 cameraPosition, BoxCollider& collider)
{
	glm::vec3 topRight = collider.topRight;
	glm::vec3 bottomLeft = collider.bottomLeft;

	bool insideFrustum = false;

	float maxX = 1;
	float maxY = 1;
	float minX = -1;
	float minY = -1;

	//first optimisation : test if the light is inside camera frustum 

	//is camera outside light bounding box ? 
	if (!(cameraPosition.x > bottomLeft.x && cameraPosition.x < topRight.x &&
		cameraPosition.y > bottomLeft.y && cameraPosition.y < topRight.y &&
		cameraPosition.z > bottomLeft.z && cameraPosition.z < topRight.z))
	{
		// ce code semble plus optimis� que celui retenu, mais je n'ai pas r�ussi � le faire marcher convenablement pour deux raisons : 
		// le carr� g�n�r� est trop petit, il n'englobe pas tout le champs d'action de la lumi�re
		// il y a des bugs lorsqu'on place la camera au dessus ou en dessous de la lumi�re, la rotation s'effectue mal et le collider se deforme.
		/*
		// permet au collider d'�tre toujours face � la camera. 
		//cela permet d'�viter les erreurs du au caract�re "AABB" du collider.
		//en effet, on stock le collider avec une seule diagonale (deux points), il faut qu'une fois projet� en rep�re ecrant cette diagonale soit aussi celle du carr� projet�.
		glm::vec3 camToCollider = glm::normalize(collider.translation - cameraPosition);
		glm::mat4 facingCameraRotation = glm::lookAt(camToCollider, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		facingCameraRotation = glm::inverse(glm::mat4(facingCameraRotation));

		collider.applyRotation(glm::quat(facingCameraRotation));

		glm::vec4 tmpTopRight = projection * view  * glm::vec4(topRight, 1);
		glm::vec4 tmpBottomLeft = projection * view  * glm::vec4(bottomLeft, 1);

		topRight = glm::vec3(tmpTopRight.x / tmpTopRight.w, tmpTopRight.y / tmpTopRight.w, tmpTopRight.z / tmpTopRight.w);
		bottomLeft = glm::vec3(tmpBottomLeft.x / tmpBottomLeft.w, tmpBottomLeft.y / tmpBottomLeft.w, tmpBottomLeft.z / tmpBottomLeft.w);
		//now, topRight and bottom left are in screen space, and their coordinates are between -1 and 1 on all axis.

		//is light outside the camera frustum ? 
		if ((topRight.x < -1 && bottomLeft.x < -1) || (topRight.y < -1 && bottomLeft.y < -1)
			|| (topRight.x > 1 && bottomLeft.x > 1) || (topRight.y > 1 && bottomLeft.y > 1))
		{
			//we don't have to draw the light if its bounding box is outside the camera frustum
			insideFrustum = false;
		}
		*/

		//compute 8 points of 3D collider : 

		float colliderWidth = glm::abs(topRight.x - bottomLeft.x);
		float colliderDepth = glm::abs(topRight.z - bottomLeft.z);

		glm::vec3 colliderPoints[8] = { bottomLeft, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z + colliderDepth), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z + colliderDepth),
										topRight, glm::vec3(topRight.x, topRight.y, topRight.z - colliderDepth), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z - colliderDepth) };

		glm::mat4 vp = projection * view;
		glm::vec4 tmpColliderPoint;
		maxX = -1;
		maxY = -1;
		minX = 1;
		minY = 1;
		for (int i = 0; i < 8; i++)
		{
			tmpColliderPoint = projection * view * glm::vec4(colliderPoints[i], 1);
			colliderPoints[i] = glm::vec3(tmpColliderPoint.x / tmpColliderPoint.w, tmpColliderPoint.y / tmpColliderPoint.w, tmpColliderPoint.z / tmpColliderPoint.w);
		
			if (colliderPoints[i].x > maxX)
				maxX = colliderPoints[i].x;
			if(colliderPoints[i].x < minX)
				minX = colliderPoints[i].x;

			if (colliderPoints[i].y > maxY)
				maxY = colliderPoints[i].y;
			if (colliderPoints[i].y < minY)
				minY = colliderPoints[i].y;
		}

		//is light outside the camera frustum ? 
		if ((maxX < -1 && minX < -1) || (maxY < -1 && minY < -1)
			|| (maxX > 1 && minX > 1) || (maxY > 1 && minY > 1))
		{
			//we don't have to draw the light if its bounding box is outside the camera frustum
			insideFrustum = false;
		}


		//we have to draw the light if its boudning box is inside/intersect the camera frustum
		insideFrustum = true;
	}
	//we draw the light if we are inside its bounding box
	insideFrustum = true;




	//second optimisation : we modify the quad verticies such that it is reduce to the area of the light
	if (insideFrustum)
	{
		float width = maxX - minX;
		float height = maxY - minY;
		quadMesh.vertices = { minX, minY, minX + width, minY, minX, minY + height, minX + width , minY + height };
		// update in CG : 
		glBindBuffer(GL_ARRAY_BUFFER, quadMesh.vbo_vertices);
		glBufferData(GL_ARRAY_BUFFER, quadMesh.vertices.size() * sizeof(float), &(quadMesh.vertices)[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return insideFrustum;
}