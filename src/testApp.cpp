#include "testApp.h"

using namespace ofxCv;
using namespace cv;


void testApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
        //    ofEnableNormalizedTexCoords(); // May as well be explicit
        //ofDisableNormalizedTexCoords();
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

void testApp::createGradient(ofImage * img, float low, float mid, float high) {
    // Taken from http://forum.openframeworks.cc/index.php?topic=1304.0
    ofFbo grad;
    grad.allocate(img->width,img->height);
    grad.begin();
    glBegin(GL_QUAD_STRIP);  
    
    glColor3f(low,low,low);  
    glVertex3f(0, 0,0);  
    glVertex3f( img->width,0,0);  
    
    glColor3f(mid,mid,mid);  
    glVertex3f( 0,img->height/2,0);  
    glVertex3f( img->width, img->height/2,0);  
    
    glColor3f(high,high,high);  
    glVertex3f( 0, img->height,0);  
    glVertex3f( img->width, img->height,0);  
    
    glEnd();  
    grad.end();
    grad.getTextureReference().readToPixels(*img);
}

void testApp::cvClamp(cv::Mat & src, cv::Mat & dst, float lowerBound, float upperBound) {
    cv::Mat upperThresh = toCv(src).clone();
    cv::Mat lowerThresh = toCv(src).clone();
                        //    imitate(upperThresh,src);
                        //    imitate(lowerThresh,src);
    
    imitate(dst,src);
        //upperThresh = src;
        //    lowerThresh = src;
    
    ofxCv::threshold(upperThresh,upperBound,true);
    ofxCv::threshold(lowerThresh,lowerBound,false);
    ofLog() << "Starting cvClamp";
    ofLog() << "upperThresh type:" << ofToString(upperThresh.type()) << "size: "
    << ofToString(upperThresh.cols) << "x" << ofToString(upperThresh.rows);
    ofLog() << "lowerThresh type:" << ofToString(lowerThresh.type()) << "size: "
    << ofToString(lowerThresh.cols) << "x" << ofToString(lowerThresh.rows);
    ofLog() << "Src type:" << ofToString(src.type()) << "size: "
    << ofToString(src.cols) << "x" << ofToString(src.rows);
    ofLog() << "dst type:" << ofToString(dst.type()) << "size: "
    << ofToString(dst.cols) << "x" << ofToString(dst.rows);
    cv::bitwise_and(upperThresh, lowerThresh, dst);
    ofLog() << "Ending cvClamp";
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    vector<ofPolyline> contours;
    blur(stitched, 20);
    imitate(curThresh,stitched);
    
    for(int i=0;i<_thresh->size();i++) {
//        cvClamp(stitchedMat,_thresh->at(i).min, _thresh->at(i).max);
//        resize(stitchedMat,smallKinect);
//      contourFinder.setThreshold(127);
        cv::Mat curThresh;
        cv::Mat curStitched;
        copy(stitched, curStitched);
        
            /* cvClamp(curStitched, 
                curThresh, 
                _thresh->at(i).min, 
                _thresh->at(i).max); // Output curThresh
        */
      contourFinder.findContours(stitched);
            //contourFinder.findContours(curThresh);
        ofPolyline theShape = getContour(&contourFinder);
        contours.push_back(theShape);
    }
    
    activeScene.updateCrowd(&contours);
}

