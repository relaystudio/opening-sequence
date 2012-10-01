#include "testApp.h"

using namespace ofxCv;
using namespace cv;


void testApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
        //    ofEnableNormalizedTexCoords(); // May as well be explicit
        //    ofDisableArbTex(); // YEAAA
    setupCamera();
    setupType();
    setupPanel();
   }


// Update takes in new frame data and passes it to the canvas
void testApp::update() {
    updateConditional();
    updatePanel();
    updateCamera();
    for(int i=0;i<numThresh;i++) {
        
        Range range = Range();
        range.min = panel.getValueF("range" + ofToString(i) + "min");
        range.max = panel.getValueF("range" + ofToString(i) + "max");
        ranges.push_back(range);

    }
    getScene(&stitchedKinect,&ranges);
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    vector<ofPolyline> contours;
    
    for(int i=0;i<_thresh->size();i++) {
            //contourFinder.setThreshold( th );
            //contourFinder.findContours(cam);
    }
    
    for(int i=0;i<contours.size();i++) {
        layer[i]->update(&contours[i]);
    }   
}


/******************************************
 Core draw and update
 *******************************************/

void testApp::draw() {
    ofBackground(100, 100, 100);
    ofPushMatrix();
        ofScale(panel.getValueF("scaleWindow"),panel.getValueF("scaleWindow"));
    if(flipv) {
        ofRotate(180, 1,0,0);
    }
    if(fliph) {
        ofRotate(180,0,1,0);
    }

    ofPopMatrix();
    
    if(debug) {
        ofPushMatrix();
            kinect.drawDepth(0,0,kinect.width,kinect.height);
#ifdef USE_TWO_KINECTS
            kinect2.drawDepth(kinect.width,0,kinect2.width,kinect2.height);
#endif
        stitched.draw(0,depthImg.height);
        ofPopMatrix();
    }
       
    GLboolean isDepthTesting;
    glGetBooleanv(GL_DEPTH_TEST, &isDepthTesting);
    if(isDepthTesting == GL_TRUE)
        glDisable(GL_DEPTH_TEST);
}


/******************************************
 Events and interface
 *******************************************/             


void testApp::keyPressed(int key) {
    if(key == ' ') {
        background.reset();
    }
}

/******************************************
 Homographic transforms + img change
 *******************************************/
void testApp::updateHomography() {
    
}

void testApp::saveHomography() {
    
}

bool testApp::loadHomography( string * path) {
    return false;
}


bool testApp::loadMap(string * path) {
    ofFile previous(*path);
    if(previous.exists()) {
        FileStorage fs(ofToDataPath(*path), FileStorage::READ);
        fs[*path] >> homography;
        matrixReady = true;
    }
    return false;
}

void testApp::parseMap(ofImage * map) {
    
}

/******************************************
 Various setup functions 
 *******************************************/

void testApp::updateConditional() {
    
    if(panel.getValueB("debug")) setDebug(true);
    else setDebug(false);
    
    if(panel.getValueB("horiz_flip")) fliph = 1.0;
    else fliph = 0;
    

    if(panel.getValueB("vert_flip")) flipv = 1.0;
    else flipv = 0;
    
}

