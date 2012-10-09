#include "testApp.h"

using namespace ofxCv;
using namespace cv;


void testApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
        //    ofEnableNormalizedTexCoords(); // May as well be explicit
    ofDisableNormalizedTexCoords();
    ofEnableAlphaBlending();
        //ofDisableArbTex(); // YEAAA
    ofDisableLighting();
        //glEnable(GL_TEXTURE_2D);
    setupCamera();
    setupType();
    setupPanel();
    
    activeScene = *new Scene();
    activeScene.loadVideo("movie/test_002_unionjack.mov"); // Test
    activeScene.loadShader("refraction"); // Test
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

void testApp::cvClamp(cv::Mat & mat, float lowerBound, float upperBound) {
    Mat upperThresh = mat;
    Mat lowerThresh = mat;
    Mat dst;
    
    ofxCv::threshold(upperThresh,upperBound,true);
    ofxCv::threshold(lowerThresh,lowerBound,false);
    cvAnd(&upperThresh, &lowerThresh, &curThresh);
}
void testApp::cvClamp(ofImage& mat, float lowerBound, float upperBound) {
    cv::Mat src=toCv(mat);
    cvClamp(src,lowerBound,upperBound);
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    vector<ofPolyline> contours;
    blur(stitched, 20);
        //if(!kinect.isConnected()) convertColor(stitched,stitchedKinect,CV_RGB2GRAY);
        //    Mat stitchedMat = toCv(stitched);
    for(int i=0;i<_thresh->size();i++) {
//        Mat stitchedMatCopy = stitchedMat.clone();
//        cvClamp(stitchedMat,_thresh->at(i).min, _thresh->at(i).max);
//        resize(stitchedMat,smallKinect);
//      contourFinder.setThreshold(127);
//  cvClamp(stitched,_thresh->at(i).min,_thresh->at(i).max); // Output curThresh
        contourFinder.findContours(stitched);
        ofPolyline theShape = getContour(&contourFinder);
        contours.push_back(theShape);
    }
    
    activeScene.updateCrowd(&contours);
}

ofPolyline testApp::getContour(ofxCv::ContourFinder * _contourFinder) {
    ofPolyline poly;
    
        //ofLog() << "Number of polylines: " << ofToString(contourFinder.size());
    
    if(_contourFinder->size() != 0 ) {
        vector<ofPolyline> polylines;
        polylines = _contourFinder->getPolylines();
        for(int i=0; i<polylines.size(); i++) {
                //   ofLog() << "Polyline" << ofToString(i) << " has " << ofToString(polylines[i].size());
            if(i==0) poly = polylines[i];
            if(polylines[i].size() >= poly.size()) poly = polylines[i];
        }
            // ofLog() << "Found contours: " << ofToString(poly.size());
    } 
    poly.close();    
        //poly.simplify(.3);    
    return poly;
}


/******************************************
 Core draw and update
 *******************************************/

