#include "testApp.h"

using namespace ofxCv;
using namespace cv;


void testApp::setup() {
    setupType();
    setupPanel();
   }


// Update takes in new frame data and passes it to the canvas
void testApp::update() {
    updateConditional();
    updateCamera();
}

/******************************************
 Core draw and update
 *******************************************/

void testApp::draw() {
     
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);
    ofPushMatrix();
        ofTranslate(panX,panY);
        ofScale(panel.getValueF("scaleWindow"),panel.getValueF("scaleWindow"));
    if(flipv) {
        ofRotate(180, 1,0,0);
    }
    if(fliph) {
        ofRotate(180,0,1,0);
    }
        for(int i=0; i<canvases.size(); i++) {
            if(i!=0) ofTranslate(canvases[i-1]->width*2);
            canvases[i]->draw();
        }
    ofPopMatrix();
    
    if(debug) {
        ofPushMatrix();
            ofNoFill();
            ofTranslate(ofGetWidth()-cam.width,0);
            cam.draw(0,0);
            graySmall.draw(0,cam.height);
            ofScale(1 / scaleFactor, 1 / scaleFactor);
            for(int i = 0; i < objects.size(); i++) {
                    //ofLog() << "Drawing face #" << ofToString(i);
                ofRect(toOf(objects[i]));
            }
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

void testApp::mousePressed(int x, int y, int button) {
  }

void testApp::mouseDragged(int x, int y, int button) {

}

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

bool testApp::movePoint(vector<ofVec2f>& points, ofVec2f point) {
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
    if(scaleFactor != panel.getValueF("faceScale")) {
        graySmall.allocate(cam.getWidth() * panel.getValueF("faceScale"), cam.getHeight() * panel.getValueF("faceScale"), OF_IMAGE_GRAYSCALE);
    }
    
    scaleFactor = panel.getValueF("faceScale");
    
    panX = panel.getValueF("panX");
    panY = panel.getValueF("panY");
    
    if(panel.getValueB("debug")) setDebug(true);
    else setDebug(false);
    
    if(panel.getValueB("add100Faces")) {
        for(int i=0;i<canvases.size();i++) {
            canvases[i]->testImages();
        }
         
        panel.setValueB("add100Faces",false);
    }
    
    if(panel.getValueB("horiz_flip")) {
        fliph = 1.0;
    } else {
        fliph = 0;
    }

    if(panel.getValueB("vert_flip")) {
        flipv = 1.0;
    } else {
        flipv = 0;
    }
    
    if(panel.hasValueChanged("circleResolution")) {
        /*        for(int i=0; canvas1->size(); i++) {
         canvas1->get(i).resolution = panel.getValueI("circleResolution");
         //                canvas1->panel.getValueI("makeInactive");
         }*/
    }
    if(panel.hasValueChanged("circleResolution")) {
            //        panel.getValueF("simplifyPolyline");
    }
    
    panel.getValueI("makeInactive");
    
    panel.getValueF("faceDetectSmothing");
    
    panel.getValueI("distanceThresh");    
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
    panel.addSlider("kinectSkew", 0, 0, 1.0, false);
    panel.addSlider("kinectLowpass", 0.0, 0, 1., false);
    panel.addSlider("kinectHighpass",1.0,0,1.,false);    
    
    panel.addPanel("Screen Move");
    panel.addSlider("panX", 0, -1000, 1000, true);
    panel.addSlider("panY", 0, -1000, 1000, true);
}

void testApp::setupType() {
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofTrueTypeFont::setGlobalDpi(72);
    raleway.loadFont("Raleway-Thin.ttf", 32, true, true);
    raleway.setLineHeight(18.0f);
    raleway.setLetterSpacing(1.037);
}

void testApp::updateCamera() {
}

void testApp::setDebug(bool _debug) {
    debug = _debug;
}

void testApp::exit() {

}