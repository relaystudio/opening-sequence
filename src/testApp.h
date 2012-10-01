#pragma once

#define USE_TWO_KINECTS 1

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxAutoControlPanel.h"
#include "ofxKinect.h"
#include "user.h"
#include "scene.h"

class testApp: public ofBaseApp {
    
   typedef struct {
        int min;
        int max;
    } Range;
    
public:
        //   virtual void setup();
        //    testApp();
        //    ~testApp();
    void setup();
    
    void update();
    void draw();
    void exit();
    
    vector<Scene*> scene; // Holds scene objects, pass pointers
    vector<Layer*> layer; // Holds ALL user obj, pass pointers

    void setupType();
    void setupCamera();
    void reset();
    void updateCamera();
    void configure();
    void updateConditional();
    void setDebug(bool _debug);
    
    void saveHomography();
    bool loadHomography( string * path );
    void updateHomography();

    // Setup declarations
    void setupPanel();
    void updatePanel();
    ofxAutoControlPanel panel;
    ofTrueTypeFont raleway;
    
    // Globals
    int panelWidth;
    bool debug;
    float fliph;
    float flipv;
    int numThresh;
    
    float threshold;
    float scaleFactor;
    
    //Camera
    ofxKinect kinect;
    ofxKinect * cam;
    ofImage depthImg;
    float angle; // kinect angle across both
#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
#endif
    ofTexture stitchKinect(ofxKinect * _k1, ofxKinect * _k2);
    ofTexture stitched;
    cv::Mat stitchedKinect;
    ofxCv::ContourFinder contourFinder;
    vector<Range> ranges;
    
    ofxCv::RunningBackground background;
    cv::Mat prevCVFrame;
    cv::Mat curCVFrame;
    
    // Blobs
    void getScene(cv::Mat * _frame, vector<Range> * _thresh);
    
    
    //Events

    void keyPressed(int key);
    
private:
    
    // Homography state
    bool loadMap(string * path);
    void parseMap(ofImage * map);
    bool saveImage;
    bool isConfigCanvases;
    float rotation;    
    ofVec3f skew;
    cv::Mat homography;
    bool isConfigHomograph;
    bool saveMatrix;
    bool loadMatrix;
    bool movingPoint;
    bool matrixReady;
};