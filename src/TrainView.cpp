/************************************************************************
	 File:        TrainView.cpp

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						The TrainView is the window that actually shows the
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within
						a TrainWindow
						that is the outer window with all the widgets.
						The TrainView needs
						to be aware of the window - since it might need to
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know
						about it (beware circular references)

	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"
#include <sstream>
#include <iomanip>


#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif


//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l)
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, 0, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event))
			return 1;

	// remember what button was used
	static int last_push;

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			doPick();
			damage(1);
			return 1;
		};
		break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		return 1;

		// Mouse button drag event
	case FL_DRAG:

		// Compute the new control point position
		if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
			ControlPoint* cp = &m_pTrack->points[selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
				static_cast<double>(cp->pos.x),
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				(Fl::event_state() & FL_CTRL) != 0);

			cp->pos.x = (float)rx;
			cp->pos.y = (float)ry;
			cp->pos.z = (float)rz;
			damage(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;

	case FL_KEYBOARD:
		int k = Fl::event_key();
		int ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;
	}

	return Fl_Gl_Window::handle(event);
}

//From https://shengyu7697.github.io/blog/2020/04/27/Cpp-check-if-file-exists/
bool fileExists(const std::string& path) {
	FILE *fp;
	if (fp = fopen(path.c_str(), "r")) {
		fclose(fp);
		return true;
	}
	return false;
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...

		if (!this->simpleShader)
		{
			this->simpleShader = new
				Shader(
					"../../src/shaders/simple.vert",
					nullptr, nullptr, nullptr,
					"../../src/shaders/simple.frag");
		}
		if (!this->backgroundShader)
		{
			this->backgroundShader = new
				Shader(
					"../../src/shaders/background.vert",
					nullptr, nullptr, nullptr,
					"../../src/shaders/background.frag");
		}
		if (!this->surfaceShader)
		{
			this->surfaceShader = new
				Shader(
					"../../src/shaders/forSurface.vert",
					"../../src/shaders/forSurface.tesc",
					"../../src/shaders/forSurface.tese",
					nullptr,
					"../../src/shaders/forSurface.frag");
		}

		if (!this->commom_matrices)
		{
			this->commom_matrices = new UBO();
			this->commom_matrices->size = 2 * sizeof(glm::mat4);

			glGenBuffers(1, &this->commom_matrices->ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
			glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glGenTextures(1, &reflectTexture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, reflectTexture);
			glGenFramebuffers(6, fbo);
			glGenRenderbuffers(6, rbo);
			for (int i = 0; i < 6; i++)
			{

				glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);


				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, this->pixel_w(), this->pixel_h(),0.1, GL_RGB, GL_UNSIGNED_BYTE, NULL);

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, reflectTexture, 0);

				glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->pixel_w(), this->pixel_h());
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[i]);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}


		if (!this->texture)
			this->texture = new Texture2D("../../Images/church.png");

		if (this->heightmap.empty())
		{
			for (int i = 0; i < 200; i++)
			{
				std::stringstream ss;
				ss << std::setw(3) << std::setfill('0') << i;
				std::string cnt = ss.str();
				std::string str = "../../Images/waves/" + cnt + ".png";
				if (fileExists(str))
				{
					this->heightmap.push_back(new Texture2D((str.c_str())));
				}
				else
				{
					break;
				}
			}
		}

		if (!this->tile)
			this->tile = new Texture2D("../../Images/tiles.jpg");

		if (!this->background)
		{
			const char* paths[6] =
			{
				"../../Images/skybox/right.jpg",
				"../../Images/skybox/left.jpg",
				"../../Images/skybox/top.jpg",
				"../../Images/skybox/bottom.jpg",
				"../../Images/skybox/back.jpg",
				"../../Images/skybox/front.jpg"
			};
			this->background = new TextureCube(paths);
		}

		if (!this->device) {
			//Tutorial: https://ffainelli.github.io/openal-example/
			this->device = alcOpenDevice(NULL);
			if (!this->device)
				puts("ERROR::NO_AUDIO_DEVICE");

			ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_FALSE)
				puts("Enumeration not supported");
			else
				puts("Enumeration supported");

			this->context = alcCreateContext(this->device, NULL);
			if (!alcMakeContextCurrent(context))
				puts("Failed to make context current");

			this->source_pos = glm::vec3(0.0f, 5.0f, 0.0f);

			ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
			alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			alGenSources((ALuint)1, &this->source);
			alSourcef(this->source, AL_PITCH, 1);
			alSourcef(this->source, AL_GAIN, 1.0f);
			alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source, AL_LOOPING, AL_TRUE);

			alGenBuffers((ALuint)1, &this->buffer);

			ALsizei size, freq;
			ALenum format;
			ALvoid* data;
			ALboolean loop = AL_TRUE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)"../WaterSurface/Audios/bounce.wav", &format, &data, &size, &freq, &loop);
			alBufferData(this->buffer, format, data, size, freq);
			alSourcei(this->source, AL_BUFFER, this->buffer);

			if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
				puts("TYPE::STEREO");
			else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
				puts("TYPE::MONO");

			alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}

	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here
#pragma region GL_Light
//######################################################################
// TODO: 
// you might want to set the lighting up differently. if you do, 
// we need to set up the lights AFTER setting up the projection
//######################################################################
// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************




	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);
#pragma endregion
#pragma region OP_AL



	// set linstener position 
	if (selectedCube >= 0)
		alListener3f(AL_POSITION,
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION,
			this->source_pos.x,
			this->source_pos.y,
			this->source_pos.z);

#pragma endregion
	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)

	glUseProgram(0);


	/*setupFloor();
	glDisable(GL_LIGHTING);
	drawFloor(200, 10);*/


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	//glEnable(GL_LIGHTING);
	//setupObjects();

	//drawStuff();

	//// this time drawing is for shadows (except for top view)
	//if (!tw->topCam->value()) {
	//	setupShadows();
	//	drawStuff(true);
	//	unsetupShadows();
	//}

	setViewAndProjToUBO();
	glBindBufferRange(
		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	this->drawBackground();

	//bind shader
	this->simpleShader->Use();
#pragma region simpleShaderLight

	//Lighting------------------------------------------------


	if (this->tw->shadingBrowser->selected(2))
	{
		this->simpleShader->setInt("u_shadingSelect", 1); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(3))
	{
		this->simpleShader->setInt("u_shadingSelect", 2); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(4))
	{
		this->simpleShader->setInt("u_shadingSelect", 3); //0-No 1-Phong 2-Garudond 3-Toon
	}
	else
	{
		this->simpleShader->setInt("u_shadingSelect", 0); //0-No 1-Phong 2-Garudond	
	}
	this->simpleShader->setVec3("u_viewer_pos", this->arcball.getEyePos());

	this->simpleShader->setVec3("dirLights[0].direction", 0.0f, -1.0f, -1.0f);
	this->simpleShader->setVec3("dirLights[0].ambient", 0.2f, 0.2f, 0.2f);
	this->simpleShader->setVec3("dirLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	this->simpleShader->setVec3("dirLights[0].specular", 1.0f, 1.0f, 1.0f);

	this->simpleShader->setVec3("pointLights[0].position", this->lightBoxPos);
	this->simpleShader->setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
	this->simpleShader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	this->simpleShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	this->simpleShader->setFloat("pointLights[0].constant", 1.0f);
	this->simpleShader->setFloat("pointLights[0].linear", 0.01f);
	this->simpleShader->setFloat("pointLights[0].quadratic", 0.0001f);

	this->simpleShader->setVec3("spotLights[0].position", this->arcball.getEyePos());
	this->simpleShader->setVec3("spotLights[0].ambient", 0.2f, 0.2f, 0.2f);
	this->simpleShader->setVec3("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	this->simpleShader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
	this->simpleShader->setFloat("spotLights[0].constant", 1.0f);
	this->simpleShader->setFloat("spotLights[0].linear", 0);
	this->simpleShader->setFloat("spotLights[0].quadratic", 0);
	this->simpleShader->setVec3("spotLights[0].direction", -this->arcball.getEyePos());
	this->simpleShader->setFloat("spotLights[0].cutoff", glm::cos(glm::radians(10.0f)));
	this->simpleShader->setFloat("spotLights[0].outer_cutoff", glm::cos(glm::radians(15.0f)));

	//Lighting------------------------------------------------
#pragma endregion


	this->simpleShaderDraw();


#pragma region disposed



	//glm::vec4 _plane = glm::vec4(0, 1, 0, glm::dot(glm::vec3(0, 1, 0), glm::vec3(0, 0, 0)));
	//glm::mat4 reflectMat;
	//reflectMat[0][0] = -2 * _plane.x*_plane.x + 1;
	//reflectMat[0][1] = -2 * _plane.x * _plane.y;
	//reflectMat[0][2] = -2 * _plane.x * _plane.z;
	//reflectMat[0][3] = -2 * _plane.x * _plane.w;

	//reflectMat[1][0] = -2 * _plane.x * _plane.y;
	//reflectMat[1][1] = -2 * _plane.y * _plane.y + 1;
	//reflectMat[1][2] = -2 * _plane.y * _plane.z;
	//reflectMat[1][3] = -2 * _plane.y * _plane.w;

	//reflectMat[2][0] = -2 * _plane.z * _plane.x;
	//reflectMat[2][1] = -2 * _plane.z * _plane.y;
	//reflectMat[2][2] = -2 * _plane.z * _plane.z + 1;

	//reflectMat[3][0] = 0; reflectMat[3][1] = 0;
	//reflectMat[3][2] = 0; reflectMat[3][3] = 1;
	//
	//float ang = atan2f(this->arcball.getEyePos().x, this->arcball.getEyePos().z);
	//if (ang < 0)
	//{
	//	ang += 2 * 3.1415926;
	//}
	//std::cout << glm::degrees(ang)<<std::endl;

	//glm::mat4 reflectCam;	
	//glGetFloatv(GL_MODELVIEW_MATRIX, &reflectCam[0][0]);	
	//reflectCam = ( reflectCam * glm::transpose(reflectMat));; //TODO wait for vaild
	//glm::rotate(reflectCam, ang, glm::vec3(reflectCam[0], reflectCam[4], reflectCam[8]));


	//auto CamPrime = glm::reflect(this->arcball.getEyePos(), glm::vec3(0, 1, 0));

	//reflectCam = glm::lookAt(CamPrime, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
#pragma endregion
	std::vector<glm::mat4> camMats =
	{
		glm::lookAt(glm::vec3(-100.0f, 0.0f, 0.0f), glm::vec3(100, 0, 0), glm::vec3(0, -1, 0)),
		glm::lookAt(glm::vec3( 100.0f, 0.0f, 0.0f), glm::vec3(-100, 0, 0), glm::vec3(0, -1, 0)),
		glm::lookAt(glm::vec3(0.0f, -100.0f, 0.0f), glm::vec3(0, 100, 0), glm::vec3(0, 0, 1)),
		glm::lookAt(glm::vec3(0.0f,  100.0f, 0.0f), glm::vec3(0, -100, 0), glm::vec3(0, 0, -1)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, -100.0f), glm::vec3(0, 0, 100), glm::vec3(0, -1, 0)),
		glm::lookAt(glm::vec3(0.0f, 0.0f,  100.0f), glm::vec3(0, 0, -100), glm::vec3(0, -1, 0))

	};

	glm::mat4 _projection_matrix = glm::perspective(90.0f, 1.0f, 100.0f, 1000.0f);
	//glGetFloatv(GL_PROJECTION_MATRIX, &_projection_matrix[0][0]);
	for (int i = 0; i < camMats.size(); i++)
	{
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		setViewAndProjToUBO(camMats[i], _projection_matrix);
		glBindBufferRange(
			GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);
		this->drawBackground(camMats[i], glm::perspective(90.0f, 1.0f, .01f, 1000.0f));
		this->simpleShader->Use();
		this->simpleShaderDraw();
	}




	//####################################################################################################
	//Draw Surface unsing indepent shader
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	setViewAndProjToUBO();
	glBindBufferRange(
		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);
	glActiveTexture(GL_TEXTURE0 + 11);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->reflectTexture);
	this->drawSurface();

	glActiveTexture(GL_TEXTURE0 + 11);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);

}

