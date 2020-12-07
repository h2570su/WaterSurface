/************************************************************************
     File:        TrainView.H

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

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"
#include "RenderUtilities/TextureCube.h"
#include "Sphere.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"
#include <vector>
#include "Object.H"


class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		void simpleShaderDraw();

		void drawSurface();

		void drawBackground(glm::mat4 view_matrix = glm::mat4(), glm::mat4 projection_matrix = glm::mat4());

		

		
		

		

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//set ubo
		void setViewAndProjToUBO();

		void setViewAndProjToUBO(glm::mat4 view_matrix, glm::mat4 projection_matrix);

		void setUseTexture(bool set);
	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube;  // simple - just remember which cube is selected

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene

		Shader* simpleShader		= nullptr;
		Shader* backgroundShader		= nullptr;
		Shader* surfaceShader		= nullptr;
		Texture2D* texture	= nullptr;
		std::vector<Texture2D*> heightmap;
		const int imgInterval = 1;
		int imgCounter = 0;
		int imgIdx = 0;
		Texture2D* tile	= nullptr;

		TextureCube* background	= nullptr;
		GLuint fbo[6];
		GLuint rbo[6];
		GLuint reflectTexture;
		
		//VAO* plane			= nullptr;
		UBO* commom_matrices= nullptr;


		//OpenAL
		glm::vec3 source_pos;
		glm::vec3 listener_pos;
		ALCdevice* device	= nullptr;
		ALCcontext* context = nullptr;
		ALuint source;
		ALuint buffer;

		aBox lightBox;
		glm::vec3 lightBoxPos = glm::vec3(0, 100, 0);

		static const int boxesAmount = 200;
		aBox box;
		glm::vec3 boxesPos[boxesAmount];
		mySphere sphere;

		aPlane plane;
		aSurface waterSurface = aSurface(1600);
		aBgPlane bgPlane;
};