ofPolyline testApp::getContour(ofxCv::ContourFinder * _contourFinder) {
    ofPolyline poly;
    
    if(_contourFinder->size() != 0 ) {
        vector<ofPolyline> polylines;
        polylines = _contourFinder->getPolylines();
        for(int i=0; i<polylines.size(); i++) {
            if(i==0) poly = polylines[i];
            if(polylines[i].size() >= poly.size()) poly = polylines[i];
        }
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
    glEnable(GL_DEPTH_TEST);
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
    
    panel.addPanel("Screen");
    panel.addLabel("Gradient");
    panel.addSlider("grad_low", 0, 0, 1., false);
    panel.addSlider("grad_mid", 0, 0, 1., false);
    panel.addSlider("grad_high", 0, 0, 1., false);
    
    panel.addLabel("Panning");
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
    
    if( panel.hasValueChanged("grad_low") ||
        panel.hasValueChanged("grad_mid") ||
        panel.hasValueChanged("grad_high") ) 
    {
        gradientChanged = true;
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
    
    //Second 
    kinect2.setRegistration(true);
    ofLog() << "Starting second kinect";
    kinect2.init(false, false, true);
    kinect2.open(1);
    kinect2.setCameraTiltAngle(angle);
    kinect2Ptr = &kinect2;
    imitate(depthImg2, kinect2);
    
    stitched.allocate(kinect.width+kinect2.width, kinect.height, OF_IMAGE_GRAYSCALE);
    smallKinect.allocate((kinect.width+kinect2.width) /4, kinect.height / 4, OF_IMAGE_GRAYSCALE);

    // Incase no kinets, default to cam for testing. Otherwise setup undistort
    if(!kinect.isConnected() || !kinect2.isConnected()) {
        ofLog() << "Setting up mbp 2011 isight Calib";
        sanityTest.initGrabber(640,480);
        calibration.setFillFrame(true); // true by default
        calibration.load("mbp-2011-isight.yml");
        depthImg.allocate(sanityTest.width, sanityTest.height, OF_IMAGE_GRAYSCALE);
        imitate(depthImg2, depthImg);
    } else {
        ofLog() << "Setting up Kinect Calib";
        calibration.setFillFrame(true); // true by default
        calibration.load("kinect-ir.yml");
        calibration2.setFillFrame(true); // true by default
        calibration2.load("kinect-ir.yml");
    }
    imitate(distGradient,stitched);
}

void testApp::updateCamera() {
    if(sanityTest.isInitialized()) sanityTest.update(); // Just in case
    kinect.update(); 
    kinect2.update(); // Update both kinects
    
    if(gradientChanged) { 
        createGradient(
                   &distGradient,
                   panel.getValueF("grad_low"),
                   panel.getValueF("grad_mid"),
                   panel.getValueF("grad_high") );
        gradientChanged = false;
    }
                                
    
    if(kinect.isFrameNew() || sanityTest.isFrameNew()) {
        
        if(sanityTest.isInitialized()) {
            // If testing without kinects, copy Sanity twice
            sanityTest.getTextureReference().readToPixels(depthImg);
            convertColor(depthImg, depthImg2, CV_RGB2GRAY);
            depthImg = depthImg2;
            depthImg2.mirror(0, 1);
        } else {
            // Copy the kinect depth map to a ofImage obj
            kinect.getDepthTextureReference().readToPixels(depthImg);
            kinect2.getDepthTextureReference().readToPixels(depthImg2);
        }
        
        // Undistort that image via ofxCv + opencv
        calibration.undistort(toCv(depthImg));
        calibration.undistort(toCv(depthImg2));
        
        // Update the texture in the ofImage object
        depthImg.reloadTexture();
        depthImg2.reloadTexture();
        
        // Pass the images to stitching, which stitches via the texture
        stitchKinect(&depthImg,&depthImg2).readToPixels(stitched);
        
        cv::Mat toCorrect; imitate(toCorrect,stitched); 
        toCorrect = toCv(stitched);
        
        cv::Mat gradientMat; imitate(gradientMat,distGradient);
        gradientMat = toCv(distGradient);
        
        cv:Mat stitchedMat; imitate(stitchedMat,stitched);
        ofLog() << "About to add grad";
            //        cvAnd( &toCorrect , &gradientMat, &stitchedMat );
        stitchedMat = toCorrect + gradientMat;
        ofLog() << "Done grad";
        ofxCv::toOf(stitchedMat, stitched);

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
        //  ofClear(255);
        ofPushMatrix();
            ofSetColor(255,255,255); 
            ofTranslate(k1offset);
        //  glEnable(GL_DEPTH_TEST);
//    _k1->getTextureReference().bind();
//    glBegin(GL_QUADS);  
//    glNormal3f(0.0f,0.0f,1.0f);
//    glTexCoord3f(0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);    
//    glTexCoord3f(k1crop, 0.0f, 0.0f); glVertex3f( _k1->width, 0.0f, 0.0f);    
//    glTexCoord3f(k1crop, 1.0f-k1crop, 0.0f); glVertex3f(  _k1->width, _k1->height, 0.0f);    
//    glTexCoord3f(0.0f, 1.0f-k1crop, 0.0f); glVertex3f( 0.0f, _k1->height, 0.0f);    
//    glEnd();  
//    _k1->getTextureReference().unbind();
    
    
        _k1->draw(0,0);

    ofTranslate(k2offset);
//    _k2->getTextureReference().bind();
//    glBegin(GL_QUADS);  
//    glNormal3f(0.0f,0.0f,1.0f);
//    glTexCoord3f(0.0f, 0.0f, 0.0f); glVertex3f( 0.0f, 0.0f, 0.0f);    
//    glTexCoord3f(0-k2crop, 0.0f, 0.0f); glVertex3f( _k2->width, 0.0f, 0.0f);    
//    glTexCoord3f(0-k2crop, 1.0f+k2crop, 0.0f); glVertex3f( _k2->width,_k2->height, 0.0f);    
//    glTexCoord3f(0.0f, 1.0f+k2crop, 0.0f); glVertex3f( 0.0f, _k2->height, 0.0f);       glEnd();  
//    _k2->getTextureReference().unbind();
        _k2->draw(0,0);
        // glDisable(GL_DEPTH_TEST);
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