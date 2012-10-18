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
                                                           //activeScene.loadShader("refraction"); // Test
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
    updatePanel();
    updateConditional();
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
    ofLog() << "Generating gradiant";
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
    ofxCv::imitate(dst,src);
    
//    
    cv::Mat upperThresh;// = toCv(src).clone();
    cv::Mat lowerThresh;// = toCv(src).clone();
    copy(src, upperThresh);
    copy(src, lowerThresh);

    ofxCv::threshold(upperThresh,upperBound*255,false);
    ofxCv::threshold(lowerThresh,lowerBound*255,true);
    dst = lowerThresh-upperThresh;
        //    ofLog() << "Ending cvClamp";
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    contours.clear();
        //  blur(stitched, 20);
    cv::Mat curThresh;
    cv::Mat curStitched;
    
    curStitched = toCv(stitched).clone();//.clone();
        //copy(curStitched,curThresh);
    imitate(curThresh, curStitched);
    
    for(int i=0;i<4;i++) {
            //        imitate(curThresh,curStitched); 
         cvClamp(curStitched, 
                 curThresh, 
                 _thresh->at(i).min, 
                 _thresh->at(i).max); // Output curThresh

        
        blur(curThresh, 40);
        
        if(debug) {
            ofxCv::toOf(curThresh, clamp[i]);
            clamp[i].reloadTexture();
        }
            //resize(stitched, fullSized);
        contourFinder.findContours(curThresh);
            //contourFinder.findContours(curThresh);
        ofPolyline theShape = getContour(&contourFinder);
        contours.push_back(theShape);
    }
        //ofLog() << "There are " << ofToString(contours.size()) << "Contours";
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
    glDisable(GL_DEPTH_TEST);
    ofBackground(70, 70, 70);
        //glEnable(GL_DEPTH_TEST);
    ofPushMatrix();
    ofScale(scaleFactor,scaleFactor);
    ofSetColor(255,255,255);   
    if(activeScene.isActive()) {
        activeScene.draw();
            //ofLog() << "Rendering scene";
    }
    if(debug) {
        ofPushMatrix();
        stitched.draw(0,0);
        activeScene.drawDebug();
        
        ofTranslate(ofGetWidth()-200,0);
        ofSetColor(255);
        for(int i=0;i<4;i++) {
            clamp[i].draw(0,i*100,200,100);
        }
        
        ofPopMatrix();
    }
    ofPopMatrix();
    

    
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
    panel.addToggle("flipKinect", false);
    panel.addToggle("mirrorKinect", false);
    
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
        // These vars acount for basic, crass positioning.
        // There should probably be a better solution for this.
        // Either auto stitching, or IDing features on first pass
        // Maybe a config mode?
    panel.addSlider("k1_x",0,-400,400,true);
    panel.addSlider("k1_y",0,-400,400,true);
    panel.addSlider("k2_x",0,-400,400,true);
    panel.addSlider("k2_y",0,-400,400,true);
        // These remove right for k1, left for k2
    panel.addSlider("k1_clear",0,0,1.,false);
    panel.addSlider("k2_clear",0,0,1.,false);
    
    panel.addSlider("kinectAngle",-30,-45,45,true); // Looks down
    panel.addSlider("kHorizKey", 0, 0, 1.0, false);
    panel.addSlider("kVertKey", 0, 0, 1.0, false);
    panel.addSlider("kinectLowpass", 0.0, 0, 1., false);
    panel.addSlider("kinectHighpass",1.0,0,1.,false);    
    
    panel.addPanel("Kinect1Skew");
    panel.addSlider("k1_tl_x",0,0,1.,false);
    panel.addSlider("k1_tl_y",0,0,1.,false);
    
    panel.addSlider("k1_tr_x",1.,0,1.,false);
    panel.addSlider("k1_tr_y",0,0,1.,false);
    
    panel.addSlider("k1_br_x",1.,0,1.,false);
    panel.addSlider("k1_br_y",1.,0,1.,false);
    
    panel.addSlider("k1_bl_x",0,0,1.,false); 
    panel.addSlider("k1_bl_y",1.,0,1.,false);

    panel.addPanel("Kinect2Skew");
    panel.addSlider("k2_tl_x",0,0,1.,false);
    panel.addSlider("k2_tl_y",0,0,1.,false);
    
    panel.addSlider("k2_tr_x",1.,0,1.,false);
    panel.addSlider("k2_tr_y",0,0,1.,false);
    
    panel.addSlider("k2_br_x",1.,0,1.,false);
    panel.addSlider("k2_br_y",1.,0,1.,false);
    
    panel.addSlider("k2_bl_x",0,0,1.,false); 
    panel.addSlider("k2_bl_y",1.,0,1.,false);
    
    panel.addPanel("Ranges");
    panel.addSlider("NumRanges", 4,1,6,true);
    for(int i=1;i<panel.getValueI("NumRanges")+1;i++) {
        panel.addSlider("range" + ofToString(i) + "min",i*.2,0.1,1.0,false);
        panel.addSlider("range" + ofToString(i) + "max",i*.2+.1,0.1,1.0,false);
    }
    
    panel.addPanel("Screen");
    panel.addLabel("Gradient");
    panel.addToggle("changeGradient", true);
    panel.addSlider("grad_low", 0, 0, 1., false);
    panel.addSlider("grad_mid", .4, 0, 1., false);
    panel.addSlider("grad_high", 0.8, 0, 1., false);
    
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
    
    if( panel.getValueB("changeGradient") ) 
    {
        createGradient(
                       &distGradient,
                       panel.getValueF("grad_low"),
                       panel.getValueF("grad_mid"),
                       panel.getValueF("grad_high") );
        panel.setValueB("changeGradient", false);
    }
    
    for(int i=1;i<panel.getValueI("NumRanges")+1;i++) {
        if(panel.getValueI("range" + ofToString(i) + "min")) {
            panel.addSlider("range" + ofToString(i) + "min", 0,1.0,false);
            panel.addSlider("range" + ofToString(i) + "max", 0,1.0,false);
        }
    }
}

