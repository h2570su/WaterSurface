#include "Object.H"

void aBox::draw(Shader* shader, glm::mat4 model)
{
	if (this->vao == nullptr)
	{
		this->generateVAO();
	}
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(this->vao->vao);

	glDrawElements(GL_TRIANGLES, this->vao->element_amount, GL_UNSIGNED_INT, 0);
	// Unbind VAO
	glBindVertexArray(0);
}

void aBox::generateVAO()
{
	GLfloat  vertices[] = {
				-0.5f ,-0.5f , -0.5f,
				-0.5f ,-0.5f ,  0.5f ,
				 0.5f ,-0.5f ,  0.5f ,
				 0.5f ,-0.5f , -0.5f,

				-0.5f ,0.5f  , -0.5f,
				-0.5f ,0.5f  ,  0.5f,
				 0.5f ,0.5f  ,  0.5f ,
				 0.5f ,0.5f  , -0.5f ,

				 0.5f ,-0.5f  , -0.5f,
				 0.5f ,-0.5f  ,  0.5f,
				 0.5f , 0.5f  ,  0.5f ,
				 0.5f , 0.5f  , -0.5f,

				 -0.5f ,-0.5f  , -0.5f,
				 -0.5f ,-0.5f  ,  0.5f,
				 -0.5f , 0.5f  ,  0.5f ,
				 -0.5f , 0.5f  , -0.5f,

				 -0.5f ,-0.5f  , -0.5f,
				 -0.5f , 0.5f  , -0.5f,
				  0.5f , 0.5f  , -0.5f,
				  0.5f ,-0.5f  , -0.5f,

				  -0.5f ,-0.5f  , 0.5f,
				 -0.5f , 0.5f  , 0.5f,
				  0.5f , 0.5f  , 0.5f ,
				  0.5f ,-0.5f  , 0.5f,

	};
	GLfloat  normal[] = {
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};
	GLfloat  texture_coordinate[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f

	};
	GLfloat  vertexColor[] = {
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	};
	GLuint element[] = {
		0, 1, 2,//UP
		0, 2, 3,
		4, 5, 6,//DN
		4, 6, 7,
		8, 9, 10,//LEFT
		8, 10, 11,
		12, 13, 14,//RIGHT
		12, 14, 15,
		16, 17, 18,//BACK
		16, 18, 19,
		20, 21, 22,//FRONT
		20, 22, 23
	};

	this->vao = new VAO;
	this->vao->element_amount = sizeof(element) / sizeof(GLuint);
	glGenVertexArrays(1, &this->vao->vao);
	glGenBuffers(4, this->vao->vbo);
	glGenBuffers(1, &this->vao->ebo);

	glBindVertexArray(this->vao->vao);

	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture Coordinate attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	// Color attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColor), vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(3);

	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}

void mySphere::draw(Shader * shader, glm::mat4 model)
{
	if (this->vao == nullptr)
	{
		this->generateVAO();
	}
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(this->vao->vao);

	glDrawElements(GL_TRIANGLES, this->vao->element_amount, GL_UNSIGNED_INT, 0);
	// Unbind VAO
	glBindVertexArray(0);
}

