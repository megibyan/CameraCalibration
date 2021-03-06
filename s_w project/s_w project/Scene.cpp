/**
*	@file Scene.cpp
*	@brief file contains all necessary methods to handle scene.
*	
*	@author Razmik Avetisyan
*/

#include "stdafx.h"
#include "Scene.h"

Scene* Scene::instance  = NULL;

/** 
 * This method is used to get the instance of scene (singleton util)
 * @author Razmik Avetisyan
 * @return Pointer to instance of the scene
 */
Scene * Scene::getInstance(){
	if (instance == NULL){
		instance = new Scene();
	}

	return instance;
}

/** 
 * Default constructor 
 * @author Razmik Avetisyan
 */
Scene::Scene(){
	show_frustums = true;
	show_local_coord_axes = true;
	mouse.left_button_pressed = false;
	mouse.middle_button_pressed = false;
	mouse.prev_x_coord = -1;
	mouse.prev_y_coord = -1;
}


/** 
 * This method is used to load calibration file into memory
 * @author Razmik Avetisyan
 * @param An absolute path of the file of calibration
 */
void Scene::loadFileIntoMemory(char *file_path)
{	
	ifstream indata;
	indata.open(file_path);

	if(!indata) {
      cerr << "Error: file could not be opened" << endl;
      exit(1);
    }

	string line;
	while (getline(indata, line)){
		file_data.push_back(line);
	}

	loadCalibrationData();

}

/** 
 * Method-wrapper for loading file into memory
 * @author Razmik Avetisyan
 */
void Scene::loadCalibrationData()
{
	loadHeaders();
	loadCameraData();
}

/** 
 * Method-helper for loading headers of calibration file into the memory
 * @author Razmik Avetisyan
 */
void Scene::loadHeaders()
{
	string matrix_name = "";
	vector<string>::iterator ii;
	for(ii = file_data.begin(); ii < file_data.end(); ii++)
	{
		string line = *ii;
		if(line.find("]") != string::npos && matrix_name == "nccl_cam_calib_header"){		
			break;
		}
		int pos = line.find("=[");
		if(string::npos != pos)
			matrix_name = line.substr(0,pos);

		if(matrix_name != "nccl_cam_calib_header" || (string::npos != pos) ) continue;
		
		line = line.substr(line.find("\"") + 1, line.length());
		string camera_id = line.substr(0, line.find("\""));
		line = line.substr(line.find("\"") + 1, line.length());

		int cam_id = atoi(camera_id.c_str());
 		Camera *camera = new Camera(cam_id);

		line = line.substr(line.find("\"") + 1, line.length());
		string _distortion = line.substr(0, line.find("\""));
		line = line.substr(line.find("\"") + 1, line.length());
		camera->_setCameraDistortion(_distortion);

		line = line.substr(line.find("\"") + 1, line.length());
		string _cam_K = line.substr(0, line.find("\""));
		line = line.substr(line.find("\"") + 1, line.length());
		camera->_setCameraK(_cam_K);

		line = line.substr(line.find("\"") + 1, line.length());
		string _cam_R = line.substr(0, line.find("\""));
		line = line.substr(line.find("\"") + 1, line.length());
		camera->_setCameraR(_cam_R);

		line = line.substr(line.find("\"") + 1, line.length());
		string _cam_t = line.substr(0, line.find("\""));
		line = line.substr(line.find("\"") + 1, line.length());
		camera->_setCameraT(_cam_t);
		
		cameras[cam_id] = static_cameras[cam_id] = camera;
		
		//static_cameras.push_back(camera);
	}
}

/** 
 * Method-helper scans the string vector and exracts camera data
 * @author Razmik Avetisyan
 */
