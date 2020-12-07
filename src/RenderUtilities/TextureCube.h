#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>


class TextureCube
{
public:
	enum Type {
		TEXTURE_DEFAULT = 0,
		TEXTURE_DIFFUSE, TEXTURE_SPECULAR,
		TEXTURE_NORMAL, TEXTURE_DISPLACEMENT,
		TEXTURE_HEIGHT,
	};

	Type type;
	//Right, Left, Top, Bottom, Front, Back
	TextureCube(const char* path[6], Type texture_type = TextureCube::TEXTURE_DEFAULT) :
		type(texture_type)
	{
		glGenTextures(1, &this->id);

		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
		
		for (int i = 0; i < 6; i++)
		{
			cv::Mat img;
			img = cv::imread(path[i], cv::IMREAD_COLOR);

			this->size.x = img.cols;
			this->size.y = img.rows;

			if (img.type() == CV_8UC3)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB8, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
			}
			else if (img.type() == CV_8UC4)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8, img.cols, img.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);
			}
			
			img.release();
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	void bind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
	}
	static void unbind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	glm::ivec2 size;
private:
	GLuint id;

};