void mySphere::generateVAO()
{
	this->vao = new VAO;
	this->vao->element_amount = this->sp.getIndexCount();
	glGenVertexArrays(1, &this->vao->vao);
	glGenBuffers(4, this->vao->vbo);
	glGenBuffers(1, &this->vao->ebo);

	glBindVertexArray(this->vao->vao);

	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, this->sp.getVertexSize(), this->sp.getVertices(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, this->sp.getNormalSize(), this->sp.getNormals(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture Coordinate attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, this->sp.getTexCoordSize(), this->sp.getTexCoords(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	// Color attribute
	GLfloat* colorArr = new GLfloat[this->sp.getIndexCount() * 3];
	for (int i = 0; i < this->sp.getIndexCount() * 3; i += 3)
	{
		colorArr[i] = color3f.x;
		colorArr[i + 1] = color3f.y;
		colorArr[i + 2] = color3f.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, this->sp.getIndexCount() * 3, colorArr, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(3);
	delete[] colorArr;

	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->sp.getIndexSize(), this->sp.getIndices(), GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}

void aPlane::draw(Shader * shader, glm::mat4 model)
{
	if (this->vao == nullptr)
	{
		this->generateVAO();
	}
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(this->vao->vao);

	glDrawElements(GL_TRIANGLES, this->vao->element_amount, GL_UNSIGNED_INT, 0);
	// Unbind VAO
	glBindVertexArray(0);
}

void aPlane::generateVAO()
{
	using namespace std;
	GLfloat  vertices[] = {
		-0.5f ,0.0f , -0.5f,
		-0.5f ,0.0f , 0.5f ,
		0.5f ,0.0f ,0.5f ,
		0.5f ,0.0f ,-0.5f };
	GLfloat  normal[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f };
	GLfloat  texture_coordinate[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f };
	GLuint element[] = {
		0, 1, 2,
		0 ,2, 3
	};

	this->vao = new VAO;
	this->vao->element_amount = sizeof(element) / sizeof(GLuint);
	glGenVertexArrays(1, &this->vao->vao);
	glGenBuffers(4, this->vao->vbo);
	glGenBuffers(1, &this->vao->ebo);

	glBindVertexArray(this->vao->vao);

	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture Coordinate attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	// Color attribute
	GLfloat* colorArr = new GLfloat[sizeof(vertices)];
	for (int i = 0; i < sizeof(vertices) / sizeof(GLfloat); i += 3)
	{
		colorArr[i] = color3f.x;
		colorArr[i + 1] = color3f.y;
		colorArr[i + 2] = color3f.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), colorArr, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(3);
	delete[] colorArr;

	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);

}

void aSurface::draw(Shader * shader, glm::mat4 model)
{
	if (this->vao == nullptr)
	{
		this->generateVAO();
	}
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(this->vao->vao);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawElements(GL_PATCHES, this->vao->element_amount, GL_UNSIGNED_INT, 0);
	// Unbind VAO
	glBindVertexArray(0);
}

void aSurface::generateVAO()
{
	using namespace std;
	GLfloat  sourceVertices[] = {
		-100.0f ,0.0f , -100.0f,
		-100.0f ,0.0f , 100.0f ,
		100.0f ,0.0f ,100.0f ,
		100.0f ,0.0f ,-100.0f };
	GLfloat  sourceNormal[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f };
	GLfloat  sourceTexture_coordinate[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f };
	GLuint sourceElement[] = {
		0, 1, 2,
		0, 2, 3
	};

	int quadLength = ceil(sqrt(this->quadsAmount));
	vector<GLfloat> vertices;
	vector<GLfloat> normal;
	vector<GLfloat> texture_coordinate;
	vector<GLint> element;

	GLfloat VzInc = (sourceVertices[5] - sourceVertices[2]) / quadLength;
	GLfloat VzStart = sourceVertices[2];
	GLfloat VzCurr = VzStart;
	GLfloat TyInc = 1.0f / quadLength;
	GLfloat TyStart = 0.0f;
	GLfloat TyCurr = TyStart;
	for (int i = 0; i < quadLength; i++)
	{
		GLfloat VxInc = (sourceVertices[6] - sourceVertices[0]) / quadLength;
		GLfloat VxStart = sourceVertices[0];
		GLfloat VxCurr = VxStart;

		GLfloat TxInc = 1.0f / quadLength;
		GLfloat TxStart = 0.0f;
		GLfloat TxCurr = TxStart;
		for (int j = 0; j < quadLength; j++)
		{
			vertices.push_back(VxCurr);
			vertices.push_back(sourceVertices[1]);
			vertices.push_back(VzCurr);

			vertices.push_back(VxCurr);
			vertices.push_back(sourceVertices[4]);
			vertices.push_back(VzCurr + VzInc);

			vertices.push_back(VxCurr + VxInc);
			vertices.push_back(sourceVertices[7]);
			vertices.push_back(VzCurr + VzInc);

			vertices.push_back(VxCurr + VxInc);
			vertices.push_back(sourceVertices[10]);
			vertices.push_back(VzCurr);
			for (int k = 0; k < sizeof(sourceNormal) / sizeof(GLfloat); k++)
			{
				normal.push_back(sourceNormal[k]);
			}

			texture_coordinate.push_back(TxCurr);
			texture_coordinate.push_back(TyCurr);

			texture_coordinate.push_back(TxCurr);
			texture_coordinate.push_back(TyCurr + TyInc);

			texture_coordinate.push_back(TxCurr + TxInc);
			texture_coordinate.push_back(TyCurr + TyInc);

			texture_coordinate.push_back(TxCurr + TxInc);
			texture_coordinate.push_back(TyCurr);

			int idx = i * quadLength + j;
			element.push_back(sourceElement[0] + (idx * 4));
			element.push_back(sourceElement[1] + (idx * 4));
			element.push_back(sourceElement[2] + (idx * 4));

			element.push_back(sourceElement[3] + (idx * 4));
			element.push_back(sourceElement[4] + (idx * 4));
			element.push_back(sourceElement[5] + (idx * 4));

			VxCurr += VxInc;
			TxCurr += TxInc;
		}
		VzCurr += VzInc;
		TyCurr += TyInc;
	}


	this->vao = new VAO;
	this->vao->element_amount = element.size();
	glGenVertexArrays(1, &this->vao->vao);
	glGenBuffers(4, this->vao->vbo);
	glGenBuffers(1, &this->vao->ebo);

	glBindVertexArray(this->vao->vao);

	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof(GLfloat), normal.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture Coordinate attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, texture_coordinate.size() * sizeof(GLfloat), texture_coordinate.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	// Color attribute
	GLfloat* colorArr = new GLfloat[vertices.size() * sizeof(GLfloat)];
	for (int i = 0; i < vertices.size(); i += 3)
	{
		colorArr[i] = color3f.x;
		colorArr[i + 1] = color3f.y;
		colorArr[i + 2] = color3f.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), colorArr, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(3);
	delete[] colorArr;

	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, element.size() * sizeof(GLuint), element.data(), GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}

void aBgPlane::draw(Shader * shader, glm::mat4 model)
{
	if (this->vao == nullptr)
	{
		this->generateVAO();
	}
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "u_model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(this->vao->vao);

	glDrawElements(GL_QUADS, this->vao->element_amount, GL_UNSIGNED_INT, 0);
	// Unbind VAO
	glBindVertexArray(0);
}

void aBgPlane::generateVAO()
{
	GLfloat  vertices[] = {
		-1.0f ,1.0f , -1.0f,
		 1.0f ,1.0f , -1.0f ,
		 1.0f ,-1.0f , -1.0f,
		-1.0f ,-1.0f , -1.0f };
	GLfloat  normal[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f };
	GLfloat  texture_coordinate[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f };
	GLuint element[] = {
		0, 1, 2, 3
	};
	this->vao = new VAO;
	this->vao->element_amount = sizeof(element) / sizeof(GLuint);
	glGenVertexArrays(1, &this->vao->vao);
	glGenBuffers(3, this->vao->vbo);
	glGenBuffers(1, &this->vao->ebo);

	glBindVertexArray(this->vao->vao);

	// Position attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// Texture Coordinate attribute
	glBindBuffer(GL_ARRAY_BUFFER, this->vao->vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	//Element attribute
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vao->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}
