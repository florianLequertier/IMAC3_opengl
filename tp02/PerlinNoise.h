#pragma once

#include  <vector>
#include <time.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Utils.h"


struct Perlin2D
{
	int m_seed;
	float m_persistence;
	int m_samplingOffset;
	int m_octaveCount;
	int m_height;
	int m_maxHeight;
	std::vector<double> m_values;

	Perlin2D(int l, int p, int n, float persistence, int seed = 0);
	
	float getNoiseValue(float x, float y);

	//float getPersistence() const;
	//void setPersistence(float p);

	//int getSamplingOffset() const;
	//void setSamplingOffset(int s);

	//int getOctaveCount() const;
	//void setOctaveCount()

private :
	float getValue2D(int i, int j);
	float getNoise2D(float x, float y);
};

struct NoiseGenerator
{
	float persistence;
	int samplingOffset;
	int octaveCount;
	int height;
	int seed;

	NoiseGenerator();
	Perlin2D generatePerlin2D();
};
