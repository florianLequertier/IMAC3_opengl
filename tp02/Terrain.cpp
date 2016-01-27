#include "Terrain.h"
#include "Factories.h" //forward

Terrain::Terrain(float width, float height, float depth, int subdivision, glm::vec3 offset) : m_width(width), m_height(height), m_depth(depth), m_subdivision(subdivision), m_offset(offset)
{
	diffuseTextureName.reserve(20);
	diffuseTextureName[0] = '\0';
	specularTextureName.reserve(20);
	specularTextureName[0] = '\0';

	float paddingZ = m_depth / (float)m_subdivision;
	float paddingX = m_width / (float)m_subdivision;

	int lineCount = (m_subdivision - 1);
	int rowCount = (m_subdivision - 1);
	m_triangleCount = (m_subdivision - 1) * (m_subdivision - 1) * 2 + 1;

	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
	m_triangleIndex.clear();

	for (int j = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++)
		{
			m_heightMap.push_back(0);

			m_vertices.push_back(i*paddingX + offset.x);
			m_vertices.push_back(offset.y);
			m_vertices.push_back(j*paddingZ + offset.z);

			m_normals.push_back(0);
			m_normals.push_back(1);
			m_normals.push_back(0);

			m_uvs.push_back(i / (float)(subdivision - 1));
			m_uvs.push_back(j / (float)(subdivision - 1));
		}
	}

	for (int i = 0, k = 0; i < m_triangleCount; i++)
	{


		if (i % 2 == 0)
		{
			m_triangleIndex.push_back(k + 0);
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision);


			//m_vertices.push_back(line*paddingX);
			//m_vertices.push_back(row*paddingY);
			//m_vertices.push_back(0);

			//m_vertices.push_back((line + 1)*paddingX);
			//m_vertices.push_back(row*paddingY);
			//m_vertices.push_back(0);

			//m_vertices.push_back(line*paddingX);
			//m_vertices.push_back((row + 1)*paddingY);
			//m_vertices.push_back(0);
		}
		else
		{
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision + 1);
			m_triangleIndex.push_back(k + m_subdivision);

			//m_vertices.push_back((line + 1)*paddingX);
			//m_vertices.push_back(row*paddingY);
			//m_vertices.push_back(0);

			//m_vertices.push_back((line + 1)*paddingX);
			//m_vertices.push_back((row + 1)*paddingY);
			//m_vertices.push_back(0);

			//m_vertices.push_back(line*paddingX);
			//m_vertices.push_back((row + 1)*paddingY);
			//m_vertices.push_back(0);
		}

		if (i % 2 == 0  && i != 0)
			k++;

		if ((k  + 1 )% (m_subdivision) == 0 && i != 0)
		{
			k++;
		}
	}


	initGl();


	applyNoise(m_terrainNoise.generatePerlin2D());

}


Terrain::~Terrain()
{
	if (vbo_index != 0)
		glDeleteBuffers(1, &vbo_index);

	if (vbo_vertices != 0)
		glDeleteBuffers(1, &vbo_vertices);

	if (vbo_uvs != 0)
		glDeleteBuffers(1, &vbo_uvs);

	if (vbo_normals != 0)
		glDeleteBuffers(1, &vbo_normals);

	glDeleteVertexArrays(1, &vao);
}

