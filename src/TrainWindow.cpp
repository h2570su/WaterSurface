/************************************************************************
	 File:        TrainWindow.H

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						this class defines the window in which the project
						runs - its the outer windows that contain all of
						the widgets, including the "TrainView" which has the
						actual OpenGL window in which the train is drawn

						You might want to modify this class to add new widgets
						for controlling	your train

						This takes care of lots of things - including installing
						itself into the FlTk "idle" loop so that we get periodic
						updates (if we're running the train).


	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <FL/fl.h>
#include <FL/Fl_Box.h>

// for using the real time clock
#include <time.h>

#include "TrainWindow.H"
#include "TrainView.H"
#include "CallBacks.H"



//************************************************************************
//
// * Constructor
//========================================================================
TrainWindow::
TrainWindow(const int x, const int y)
	: Fl_Double_Window(x, y, 800, 600, "Train and Roller Coaster")
	//========================================================================
{
	// make all of the widgets
	begin();	// add to this widget
	{
		int pty = 5;			// where the last widgets were drawn

		trainView = new TrainView(5, 5, 590, 590);
		trainView->tw = this;
		trainView->m_pTrack = &m_Track;
		this->resizable(trainView);

		// to make resizing work better, put all the widgets in a group
		widgets = new Fl_Group(600, 5, 190, 590);
		widgets->begin();

		runButton = new Fl_Button(605, pty, 60, 20, "Run");
		togglify(runButton);

		Fl_Button* fb = new Fl_Button(700, pty, 25, 20, "@>>");
		fb->callback((Fl_Callback*)forwCB, this);
		Fl_Button* rb = new Fl_Button(670, pty, 25, 20, "@<<");
		rb->callback((Fl_Callback*)backCB, this);

		pty += 25;
		speed = new Fl_Value_Slider(655, pty, 140, 20, "speed");
		speed->range(0, 5);
		speed->value(1);
		speed->align(FL_ALIGN_LEFT);
		speed->type(FL_HORIZONTAL);

		pty += 30;
		amplitude = new Fl_Value_Slider(655, pty, 140, 20, "Amp");
		amplitude->range(0, 30);
		amplitude->value(5);
		amplitude->align(FL_ALIGN_LEFT);
		amplitude->type(FL_HORIZONTAL);
		amplitude->callback((Fl_Callback*)damageCB, this);

		pty += 30;
		waveLength = new Fl_Value_Slider(655, pty, 140, 20, "waveLen");
		waveLength->range(0.0001, 1);
		waveLength->value(0.2);
		waveLength->align(FL_ALIGN_LEFT);
		waveLength->type(FL_HORIZONTAL);
		waveLength->callback((Fl_Callback*)damageCB, this);

		pty += 30;

		// camera buttons - in a radio button group
		Fl_Group* camGroup = new Fl_Group(600, pty, 195, 20);
		camGroup->begin();
		worldCam = new Fl_Button(605, pty, 60, 20, "World");
		worldCam->type(FL_RADIO_BUTTON);		// radio button
		worldCam->value(1);			// turned on
		worldCam->selection_color((Fl_Color)3); // yellow when pressed
		worldCam->callback((Fl_Callback*)damageCB, this);
		trainCam = new Fl_Button(670, pty, 60, 20, "Train");
		trainCam->type(FL_RADIO_BUTTON);
		trainCam->value(0);
		trainCam->selection_color((Fl_Color)3);
		trainCam->callback((Fl_Callback*)damageCB, this);
		topCam = new Fl_Button(735, pty, 60, 20, "Top");
		topCam->type(FL_RADIO_BUTTON);
		topCam->value(0);
		topCam->selection_color((Fl_Color)3);
		topCam->callback((Fl_Callback*)damageCB, this);
		camGroup->end();

		pty += 30;

		// browser to select spline types
		// TODO: make sure these choices are the same as what the code supports
		shadingBrowser = new Fl_Browser(605, pty, 90, 75, "Shading Type");
		shadingBrowser->type(2);		// select
		shadingBrowser->callback((Fl_Callback*)damageCB, this);
		shadingBrowser->add("None");
		shadingBrowser->add("Phong");
		shadingBrowser->add("Smooth");
		shadingBrowser->add("Toon");
		shadingBrowser->select(2);

		waveBrowser = new Fl_Browser(700, pty, 90, 75, "Wave Type");
		waveBrowser->type(2);		// select
		waveBrowser->callback((Fl_Callback*)damageCB, this);
		waveBrowser->add("Sine");
		waveBrowser->add("Height Map");
		waveBrowser->select(1);

		pty += 110;

		// add and delete points
		Fl_Button* ap = new Fl_Button(605, pty, 80, 20, "Add Point");
		ap->callback((Fl_Callback*)addPointCB, this);
		Fl_Button* dp = new Fl_Button(690, pty, 80, 20, "Delete Point");
		dp->callback((Fl_Callback*)deletePointCB, this);

		pty += 25;
		// reset the points
		resetButton = new Fl_Button(735, pty, 60, 20, "Reset");
		resetButton->callback((Fl_Callback*)resetCB, this);
		Fl_Button* loadb = new Fl_Button(605, pty, 60, 20, "Load");
		loadb->callback((Fl_Callback*)loadCB, this);
		Fl_Button* saveb = new Fl_Button(670, pty, 60, 20, "Save");
		saveb->callback((Fl_Callback*)saveCB, this);

		pty += 25;
		// roll the points
		Fl_Button* rx = new Fl_Button(605, pty, 30, 20, "R+X");
		rx->callback((Fl_Callback*)rpxCB, this);
		Fl_Button* rxp = new Fl_Button(635, pty, 30, 20, "R-X");
		rxp->callback((Fl_Callback*)rmxCB, this);
		Fl_Button* rz = new Fl_Button(670, pty, 30, 20, "R+Z");
		rz->callback((Fl_Callback*)rpzCB, this);
		Fl_Button* rzp = new Fl_Button(700, pty, 30, 20, "R-Z");
		rzp->callback((Fl_Callback*)rmzCB, this);

		pty += 30;
		testButton = new Fl_Button(605, pty, 30, 20, "Test");
		togglify(testButton);
		testButton->callback((Fl_Callback*)damageCB, this);;
		// TODO: add widgets for all of your fancier features here
#ifdef EXAMPLE_SOLUTION
		makeExampleWidgets(this, pty);
#endif

		// we need to make a little phantom widget to have things resize correctly
		Fl_Box* resizebox = new Fl_Box(600, 595, 200, 5);
		widgets->resizable(resizebox);

		widgets->end();
	}
	end();	// done adding to this widget

	// set up callback on idle
	Fl::add_idle((void(*)(void*))runButtonCB, this);
}

//************************************************************************
//
// * handy utility to make a button into a toggle
//========================================================================
void TrainWindow::
togglify(Fl_Button* b, int val)
//========================================================================
{
	b->type(FL_TOGGLE_BUTTON);		// toggle
	b->value(val);		// turned off
	b->selection_color((Fl_Color)3); // yellow when pressed	
	b->callback((Fl_Callback*)damageCB, this);
}

//************************************************************************
//
// *
//========================================================================
void TrainWindow::
damageMe()
//========================================================================
{
	if (trainView->selectedCube >= ((int)m_Track.points.size()))
		trainView->selectedCube = 0;
	trainView->damage(1);
}

//************************************************************************
//
// * This will get called (approximately) 30 times per second
//   if the run button is pressed
//========================================================================
void TrainWindow::
advanceTrain(float dir)
//========================================================================
{
	this->trainView->lightBoxPos;
	glm::mat4 rot = glm::rotate(1.0f*((float)this->speed->value()*0.05f)*dir, glm::vec3(1.0f, 0.0f, 0.0f));
	this->trainView->lightBoxPos = rot * glm::vec4(this->trainView->lightBoxPos, 1.0f);
	this->m_Track.trainU += dir * (float)this->speed->value()*0.02f;

	this->trainView->imgIdx++;
	if (this->trainView->imgIdx >= this->trainView->heightmap.size())
	{
		this->trainView->imgIdx = 0;
	}
	//std::cout << "idx:" << this->trainView->imgIdx << std::endl;
}