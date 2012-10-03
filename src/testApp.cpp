#include "testApp.h"

using namespace ofxCv;
using namespace cv;


void testApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
        //    ofEnableNormalizedTexCoords(); // May as well be explicit
        //    ofDisableArbTex(); // YEAAA
    ofDisableLighting();
    glEnable(GL_TEXTURE_2D);
    setupCamera();
    setupType();
    setupPanel();
   }

void testApp::exit() {
        // kinect.setCameraTiltAngle(0);
    kinect.close();
    kinect.clear();
#ifdef USE_TWO_KINECTS
        //    kinect2.setCameraTiltAngle(0);
    kinect2.close();
    kinect2.clear();
#endif
}

// Update takes in new frame data and passes it to the canvas
void testApp::update() {
    updateConditional();
    updatePanel();
    updateCamera();
    updateActiveScene();
    ranges.clear();
    for(int i=1;i<numThresh+1;i++) {
        
        Range range = Range();
        range.min = panel.getValueF("range" + ofToString(i) + "min");
        range.max = panel.getValueF("range" + ofToString(i) + "max");
        ranges.push_back(range);

    }
    getScene(&stitchedKinect,&ranges);
}

void testApp::cvClamp(Mat & mat, float lowerBound, float upperBound) {
    Mat upperThresh = mat;
    Mat lowerThresh = mat;
    
    ofxCv::threshold(upperThresh,upperBound,true);
    ofxCv::threshold(lowerThresh,lowerBound,false);
    cvAnd(&upperThresh, &lowerThresh, &mat);
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    vector<ofPolyline> contours;
//    blur(stitched, 10);
//    if(!kinect.isConnected()) convertColor(stitched,stitchedKinect,CV_RGB2GRAY);
//    Mat stitchedMat = toCv(stitched);
//    for(int i=0;i<_thresh->size();i++) {
//        Mat stitchedMatCopy = stitchedMat.clone();
//        cvClamp(stitchedMat,_thresh->at(i).min, _thresh->at(i).max);
//        resize(stitchedMat,smallKinect);
//        contourFinder.setThreshold(127);
//        contourFinder.findContours(smallKinect);
//        contours.push_back(getContour(&contourFinder));
//    }
    
    
    for(int i=0;i<contours.size();i++) {
        layer[i]->update(&contours[i]);
        
    }   
}

ofPolyline testApp::getContour(ofxCv::ContourFinder * _contourFinder) {
    ofPolyline poly;
    
    ofLog() << "Number of polylines: " << ofToString(contourFinder.size());
    
    if(_contourFinder->size() != 0 ) {
        vector<ofPolyline> polylines;
        polylines = _contourFinder->getPolylines();
        for(int i=0; i<polylines.size(); i++) {
            ofLog() << "Polyline" << ofToString(i) << " has " << ofToString(polylines[i].size());
            if(i==0) poly = polylines[i];
            if(polylines[i].size() >= poly.size()) poly = polylines[i];
        }
        ofLog() << "Found contours: " << ofToString(poly.size());
    } 
    poly.simplify(.3);    
    poly.close();    
    return poly;
}


/******************************************
 Core draw and update
 *******************************************/

void testApp::draw() {
    ofBackground(70, 70, 70);
    ofSetColor(255,255,255);   

        //    kinectPtr->drawDepth(0,0,kinect.width,kinect.height);
        //        depthImg.draw(0,0);
        //        kinect2Ptr->drawDepth(kinect.width,0,kinect2.width,kinect2.height);
                stitched.draw(0,depthImg.height);
            sanityTest.draw(0,0);

    
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
    
    for(int i=1;i<panel.getValueI("NumRanges")+1;i++) {
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


/******************************************
    Various Camera functions 
 *******************************************/
void testApp::setupCamera() {
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    
    angle = 0;
    sanityTest.initGrabber(640,480);
    kinect.setRegistration(true);
    ofLog() << "Starting first kinect";
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    kinect.setCameraTiltAngle(angle);
    kinectPtr = &kinect;
    depthImg.allocate(kinect.width, kinect.height,OF_IMAGE_GRAYSCALE);
#ifdef USE_TWO_KINECTS
    ofLog() << "Starting second kinect";
    kinect2.setRegistration(true);
    kinect2.init(false, false, true);
    kinect2.open(1);
    kinect2Ptr = &kinect2;
    kinect2.setCameraTiltAngle(angle);
#endif
    stitched.allocate(kinect.width+kinect2.width, kinect.height, OF_IMAGE_GRAYSCALE);
    smallKinect.allocate((kinect.width+kinect2.width) /4, kinect.height / 4, OF_IMAGE_GRAYSCALE);
                            
}

void testApp::updateCamera() {
    sanityTest.update();
    kinect.update();
    depthImg.setFromPixels(kinectPtr->getDepthPixels(), kinect.width, kinect.height,OF_IMAGE_GRAYSCALE);
#ifdef USE_TWO_KINECTS
    kinect2.update();
    if(kinect.isFrameNew()) {
        stitchKinect(&kinect,&kinect2).readToPixels(stitched);
        stitched.update();
        //ofLog() << "Dist from center:" << ofToString(kinect.getColorAt(kinect.width/2,kinect.height/2));
    }
#endif
    
    // In case of no kinects, break glass
    if(!kinect.isConnected() && !kinect2.isConnected()) {
        stitchedImage.allocate(
                               sanityTest.getWidth()*2,
                               sanityTest.getHeight()
                                   );
        
        stitchedImage.begin();
            ofSetColor(255,255,255); 
            ofPushMatrix();
            ofTranslate(0,0);   sanityTest.draw(0,0);
            ofTranslate(sanityTest.getWidth(),0);   sanityTest.draw(0,0);
            ofPopMatrix();
        stitchedImage.end();
        stitchedImage.getTextureReference().readToPixels(stitched);
        stitched.update();
    }
}

ofTexture testApp::stitchKinect(ofxKinect * _k1, ofxKinect * _k2) {
    if(!stitchedImage.isAllocated()) {
    stitchedImage.allocate(
                           _k1->width+_k2->width
                           , _k1->height
                           , GL_RGBA, 1);
    }
    
    stitchedImage.begin();
        ofPushMatrix();
            ofSetColor(255,255,255); 
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

/******************************************
 Scene management functions 
 *******************************************/

void testApp::updateActiveScene() {
    
}

void testApp::transitionScene() {
    
}