void Terrain::computeNormals()
{
	glm::vec3 u(0,0,0);
	glm::vec3 v(0, 0, 0);
	glm::vec3 normal(0, 0, 0);

	for (int j = 0, k = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k++)
		{
			if (j - 1 >= 0 && i - 1 >= 0)
			{
				u = vertexFrom3Floats(m_vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				v = vertexFrom3Floats(m_vertices, (i - 1)+ j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				normal += glm::normalize(glm::cross(u, v));
			}

			if (i - 1 >= 0 && j + 1 < (m_subdivision - 1))
			{
				u = vertexFrom3Floats(m_vertices, (i - 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				v = vertexFrom3Floats(m_vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				normal += glm::normalize(glm::cross(u, v));
			}

			if (j + 1 < (m_subdivision - 1) && i+1 < (m_subdivision - 1))
			{
				u = vertexFrom3Floats(m_vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				v = vertexFrom3Floats(m_vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
				normal += glm::normalize(glm::cross(u, v));
			}

			if (i + 1 < (m_subdivision - 1) && j - 1 >= 0)
			{
				u = vertexFrom3Floats(m_vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision); 
				v = vertexFrom3Floats(m_vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);

				normal += glm::normalize(glm::cross(u, v));
			}

			normal = glm::normalize(normal);
		}
	}


	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size()*sizeof(float), &m_normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::applyNoise(Perlin2D& perlin2D)
{
	for (int j = 0, k = 1, l = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k += 3, l++)
		{
			m_heightMap[l] = perlin2D.getNoiseValue(i, j) * 2.f - 1.f;

			m_vertices[k] = m_heightMap[l] * m_height + m_offset.y;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	computeNormals();
}

void Terrain::generateTerrain()
{
	float paddingZ = m_depth / (float)m_subdivision;
	float paddingX = m_width / (float)m_subdivision;

	int lineCount = (m_subdivision - 1);
	int rowCount = (m_subdivision - 1);
	m_triangleCount = (m_subdivision - 1) * (m_subdivision - 1) * 2 + 1;

	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
	m_triangleIndex.clear();
	m_heightMap.clear();

	for (int j = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++)
		{
			m_heightMap.push_back(0);

			m_vertices.push_back(i*paddingX + m_offset.x);
			m_vertices.push_back(m_offset.y);
			m_vertices.push_back(j*paddingZ + m_offset.z);

			m_normals.push_back(0);
			m_normals.push_back(1);
			m_normals.push_back(0);

			m_uvs.push_back(i / (float)(m_subdivision - 1));
			m_uvs.push_back(j / (float)(m_subdivision - 1));
		}
	}

	for (int i = 0, k = 0; i < m_triangleCount; i++)
	{


		if (i % 2 == 0)
		{
			m_triangleIndex.push_back(k + 0);
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision);
		}
		else
		{
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision + 1);
			m_triangleIndex.push_back(k + m_subdivision);
		}

		if (i % 2 == 0 && i != 0)
			k++;

		if ((k + 1) % (m_subdivision) == 0 && i != 0)
		{
			k++;
		}
	}

	initGl();
}

void Terrain::updateTerrain()
{
	float paddingZ = m_depth / (float)m_subdivision;
	float paddingX = m_width / (float)m_subdivision;

	for (int j = 0, k = 0, l = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k += 3, l++)
		{
			m_vertices[k] = i*paddingX + m_offset.x;
			m_vertices[k+1] = m_heightMap[l] * m_height + m_offset.y;
			m_vertices[k+2] = j*paddingZ + m_offset.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//initialize vbos and vao, based on the informations of the mesh.
void Terrain::initGl()
{

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glGenBuffers(1, &vbo_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndex.size()*sizeof(int), &m_triangleIndex[0], GL_STATIC_DRAW);


	glGenBuffers(1, &vbo_vertices);
	glEnableVertexAttribArray(VERTICES);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(VERTICES, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);



	glGenBuffers(1, &vbo_normals);
	glEnableVertexAttribArray(NORMALS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size()*sizeof(float), &m_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(NORMALS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);


	glGenBuffers(1, &vbo_uvs);
	glEnableVertexAttribArray(UVS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size()*sizeof(float), &m_uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(UVS, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 2, (void*)0);


	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// simply draw the vertices, using vao.
void Terrain::render(const glm::mat4& projection, const glm::mat4& view)
{
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1), m_offset);
	glm::mat4 mvp = projection * view * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

	m_material.setUniform_MVP(mvp);
	m_material.setUniform_normalMatrix(normalMatrix);

	m_material.use();

	glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, m_triangleCount * 3, GL_UNSIGNED_INT, (GLvoid*)0);
	glBindVertexArray(0);
}

void Terrain::drawUI()
{

	if (ImGui::InputFloat3("terrain offset", &m_offset[0]))
	{
		updateTerrain();
	}

	glm::vec3 terrainDim(m_width, m_height, m_depth);
	if (ImGui::InputFloat3("terrain dimensions", &terrainDim[0]))
	{
		m_width = terrainDim.x;
		m_height = terrainDim.y;
		m_depth = terrainDim.z;

		updateTerrain();
	}

	if (ImGui::InputInt("terrain subdivision", &m_subdivision))
	{
		generateTerrain();
		applyNoise(m_terrainNoise.generatePerlin2D());
	}

	ImGui::PushID("terrainMaterial");
	m_material.drawUI();
	ImGui::PopID();
/*
	ImGui::InputFloat("specular power", &m_material.specularPower);

	ImGui::InputFloat2("texture repetition", &m_material.textureRepetition[0]);

	char tmpTxt[30];
	diffuseTextureName.copy(tmpTxt, glm::min(30, (int)diffuseTextureName.size()), 0);
	tmpTxt[diffuseTextureName.size()] = '\0';

	if (ImGui::InputText("diffuse texture name", tmpTxt, 20))
	{
		diffuseTextureName = tmpTxt;

		if (TextureFactory::get().contains(diffuseTextureName))
		{
			
			m_material.textureDiffuse = TextureFactory::get().get(diffuseTextureName);
		}
	}

	specularTextureName.copy(tmpTxt, glm::min(30, (int)specularTextureName.size()), 0);
	tmpTxt[specularTextureName.size()] = '\0';
	if (ImGui::InputText("specular texture name", tmpTxt, 20))
	{
		specularTextureName = tmpTxt;

		if (TextureFactory::get().contains(specularTextureName))
			m_material.textureSpecular = TextureFactory::get().get(specularTextureName);
	}
*/
	
	if (ImGui::SliderFloat("noise persistence", &m_terrainNoise.persistence, 0.f, 1.f))
	{
		applyNoise(m_terrainNoise.generatePerlin2D());
	}
	if (ImGui::SliderInt("noise octave count", &m_terrainNoise.octaveCount, 1, 10))
	{
		applyNoise(m_terrainNoise.generatePerlin2D());
	}
	if (ImGui::SliderInt("noise height", &m_terrainNoise.height, 1, 512))
	{
		applyNoise(m_terrainNoise.generatePerlin2D());
	}
	if (ImGui::SliderInt("noise sampling offset", &m_terrainNoise.samplingOffset, 1, 128))
	{
		applyNoise(m_terrainNoise.generatePerlin2D());
	}


	//TODO
}