void testApp::setupPanel() {
        //Setup panel
    panelWidth = 200;
    panel.setup(panelWidth, 800);
    
    panel.addPanel("Setup");
    panel.addLabel("Debug switches");
    panel.addToggle("debug",false);
    panel.addToggle("horiz_flip", false);
    panel.addToggle("vert_flip", false);
    
    panel.addLabel("Main Window");
    panel.addSlider("scaleWindow", 1.0, 0.005, 1.0, false);
    panel.addSlider("scaleTop", 1.0, 0.05, 1.0, false);
    panel.addSlider("scaleBottom", 1.0, 0.05, 1.0, false);
    panel.addLabel("Image Processing");
    
    panel.addLabel("Animation");
    panel.addLabel("Scene Transition");
    panel.addSlider("transTime",1.06, 1.01,2.0,false);
    panel.addSlider("transType",1, 1, 5,true);
    
    panel.addPanel("Kinect");
    panel.addSlider("kinectAngle",0,-45,45,true);
    panel.addSlider("kinectSkew", 0, 0, 1.0, false);
    panel.addSlider("kinectLowpass", 0.0, 0, 1., false);
    panel.addSlider("kinectHighpass",1.0,0,1.,false);    
    
    panel.addPanel("Ranges");
    panel.addSlider("NumRanges", 4,1,6,true);
    for(int i=1;i<panel.getValueI("NumRanges")+1;i++) {
        panel.addSlider("range" + ofToString(i) + "min",i*.1,0.1,1.0,false);
        panel.addSlider("range" + ofToString(i) + "max",i*.1+.1,0.1,1.0,false);
    }
    
    panel.addPanel("Screen Move");
    panel.addSlider("panX", 0, -1000, 1000, true);
    panel.addSlider("panY", 0, -1000, 1000, true);
}

void testApp::updatePanel() {
    numThresh = panel.getValueI("NumRanges");
    
    if(panel.hasValueChanged("kinectAngle")) {
            angle = panel.getValueI("kinectAngle");
            kinect.setCameraTiltAngle(angle);
            kinect2.setCameraTiltAngle(angle);
    }
    
    for(int i=0;i<panel.getValueI("NumRanges");i++) {
        if(panel.getValueI("range" + ofToString(i) + "min")) {
            panel.addSlider("range" + ofToString(i) + "min", 0,1.0,false);
            panel.addSlider("range" + ofToString(i) + "max", 0,1.0,false);
        }
    }
}

void testApp::setupType() {
    ofTrueTypeFont::setGlobalDpi(72);
    raleway.loadFont("Raleway-Thin.ttf", 32, true, true);
    raleway.setLineHeight(18.0f);
    raleway.setLetterSpacing(1.037);
}

void testApp::setupCamera() {
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    
    angle = 0;
    
    kinect.setRegistration(true);
    kinect.init(false, false, true); // infrared=false, video=true, texture=true)
    kinect.open(0);
    kinect.setCameraTiltAngle(angle);
    
    cam = &kinect;
    depthImg.allocate(kinect.width, kinect.height,OF_IMAGE_GRAYSCALE);
#ifdef USE_TWO_KINECTS
    ofLog() << "Starting second kinect";
    kinect2.setRegistration(true);
    kinect2.init(false, false, true);
    kinect2.open(1);
    kinect2.setCameraTiltAngle(angle);
#endif
}

void testApp::updateCamera() {
    kinect.update();
#ifdef USE_TWO_KINECTS
    kinect2.update();
    if(kinect.isFrameNew()) {
            //if(kinect2.isFrameNew()) {
            //depthImg.setFromPixels(kinect.getDepthPixels(),kinect.width, kinect.height, OF_IMAGE_GRAYSCALE,true);
            stitched = stitchKinect(&kinect,&kinect2);
                //stitched.update();
                //}
    }
#endif
}

ofTexture testApp::stitchKinect(ofxKinect * _k1, ofxKinect * _k2) {
    ofFbo stitchedImage;
    stitchedImage.allocate(
                           _k1->width+_k2->width
                           , _k1->height
                           );
    stitchedImage.begin();
        ofPushMatrix();

            ofTranslate(0,0);
            _k1->drawDepth(0,0);

            ofTranslate(_k1->width,0);
            _k2->drawDepth(0,0);

        ofPopMatrix();
    stitchedImage.end();
    return stitchedImage.getTextureReference();
}

void testApp::setDebug(bool _debug) {
    debug = _debug;
}

void testApp::exit() {
    kinect.close();
#ifdef USE_TWO_KINECT
    kinect2.close();
#endif
}