void testApp::setupType() {
//    ofTrueTypeFont::setGlobalDpi(72);
//    raleway.loadFont("Raleway-Thin.ttf", 32, true, true);
//    raleway.setLineHeight(18.0f);
//    raleway.setLetterSpacing(1.037);
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
    
        // fullSized.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_GRAYSCALE);
    stitched.allocate(kinect.width+kinect2.width, kinect.height, OF_IMAGE_GRAYSCALE);
        //smallKinect.allocate((kinect.width+kinect2.width) /4, kinect.height / 4, OF_IMAGE_GRAYSCALE);

    // Incase no kinects, default to cam for testing. Otherwise setup undistort
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
    createGradient(
                   &distGradient,
                   panel.getValueF("grad_low"),
                   panel.getValueF("grad_mid"),
                   panel.getValueF("grad_high") );

}

void testApp::updateCamera() {
    if(sanityTest.isInitialized()) 
    { 
        sanityTest.update(); // Just in case
    }
    else {
        kinect.update(); 
        kinect2.update(); // Update both kinects
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
//        depthImg.reloadTexture();
//        depthImg2.reloadTexture();
        
        // Pass the images to stitching, which stitches via the texture
//        try {
//            stitchKinect(&depthImg,&depthImg2);//.readToPixels(stitched);
//        } catch (int fuck) {
//            ofLog() << ofToString(fuck);
//        }
        cv::Mat kstitched;
            //        cv::Mat kstitched = cvStitch(&depthImg, &depthImg2);
        cvStitch(kstitched, &depthImg, &depthImg2);
        
            //        ofxCv::toOf(kstitched,stitched);    
            //imitate(stitched, kstitched);
            //ofxCv::copy(kstitched, stitched);
        
            //cv::Mat resizedGrad; imitate(resizedGrad,kstitched); 
        cv::Mat gradientMat; imitate(gradientMat, kstitched);
        cv::Mat gradientMatRoi = gradientMat(cv::Rect(
                                0,0,distGradient.width,distGradient.height));
        toCv(distGradient).clone().copyTo(gradientMatRoi);
        
            //        gradientMat = toCv(distGradient).clone();     
            // ofxCv::resize(gradientMat, resizedGrad); 
            //  ofLog() << "Done grad";
            //        ofLog() << "About to add grad";
            //        cvAnd( &toCorrect , &gradientMat, &stitchedMat );
        cv::add(kstitched, gradientMat, kstitched);
            //        toCorrect += gradientMat;
            //        ofxCv::resize(toCorrect, stitchedMat, INTER_NEAREST);
            //        ofxCv::toOf(stitchedMat, stitched);

        ofxCv::toOf(kstitched, stitched);
        stitched.update();
    }
}

