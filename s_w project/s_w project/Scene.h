/**
*	@file Scene.h
*	@brief file contains the definaion of the class Scene.
*	
*	@author Razmik Avetisyan
*/

#pragma once
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <GL/glut.h>
#include "string"
#include "Camera.h"
#include "Grid.h"
#include "LookAt.h"

using namespace std;

struct Mouse {
	bool left_button_pressed;
	bool middle_button_pressed;
	double prev_x_coord;
	double prev_y_coord;
};

struct Menues {
	int mainMenuID;
	int transformationModeMenuID;
	int frustumMenuID;
	int axisMenuID;
};

enum State {OFF, ON};

class Scene{
public:
	void loadFileIntoMemory(char *);
	void loadCalibrationData();
	void loadHeaders();
	void loadCameraData();
	int getCameraCount();
	map<int, Camera *> getCameras(bool staticCameras=false);
	Camera * getCameraByID(int);
	void printCameraData();
	static Scene * getInstance();

	void toggleShowFrustums();
	void toggleShowLocalCoordAxes();

	bool getShowFrustumsState();
	bool getShowLocalCoordAxes();

	void keyHandler(unsigned char key, int x, int y);
	void keySpecialHandler(int key, int x, int y);
	void mouseButtonHandler(int button, int state, int x, int y);
	void mouseMoveHandler(int x, int y);

	void setGrid();
	Grid* getGrid();
	
	struct Menues menues;
	void handleMainMenu(int);
	void handleTransformationModeMenu(int);
	void handleFrustumMenu(int);
	void handleAxisMenu(int);
	
	void lookAtMatrix(GLdouble data[3][3]);
	void lookAtMatrix(LookAt data);
	LookAt & lookAtMatrix();
	void _glLookAt();
private:
	Scene();
	static Scene * instance;
	map<int, Camera *> static_cameras;
	map<int, Camera *> cameras;
	vector<string> file_data;
	bool show_frustums;
	bool show_local_coord_axes;
	struct Mouse mouse;
	Grid grid;
	LookAt look_at;
};