void Scene::loadCameraData()
{
	if(getCameraCount() < 1){
		cerr << "Error: file could not be opened" << endl;
		exit(1);	
	}

	string r,k,t,dis;	
	string matrix_name = "";
	for(map<int,Camera *>::iterator ii=cameras.begin(); ii!=cameras.end(); ++ii)
	{
		r = (*ii).second->_getCameraR();
		k = (*ii).second->_getCameraK();
		t = (*ii).second->_getCameraT();
		dis = (*ii).second->_getCameraDistortion();

		vector<string>::iterator jj;
		string line;
		GLdouble r_c[3][3];
		GLdouble k_c[3][3];
		GLdouble dis_c[5];
		GLdouble t_c[3];
		
		int r_row = 0;
		int k_row = 0;
		for(jj = file_data.begin(); jj < file_data.end(); jj++)
		{
			line = *jj;
			int pos = line.find("=[");
			if(string::npos != pos)
				matrix_name = line.substr(0,pos);
			
			if((line.find("=[") == string::npos) && (line.find("]") == string::npos)){
				float p1,p2,p3,p4,p5;
				//************** case when is R matrix
				if(matrix_name == r)
				{					
					sscanf_s(line.c_str(),"%e %e %e", &p1, &p2, &p3);	
					r_c[r_row][0] = p1;
					r_c[r_row][1] = p2;
					r_c[r_row][2] = p3;
					r_row++;
				}	
				//************** case when is K matrix
				if(matrix_name == k)
				{
					sscanf_s(line.c_str(),"%e %e %e", &p1, &p2, &p3);	
					k_c[k_row][0] = p1;
					k_c[k_row][1] = p2;
					k_c[k_row][2] = p3;
					k_row++;
				}
				//************** case when is T matrix
				if(matrix_name == t)
				{
					sscanf_s(line.c_str(),"%e %e %e", &p1, &p2, &p3);	
					t_c[0] = p1;
					t_c[1] = p2;
					t_c[2] = p3;
				}
				//************** case when is distortion matrix
				if(matrix_name == dis)
				{
					sscanf_s(line.c_str(),"%e %e %e %e %e", &p1, &p2, &p3, &p4, &p5);	
					dis_c[0] = p1;
					dis_c[1] = p2;
					dis_c[2] = p3;
					dis_c[3] = p4;
					dis_c[4] = p5;
				}
			}
		}	

		(*ii).second->c_R.set(r_c);
		(*ii).second->c_K.set(k_c);
		(*ii).second->c_d.set(dis_c);
		(*ii).second->c_t.set(t_c);
		
	}
}

/** 
 * This method is used to return number of cameras
 * @author Razmik Avetisyan
 * @return Number of cameras
 */
int Scene::getCameraCount()
{
	return cameras.size();
}

/** 
 * This method is used to get the map of cameras
 * @author Razmik Avetisyan 
 * @param boolean staticCameras	 Pass true to get static cameras, othewise will return dynamic cameras
 * @return Map of cameras (id , camera *)
 */
map<int, Camera *> Scene::getCameras(bool staticCameras)
{
	if(staticCameras == true) 
		return static_cameras;

	return cameras;
}

/** 
 * This method is used to get camera by id
 * @author Razmik Avetisyan
 * @return Pointer to camera
 */
Camera* Scene::getCameraByID(int camera_id)
{
	return cameras[camera_id];
}

/** 
 * This method is used to print all cameras within the scene 
 * @author Razmik Avetisyan
 */
void Scene::printCameraData()
{
	for(map<int,Camera *>::iterator ii=cameras.begin(); ii!=cameras.end(); ++ii)
		(*ii).second->print();
}


/** 
 * This method is used to toggle state(show/hide) of frustums 
 * @author Razmik Avetisyan
 */
void Scene::toggleShowFrustums()
{
	show_frustums = !show_frustums;
}

/** 
 * This method is used to toggle state(show/hide) of local coordinate axis 
 * @author Razmik Avetisyan
 */
void Scene::toggleShowLocalCoordAxes()
{
	show_local_coord_axes = !show_local_coord_axes;
}

/** 
 * This method is used to get the state of show frustums var
 * @author Razmik Avetisyan
 * @return Returns the state of frustums
 */
bool Scene::getShowFrustumsState()
{
	return show_frustums;
}

/** 
 * This method is used to get the state of show local coordinate axes var
 * @author Razmik Avetisyan
 * @return Returns the state of local coordinate axes
 */
bool Scene::getShowLocalCoordAxes()
{
	return show_local_coord_axes;
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Keyboard event handler
 * @param key The ASCII code of pressed key
 * @param x,y location of mouse when 'key' was pressed 
 */
void Scene::keyHandler(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27: {
			exit(0); break;
		}
		case 'f': {
			toggleShowFrustums();			
			break;
		}
		case 'a': {
			toggleShowLocalCoordAxes();
			break;
		}
		case 'b': {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateZ(-0.3);
			} else {
				grid.updateRotationZ(-5); // rotation around z counterclockwise
			}		
			break;
		}
		case 'u': {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateZ(0.3);
			} else {
				grid.updateRotationZ(5); // rotation around z clockwise
			}	
			break;
		}
		case 'r': {
			grid.setTransformationMode(ROTATION);
			break;
		}
		case 't': {
			grid.setTransformationMode(TRANSLATION);
			break;
		}
	}
	glutPostRedisplay();
}
/**
 * @author: Geghetsik Dabaghyan
 * @brief Keyboard special key event handler
 * @param key The code of pressed specials key
 * @param x,y location of mouse when 'key' was pressed 
 */
