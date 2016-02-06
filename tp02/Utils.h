#pragma once

#include "glew/glew.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include <vector>
#include <sstream>

#include "Camera.h"

namespace patch
{
	template < typename T > std::string to_string(const T& n)
	{
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}

// Shader utils
int check_link_error(GLuint program);
int check_compile_error(GLuint shader, const char ** sourceBuffer);
GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize);
GLuint compile_shader_from_file(GLenum shaderType, const char * fileName);

// OpenGL utils
bool checkError(const char* title);

//return the point in world coordinate matching the given mouse position in screen space : 
glm::vec3 screenToWorld(float mouse_x, float mouse_y, int width, int height, Camera& camera);

double interpolation_cos2D(double a, double b, double c, double d, double x, double y);
double interpolation_cos1D(double a, double b, double x);

glm::vec3 vertexFrom3Floats(const std::vector<float>& vertices, int indice);

//array :
int idx2DToIdx1D(int i, int j, int array2DWidth);

//physic : 
bool rayOBBoxIntersect(glm::vec3 Start, glm::vec3 Dir, glm::vec3 P, glm::vec3 H[3], glm::vec3 E, float* t);
bool raySlabIntersect(float start, float dir, float min, float max, float* tfirst, float* tlast);