ofTexture testApp::stitchKinect(ofImage * _k1, ofImage * _k2) {
    if(!stitchedImage.isAllocated()) {
    stitchedImage.allocate(
                           _k1->width+_k2->width
                           , _k1->height);
        ofLog() << "Allocating stitched image";
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
    bool flip = panel.getValueB("flipKinect");
    bool mirror = panel.getValueB("mirrorKinect");
    
    int w=_k1->getWidth();
    int h=_k1->getHeight();
    
    
    if(mirror) {
        _k1->mirror(0,1);
        _k2->mirror(0,1);
    }
    
    stitchedImage.begin();
        ofClear(0);
    ofPushMatrix();
        ofSetColor(255); 
            ofTranslate(k1offset);

        glPushMatrix();
        //glMultMatrixf(myMatrix);
          if(flip) _k1->drawSubsection(60, 40, _k1->getWidth()-60, _k1->getHeight()-40, 0, 0);
          else _k2->drawSubsection(60, 40, _k2->getWidth()-60, _k2->getHeight()-40, 0, 0);
        glPopMatrix();
    

            ofTranslate(k2offset);

    
    if(flip) _k2->drawSubsection(60, 40, _k2->getWidth()-60, _k2->getHeight()-40, 0, 0);
    else _k1->drawSubsection(60, 40, _k1->getWidth()-60, _k1->getHeight()-40, 0, 0);
    // ofPopMatrix();
    
    
    
    ofPopMatrix();
    stitchedImage.end();
    
    stitchedImage.getTextureReference().readToPixels(stitched);
    
    return stitchedImage.getTextureReference();
}

void testApp::cvStitch(cv::Mat & dst, ofImage * _k1, ofImage * _k2) {    
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
    bool flip = panel.getValueB("flipKinect");
    bool mirror = panel.getValueB("mirrorKinect");
    
    int w=_k1->getWidth();
    int h=_k1->getHeight();
    
    if(mirror) {
        _k1->mirror(0,1);
        _k2->mirror(0,1);
    }
    
    
ofVec2f k1tl = ofVec2f(panel.getValueF("k1_tl_x"), panel.getValueF("k1_tl_y"));
ofVec2f k1tr = ofVec2f(panel.getValueF("k1_tr_x"), panel.getValueF("k1_tr_y"));
ofVec2f k1br = ofVec2f(panel.getValueF("k1_br_x"), panel.getValueF("k1_br_y"));
ofVec2f k1bl = ofVec2f(panel.getValueF("k1_bl_x"), panel.getValueF("k1_bl_y"));
    
ofVec2f k2tl = ofVec2f(panel.getValueF("k2_tl_x"), panel.getValueF("k2_tl_y"));
ofVec2f k2tr = ofVec2f(panel.getValueF("k2_tr_x"), panel.getValueF("k2_tr_y")); 
ofVec2f k2br = ofVec2f(panel.getValueF("k2_br_x"), panel.getValueF("k2_br_y"));
ofVec2f k2bl = ofVec2f(panel.getValueF("k2_bl_x"), panel.getValueF("k2_bl_y"));  
    
    
    cv::Mat k1;
    cv::Mat k2;
    copy(*_k1,k1);
    copy(*_k2,k2);
    
    cv::Mat kstitch;
    kstitch.create(k1.rows,k1.cols+k2.cols,k1.type());
    
    imitate(dst,kstitch);
        //ofLog() << "Writing img to ROI";
    if(flip) {
        
        cv::Mat leftRoi = kstitch(cv::Rect(0+(int)k2offset.x,0+(int)k2offset.y,k2.cols-(int)k2crop,k2.rows));
        cv::Mat rightRoi = kstitch(cv::Rect(k1.cols+(int)k1offset.x,0+(int)k1offset.y,k2.cols,k2.rows));
        k1.copyTo(rightRoi);
        k2.copyTo(leftRoi);
    } else {
        cv::Mat leftRoi = kstitch(cv::Rect(0+(int)k1offset.x,0+(int)k1offset.y,k2.cols+(int)k1crop,k2.rows));
        cv::Mat rightRoi = kstitch(cv::Rect(k2.cols-(int)k1crop,0,k1.cols,k1.rows));
        k1.copyTo(leftRoi);
        k2.copyTo(rightRoi);        
    }
    dst = kstitch;
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