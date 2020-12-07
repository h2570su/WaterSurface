#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"
#include "RenderUtilities/TextureCube.h"
#include "Sphere.h"
#include <vector>



class mySphere
{
	
public:
	 VAO* vao=nullptr;
	 Texture2D* texture	= nullptr;
	 glm::vec3 color3f = glm::vec3(0.0f, 0.5f, 0.5f);

	 Sphere sp = Sphere(2.0f,1920,36,true);

	 void draw(Shader* shader, glm::mat4 model);
	 void generateVAO();
};

class aBox
{
public:
	 VAO* vao=nullptr;
	 Texture2D* texture	= nullptr;
	 glm::vec3 color3f;

	 

	 void draw(Shader* shader, glm::mat4 model);
	 void generateVAO();
};

class aPlane
{
	public:
	VAO* vao=nullptr;
	 Texture2D* texture	= nullptr;
	 glm::vec3 color3f = glm::vec3(0.3f, 0.3f, 0.3f);

	 

	 void draw(Shader* shader, glm::mat4 model);
	 void generateVAO();
};
class aSurface
{
	public:
	VAO* vao=nullptr;
	 Texture2D* texture	= nullptr;
	 glm::vec3 color3f = glm::vec3(51.0/255, 204.0/255, 1.0);
	 int quadsAmount = 1600;

	 aSurface(int _quadsAmount = 1600)
	 {
		 quadsAmount = _quadsAmount;
	 }
	 

	 void draw(Shader* shader, glm::mat4 model);
	 void generateVAO();
};
class aBgPlane
{
	public:
	VAO* vao=nullptr;
	 Texture2D* texture	= nullptr;
	 glm::vec3 color3f = glm::vec3(0.3f, 0.3f, 0.3f);

	 

	 void draw(Shader* shader, glm::mat4 model);
	 void generateVAO();
};