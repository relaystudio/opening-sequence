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
        float min;
        float max;
    } Range;
    
public:
        //   virtual void setup();
        //    testApp();
        //    ~testApp();
    void setup();
    
    void update();
    void draw();
    void exit();
    
        //vector<Scene*> scene; // Holds scene objects, pass pointers
        //vector<Crowd*> crowd; // Holds ALL user obj, pass pointers

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
    void updateTransition();
    ofVideoPlayer trans;
    ofPolyline getContour(ofxCv::ContourFinder * _contourFinder);
    void cvStitch(cv::Mat& dst, ofImage * _k1, ofImage * _k2);
    
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
    vector<ofPolyline> contours;
    
    bool transActive;
    float threshold;
    
    //Camera
    ofxCv::Calibration calibration;
    ofxCv::Calibration calibration2;
    ofImage depthImg, depthImg2;
    
    ofImage clamp[4];
    
    ofxKinect kinect;
    ofxKinect * kinectPtr;
    float angle; // kinect angle across both
#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
    ofxKinect * kinect2Ptr;
    cv::Mat kinect1Homo;
    cv::Mat kinect2Homo;
#endif

    ofTexture stitchKinect(ofImage * _k1, ofImage * _k2);
    cv::Mat cvStitch(ofImage * _k1, ofImage * _k2);
    void drawSkew(ofImage * src, ofPoint * tl, ofPoint* tr, ofPoint* br, ofPoint* bl);

    ofFbo stitchedImage;
    ofImage stitched;
    ofVideoGrabber sanityTest;
    cv::Mat stitchedKinect;
    cv::Mat bgSub;
    ofImage smallKinect;
    void cvClamp(cv::Mat& src, cv::Mat & dst, float lowerBound, float upperBound);
    void cvClamp(ofImage& src, float lowerBound, float upperBound);
    cv::Mat curThresh;
    
    ofImage fullSized;
    vector<ofImage> clamped;
    ofxCv::ContourFinder contourFinder;
    vector<Range> ranges;
    
    ofxCv::RunningBackground background;
    
    // Blobs
    void getScene(cv::Mat * _frame, vector<Range> * _thresh);
    
    void createGradient(ofImage * img, float low, float mid, float high);
    
    //Scene activeScene;
    //Scene newScene;
    
    Scene scene[2];
    
    int activeScene, newScene;
    ofShader transShader;
    
    //Events
    void makeNewScene(int sceneNum);
    void makeNewScene();
    void loadRandomTransition();
    void keyPressed(int key);
    
private:
    int panx, pany;
    float scaleFactor;
    ofImage distGradient;
    cv::Mat gradientMat;
    bool gradientChanged;
    bool transition;
    float transFade;
};