void Scene::keySpecialHandler(int key, int x, int y)
{
	switch (key)
	{
		case GLUT_KEY_RIGHT: {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateX(0.3);
			} else {
				grid.updateRotationY(5); // rotation around y clockwise
			}
			break;
		}
		case GLUT_KEY_LEFT: {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateX(-0.3);
			} else {
				grid.updateRotationY(-5); // rotation around y counterclockwise
			}
			break;
		}
		case GLUT_KEY_UP: {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateY(0.3);
			} else {
				grid.updateRotationX(5); // rotation around x clockwise
			}
			break;
		}
		case GLUT_KEY_DOWN: {
			if (TRANSLATION == grid.getTransformationMode()) {
				grid.updateTranslateY(-0.3);
			} else {
				grid.updateRotationX(-5); // rotation around x counterclockwise
			}
			break;
		}
	}
	glutPostRedisplay();
}
/**
 * @author: Geghetsik Dabaghyan
 * @brief Mouse button handler
 * @param button The mouse button to be handled
 * @param state The state of the mouse button 
 * @param x,y location of mouse
 */
void Scene::mouseButtonHandler(int button, int state, int x, int y)
{
	switch (button)
	{
		case GLUT_LEFT_BUTTON: {
			if (GLUT_DOWN == state) { // the button is pressed
				mouse.left_button_pressed = true;
				mouse.prev_x_coord = x;
				mouse.prev_y_coord = y;
			} else {
				mouse.left_button_pressed = false;
				mouse.prev_x_coord = -1;
				mouse.prev_y_coord = -1;
			}
			break;
		}
		case GLUT_MIDDLE_BUTTON: {
			if (GLUT_DOWN == state) { // the button is pressed
				mouse.middle_button_pressed = true;
				mouse.prev_y_coord = y;
			} else {
				mouse.middle_button_pressed = false;
				mouse.prev_y_coord = -1;
			}
			break;
		}
	}
	glutPostRedisplay();
}
/**
 * @author: Geghetsik Dabaghyan
 * @brief Mouse move handler
 * @param x,y location of mouse
 */
void Scene::mouseMoveHandler(int x, int y)
{
	if (mouse.left_button_pressed) {
		if (TRANSLATION == grid.getTransformationMode()) {
			double dX = (x - mouse.prev_x_coord) * 0.001;
			grid.updateTranslateX(dX);
			double dY = -(y - mouse.prev_y_coord) * 0.001;
			grid.updateTranslateY(dY);
		}
		if (ROTATION == grid.getTransformationMode()) {
			double d_angle = (x - mouse.prev_x_coord)*0.01;
			grid.updateRotationY(d_angle);
			mouse.prev_x_coord = x;

			d_angle = (y - mouse.prev_y_coord)*0.01;
			grid.updateRotationX(d_angle);	
			mouse.prev_y_coord = y;
		}
	}
	if (mouse.middle_button_pressed) {
		if (TRANSLATION == grid.getTransformationMode()) {
			double dZ = (y - mouse.prev_y_coord) * 0.001;
			grid.updateTranslateZ(dZ);
		}
		if (ROTATION == grid.getTransformationMode()) {
			double d_angle = (y - mouse.prev_y_coord)*0.01;
			grid.updateRotationZ(d_angle);
			mouse.prev_y_coord = y;
		}
	}
	glutPostRedisplay();
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to set geometry to be drawn
 */
void Scene::setGrid()
{
	map<int, Camera *>::iterator it;
	for (it = cameras.begin(); it != cameras.end(); it++) {
		grid.addCamera((*it).second);
	}
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to get geometry to be drawn
 * @return Returns pointer to Grid
 */
Grid* Scene::getGrid()
{
	return &grid;
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to handle the main menu events
 */
void Scene::handleMainMenu(int evnt)
{

}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to handle the transformation mode menu
 */
void Scene::handleTransformationModeMenu(int evnt)
{
	grid.setTransformationMode((Transformation) evnt);
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to handle the transformation mode menu
 */
void Scene::handleFrustumMenu(int evnt)
{
	show_frustums = (State) evnt;
	glutPostRedisplay();
}

/**
 * @author: Geghetsik Dabaghyan
 * @brief Method to handle the transformation mode menu
 */
void Scene::handleAxisMenu(int evnt)
{
	show_local_coord_axes = (State) evnt;
	grid.axes = (State) evnt;
	glutPostRedisplay();
}

/** 
 * This method is used to set the look at matrix
 * @author Razmik Avetisyan
 * @param The look at matrix values 3x3
 */
void Scene::lookAtMatrix(GLdouble data[3][3])
{
	look_at.set(data);
	_glLookAt();
}

/** 
 * This method is used to set the look at matrix
 * @author Razmik Avetisyan
 * @param Look at object
 */
void Scene::lookAtMatrix(LookAt data)
{
	look_at = data;
	_glLookAt();
}

/** 
 * This method is used to get the look at matrix
 * @author Razmik Avetisyan
 * @return Returns look at matrix
 */
LookAt & Scene::lookAtMatrix()
{
	return look_at;
}

/** 
 * This method triggers doCommand of look at object
 * @author Razmik Avetisyan
 */
void Scene::_glLookAt()
{
	look_at.doCommand();
}