void TrainView::simpleShaderDraw()
{
	//aPlane
	glm::mat4 model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->source_pos);
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));

	this->texture->bind(0);
	glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
	setUseTexture(false);

	//this->plane.draw(this->simpleShader, model_matrix);

	//Light Box
	model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->lightBoxPos);
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));
	setUseTexture(false);
	lightBox.draw(this->simpleShader, model_matrix);

	//Sphere
	model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, glm::vec3(0, 50, 0));
	model_matrix = glm::rotate(model_matrix, 90.0f, glm::vec3(1, 0, 0));
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));
	setUseTexture(true);
	sphere.draw(this->simpleShader, model_matrix);

	//Boxes
	for (int i = 0; i < boxesAmount; i++)
	{
		if (this->boxesPos[i] == glm::vec3())
		{
			this->boxesPos[i] = glm::vec3((rand() % 300) - 150, (rand() % 300) - 150, (rand() % 300) - 150);
		}
		model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->boxesPos[i]);
		model_matrix = glm::rotate(model_matrix, 90.0f, glm::vec3(1, 0, 0));
		model_matrix = glm::scale(model_matrix, glm::vec3(5.0f, 5.0f, 5.0f));
		setUseTexture(true);
		//sphere.draw(this->simpleShader, model_matrix);
	}
	this->texture->unbind(0);


	if (true)
	{
		glm::mat4 model_matrix = glm::mat4();
		model_matrix = glm::scale(model_matrix, glm::vec3(200.0f, 100.0f, 1.0f));
		model_matrix = glm::translate(model_matrix, glm::vec3(0, 0, -100));

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(1, 0, 0));;
		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));;
		glUniformMatrix4fv(glGetUniformLocation(this->simpleShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		this->tile->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		setUseTexture(true);

		this->plane.draw(this->simpleShader, model_matrix);

		model_matrix = glm::mat4();
		model_matrix = glm::scale(model_matrix, glm::vec3(200.0f, 100.0f, 1.0f));
		model_matrix = glm::translate(model_matrix, glm::vec3(0, 0, 100));

		model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(0, 1, 0));;
		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(1, 0, 0));;
		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));;
		glUniformMatrix4fv(glGetUniformLocation(this->simpleShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		this->plane.draw(this->simpleShader, model_matrix);

		model_matrix = glm::mat4();
		model_matrix = glm::scale(model_matrix, glm::vec3(1.0, 100.0f, 200.0f));
		model_matrix = glm::translate(model_matrix, glm::vec3(-100, 0, 0));

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));;
		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(1, 0, 0));;
		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));;
		glUniformMatrix4fv(glGetUniformLocation(this->simpleShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		this->plane.draw(this->simpleShader, model_matrix);

		model_matrix = glm::mat4();
		model_matrix = glm::scale(model_matrix, glm::vec3(200.0f, 1.0f, 200.0f));
		model_matrix = glm::translate(model_matrix, glm::vec3(0, -50, 0));

		glUniformMatrix4fv(glGetUniformLocation(this->simpleShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);

		this->plane.draw(this->simpleShader, model_matrix);
		this->tile->unbind(0);
	}
}

void TrainView::drawSurface()
{
	this->surfaceShader->Use();
#pragma region surfaceLighting

	//Lighting------------------------------------------------


	if (this->tw->shadingBrowser->selected(2))
	{
		this->surfaceShader->setInt("u_shadingSelect", 1); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(3))
	{
		this->surfaceShader->setInt("u_shadingSelect", 2); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(4))
	{
		this->surfaceShader->setInt("u_shadingSelect", 3); //0-No 1-Phong 2-Garudond 3-Toon
	}
	else
	{
		this->surfaceShader->setInt("u_shadingSelect", 0); //0-No 1-Phong 2-Garudond	
	}
	this->surfaceShader->setVec3("u_viewer_pos", this->arcball.getEyePos());

	this->surfaceShader->setVec3("dirLights[0].direction", 0.0f, -1.0f, -1.0f);
	this->surfaceShader->setVec3("dirLights[0].ambient", 0.2f, 0.2f, 0.2f);
	this->surfaceShader->setVec3("dirLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	this->surfaceShader->setVec3("dirLights[0].specular", 1.0f, 1.0f, 1.0f);

	this->surfaceShader->setVec3("pointLights[0].position", this->lightBoxPos);
	this->surfaceShader->setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
	this->surfaceShader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	this->surfaceShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	this->surfaceShader->setFloat("pointLights[0].constant", 1.0f);
	this->surfaceShader->setFloat("pointLights[0].linear", 0.01f);
	this->surfaceShader->setFloat("pointLights[0].quadratic", 0.0001f);

	//this->surfaceShader->setVec3("spotLights[0].position", this->arcball.getEyePos());
	//this->surfaceShader->setVec3("spotLights[0].ambient", 0.2f, 0.2f, 0.2f);
	//this->surfaceShader->setVec3("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	//this->surfaceShader->setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
	//this->surfaceShader->setFloat("spotLights[0].constant", 1.0f);
	//this->surfaceShader->setFloat("spotLights[0].linear", 0);
	//this->surfaceShader->setFloat("spotLights[0].quadratic", 0);
	//this->surfaceShader->setVec3("spotLights[0].direction", -this->arcball.getEyePos());
	//this->surfaceShader->setFloat("spotLights[0].cutoff", glm::cos(glm::radians(10.0f)));
	//this->surfaceShader->setFloat("spotLights[0].outer_cutoff", glm::cos(glm::radians(15.0f)));



	//Lighting------------------------------------------------
#pragma endregion

#pragma region surfaceDraw

	//wave


	this->surfaceShader->setVec2("u_direction", glm::vec2(1, -1));
	this->surfaceShader->setFloat("u_time", this->m_pTrack->trainU);
	this->surfaceShader->setFloat("u_wavelength", this->tw->waveLength->value());
	this->surfaceShader->setFloat("u_amplitude", this->tw->amplitude->value());
	this->surfaceShader->setBool("u_testNormal", this->tw->testButton->value());

	if (this->tw->waveBrowser->selected(2))
	{
		this->surfaceShader->setInt("u_waveSelect", 1);
	}
	else
	{
		this->surfaceShader->setInt("u_waveSelect", 0);
	}
	if (imgCounter >= imgInterval)
	{
		imgCounter = 0;
	}
	else
	{
		imgCounter++;
	}
	this->heightmap[imgIdx]->bind(2);
	glUniform1i(glGetUniformLocation(this->surfaceShader->Program, "u_heightmap"), 2);

	this->background->bind(10);
	glUniform1i(glGetUniformLocation(this->surfaceShader->Program, "u_skybox"), 10);


	glUniform1i(glGetUniformLocation(this->surfaceShader->Program, "u_renderbox"), 11);

	//wave
	if (true) {
		glm::mat4 model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(1.0f, 10.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(this->surfaceShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
		this->texture->bind(0);

		this->surfaceShader->setBool("u_useTexture", false);
		this->waterSurface.draw(this->surfaceShader, model_matrix);
		this->texture->unbind(0);
	}
	this->heightmap[imgIdx]->unbind(2);
	this->background->unbind(10);
#pragma endregion
}

void TrainView::drawBackground(glm::mat4 view_matrix, glm::mat4 projection_matrix)
{
	//Background
	this->backgroundShader->Use();
	glDepthMask(false);
	if (projection_matrix == glm::mat4())
	{
		glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	}
	glUniformMatrix4fv(glGetUniformLocation(this->backgroundShader->Program, "u_projection"), 1, GL_FALSE, &projection_matrix[0][0]);

	if (view_matrix == glm::mat4())
	{
		glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	}
	view_matrix = glm::mat4(glm::mat3(view_matrix));
	glUniformMatrix4fv(glGetUniformLocation(this->backgroundShader->Program, "u_view"), 1, GL_FALSE, &view_matrix[0][0]);

	glm::mat4 model_matrix = glm::mat4();

	this->background->bind(0);
	glUniform1i(glGetUniformLocation(this->backgroundShader->Program, "u_skybox"), 0);

	this->bgPlane.draw(this->backgroundShader, model_matrix);

	model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
	this->bgPlane.draw(this->backgroundShader, model_matrix);

	model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
	this->bgPlane.draw(this->backgroundShader, model_matrix);

	model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
	this->bgPlane.draw(this->backgroundShader, model_matrix);

	model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
	model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
	model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
	this->bgPlane.draw(this->backgroundShader, model_matrix);

	model_matrix = glm::mat4(1);
	model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
	this->bgPlane.draw(this->backgroundShader, model_matrix);

	this->background->unbind(0);
	glDepthMask(true);

}
//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();

	// where is the mouse?
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n", selectedCube);
}

void TrainView::setViewAndProjToUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TrainView::setViewAndProjToUBO(glm::mat4 view_matrix, glm::mat4 projection_matrix)
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();


	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TrainView::setUseTexture(bool set)
{
	GLboolean to = set;
	/*int u_useTextureLocation = glGetUniformLocation(this->shader->Program, "u_useTexture");
	glUniform1i(u_useTextureLocation, to);*/
	this->simpleShader->setBool("u_useTexture", set);
}

