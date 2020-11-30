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
	arcball.setup(this, 40, 250, .2f, .4f, 0);
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
			this->commom_matrices = new UBO();
		this->commom_matrices->size = 2 * sizeof(glm::mat4);

		glGenBuffers(1, &this->commom_matrices->ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
		glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);



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

		if (!this->bg_back)
			this->bg_back = new Texture2D("../../Images/skybox/back.jpg");
		if (!this->bg_front)
			this->bg_front = new Texture2D("../../Images/skybox/front.jpg");
		if (!this->bg_top)
			this->bg_top = new Texture2D("../../Images/skybox/top.jpg");
		if (!this->bg_bottom)
			this->bg_bottom = new Texture2D("../../Images/skybox/bottom.jpg");
		if (!this->bg_right)
			this->bg_right = new Texture2D("../../Images/skybox/right.jpg");
		if (!this->bg_left)
			this->bg_left = new Texture2D("../../Images/skybox/left.jpg");

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




	//bind shader
	this->simpleShader->Use();

	//Background
	if (true)
	{
		this->simpleShader->setInt("u_shadingSelect", 0); //0-No 1-Phong 2-Garudond	
		glm::mat4 model_matrix = glm::mat4();
		setUseTexture(true);

		this->bg_front->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_front->unbind(0);

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
		this->bg_right->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_right->unbind(0);

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
		this->bg_back->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_back->unbind(0);

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
		this->bg_left->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_left->unbind(0);

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0, 1, 0));

		model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
		model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
		this->bg_top->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_top->unbind(0);

		model_matrix = glm::mat4(1);
		model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(0, 0, 1));
		this->bg_bottom->bind(0);
		glUniform1i(glGetUniformLocation(this->simpleShader->Program, "u_texture"), 0);
		this->bgPlane.draw(this->simpleShader, model_matrix);
		this->bg_bottom->unbind(0);
	}

	//Lighting------------------------------------------------


	if (this->tw->shadingBrowser->selected(2))
	{
		this->simpleShader->setInt("u_shadingSelect", 1); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(3))
	{
		this->simpleShader->setInt("u_shadingSelect", 2); //0-No 1-Phong 2-Garudond
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
	//sphere.draw(this->simpleShader, model_matrix);

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


	//####################################################################################################
	//Draw Surface unsing indepent shader
	this->surfaceShader->Use();

	//Lighting------------------------------------------------


	if (this->tw->shadingBrowser->selected(2))
	{
		this->surfaceShader->setInt("u_shadingSelect", 1); //0-No 1-Phong 2-Garudond
	}
	else if (this->tw->shadingBrowser->selected(3))
	{
		this->surfaceShader->setInt("u_shadingSelect", 2); //0-No 1-Phong 2-Garudond
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

	//wave
	this->surfaceShader->setVec2("u_direction", glm::vec2(1, -1));
	this->surfaceShader->setFloat("u_time", this->m_pTrack->trainU);
	this->surfaceShader->setFloat("u_wavelength", this->tw->waveLength->value());
	this->surfaceShader->setFloat("u_amplitude", this->tw->amplitude->value());
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
	
	//wave
	{
		glm::mat4 model_matrix = glm::mat4();
		model_matrix = glm::translate(model_matrix, this->source_pos);
		model_matrix = glm::scale(model_matrix, glm::vec3(1.0f, 10.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(this->surfaceShader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);


		this->surfaceShader->setBool("u_useTexture", false);
		this->waterSurface.draw(this->surfaceShader, model_matrix);
	}
	this->heightmap[imgIdx]->unbind(2);
	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
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

void TrainView::setUseTexture(bool set)
{
	GLboolean to = set;
	/*int u_useTextureLocation = glGetUniformLocation(this->shader->Program, "u_useTexture");
	glUniform1i(u_useTextureLocation, to);*/
	this->simpleShader->setBool("u_useTexture", set);
}

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, element.size() * sizeof(GLint), element.data(), GL_STATIC_DRAW);

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
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex4f(-1000, 1000, -1000, 1);

	glTexCoord2f(1.0f, 1.0f);
	glVertex4f(1000, 1000, -1000, 1);

	glTexCoord2f(1.0f, 0.0f);
	glVertex4f(1000, -1000, -1000, 1);

	glTexCoord2f(0.0f, 0.0f);
	glVertex4f(-1000, -1000, -1000, 1);

	glEnd();
	GLfloat  vertices[] = {
		-1000.0f ,1000.0f , -1000.0f,
		 1000.0f ,1000.0f , -1000.0f ,
		 1000.0f ,-1000.0f , -1000.0f,
		-1000.0f ,-1000.0f , -1000.0f };
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