void testApp::draw() {
    ofBackground(70, 70, 70);
        //glEnable(GL_DEPTH_TEST);
    ofPushMatrix();
    ofScale(scaleFactor,scaleFactor);
    ofSetColor(255,255,255);   
    if(activeScene.isActive()) {
        activeScene.draw();
            //ofLog() << "Rendering scene";
    }
    ofPopMatrix();
    
    if(debug) {
        ofPushMatrix();
            stitched.draw(0,0);
            activeScene.drawDebug();
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
    panel.addToggle("debug",true);
    panel.addToggle("horiz_flip", false);
    panel.addToggle("vert_flip", false);
    
    panel.addLabel("Main Window");
    panel.addSlider("scaleFactor", .7, 0.005, 1.0, false);
    panel.addSlider("scaleTop", 1.0, 0.05, 1.0, false);
    panel.addSlider("scaleBottom", 1.0, 0.05, 1.0, false);
    panel.addLabel("Image Processing");
    
    panel.addLabel("Animation");
    panel.addLabel("Scene Transition");
    panel.addSlider("transTime",1.06, 1.01,2.0,false);
    panel.addSlider("transType",1, 1, 5,true);
    
    panel.addPanel("Kinect");
        // These cars acount for basic, crass positioning.
        // There should probably be a better solution for this.
        // Either auto stitching, or IDing features on first pass
        // Maybe a config mode?
    panel.addSlider("k1_x",0,-200,200,true);
    panel.addSlider("k1_y",0,-200,200,true);
    panel.addSlider("k2_x",0,-200,200,true);
    panel.addSlider("k2_y",0,-200,200,true);
        // These remove right for k1, left for k2
    panel.addSlider("k1_clear",0,0,1.,false);
    panel.addSlider("k2_clear",0,0,1.,false);
    
    panel.addSlider("kinectAngle",-30,-45,45,true); // Looks down
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
    if(panel.hasValueChanged("scaleFactor"))
        scaleFactor = panel.getValueF("scaleFactor");
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
    kinect.setRegistration(true);
    ofLog() << "Starting first kinect";
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    kinect.setCameraTiltAngle(angle);
    kinectPtr = &kinect;
    imitate(depthImg, kinect);
    
#ifdef USE_TWO_KINECTS
    kinect2.setRegistration(true);
    ofLog() << "Starting second kinect";
    kinect2.init(false, false, true);
    kinect2.open(1);
    kinect2.setCameraTiltAngle(angle);
    kinect2Ptr = &kinect2;
    imitate(depthImg2, kinect2);
#endif
    
    stitched.allocate(kinect.width+kinect2.width, kinect.height, OF_IMAGE_GRAYSCALE);
    smallKinect.allocate((kinect.width+kinect2.width) /4, kinect.height / 4, OF_IMAGE_GRAYSCALE);

    // Incase no kinets, default to cam for testing. Otherwise setup undistort
    if(!kinect.isConnected() || !kinect2.isConnected()) {
        sanityTest.initGrabber(640,480);
        calibration.setFillFrame(true); // true by default
        calibration.load("mbp-2011-isight.yml");
    } else {
        calibration.setFillFrame(true); // true by default
        calibration.load("kinect-ir.yml");
    }
    
}

void testApp::updateCamera() {
    if(sanityTest.isInitialized()) sanityTest.update(); // Just in case
    kinect.update(); 
    kinect2.update(); // Update both kinects
    
    if(kinect.isFrameNew() || sanityTest.isFrameNew()) {
        
        if(!kinect.isConnected() && !kinect2.isConnected() && sanityTest.isInitialized()) {
            // If testing without kinects, copy Sanity twice
            depthImg.setFromPixels(sanityTest.getPixels(), sanityTest.width, sanityTest.height,OF_IMAGE_GRAYSCALE);
            depthImg2.setFromPixels(sanityTest.getPixels(), sanityTest.width, sanityTest.height,OF_IMAGE_GRAYSCALE);
        } else {
            // Copy the kinect depth map to a ofImage obj
            depthImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height,OF_IMAGE_GRAYSCALE);
            depthImg2.setFromPixels(kinect2.getDepthPixels(), kinect2.width, kinect2.height,OF_IMAGE_GRAYSCALE);
        }
        
        // Undistort that image via ofxCv + opencv
        calibration.undistort(toCv(depthImg));
        calibration.undistort(toCv(depthImg2));
        
        // Update the texture in the ofImage object
        depthImg.update();
        depthImg2.update();
        
        // Pass the images to stitching, which stitches via the texture
        stitchKinect(&depthImg,&depthImg2).readToPixels(stitched);

        // Update the texture of the stitched object.
        stitched.update();
    }
}

ofTexture testApp::stitchKinect(ofImage * _k1, ofImage * _k2) {
    if(!stitchedImage.isAllocated()) {
    stitchedImage.allocate(
                           _k1->width+_k2->width
                           , _k1->height
                           , GL_RGBA, 1);
    }
    
    ofVec2f k1offset = ofVec2f(
                           panel.getValueI("k1_x"),
                           panel.getValueI("k1_y")
                               );

    ofVec2f k2offset = ofVec2f(
                               panel.getValueI("k2_x") + _k1->width,
                               panel.getValueI("k2_y")
                               );
    
    float k1crop = panel.getValueI("k1_clear");
    float k2crop = panel.getValueI("k2_clear");
        
    stitchedImage.begin();
        ofPushMatrix();
            ofSetColor(255,255,255); 
            ofTranslate(k1offset);
    
//    _k1->getTextureReference().bind();
//    glBegin(GL_QUADS);  
//    glNormal3f(0.0f,0.0f,1.0f);
//    glTexCoord3f(0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);    
//    glTexCoord3f(k1crop, 0.0f, 0.0f); glVertex3f( _k1->width, 0.0f, 0.0f);    
//    glTexCoord3f(k1crop, 1.0f-k1crop, 0.0f); glVertex3f( _k1->width,_k1->height, 0.0f);    
//    glTexCoord3f(0.0f, 1.0f-k1crop, 0.0f); glVertex3f( 0.0f, _k1->height, 0.0f);    
//    glEnd();  
//    _k1->getTextureReference().unbind();
    
    
        _k1->draw(0,0);

    ofTranslate(k1offset);
//    _k2->getTextureReference().bind();
//    glBegin(GL_QUADS);  
//    glNormal3f(0.0f,0.0f,1.0f);
//    glTexCoord3f(0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);    
//    glTexCoord3f(0-k2crop, 0.0f, 0.0f); glVertex3f( _k2->width, 0.0f, 0.0f);    
//    glTexCoord3f(0-k2crop, 1.0f+k2crop, 0.0f); glVertex3f( _k2->width,_k2->height, 0.0f);    
//    glTexCoord3f(0.0f, 1.0f+k2crop, 0.0f); glVertex3f( 0.0f, _k2->height, 0.0f);       glEnd();  
//    _k2->getTextureReference().unbind();
       _k2->draw(0,0);

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
    activeScene.update();
}

void testApp::transitionScene() {
    
}