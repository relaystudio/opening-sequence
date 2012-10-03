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

    // Scene related
    void updateActiveScene();
    void transitionScene();
    ofPolyline getContour(ofxCv::ContourFinder * _contourFinder);
    
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
    ofImage depthImg;
    ofxKinect * kinectPtr;
    float angle; // kinect angle across both
#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
    ofxKinect * kinect2Ptr;
    cv::Mat kinect1Homo;
    cv::Mat kinect2Homo;
#endif
    ofTexture stitchKinect(ofxKinect * _k1, ofxKinect * _k2);
    ofFbo stitchedImage;
    ofImage stitched;
    ofVideoGrabber sanityTest;
    cv::Mat stitchedKinect;
    ofImage smallKinect;
    void cvClamp(cv::Mat& mat, float lowerBound, float upperBound);
    
    ofxCv::ContourFinder contourFinder;
    vector<Range> ranges;
    
    ofxCv::RunningBackground background;
    
    // Blobs
    void getScene(cv::Mat * _frame, vector<Range> * _thresh);
    
    
    //Events

    void keyPressed(int key);
    
private:
    
};