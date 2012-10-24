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
    setupPanel();
    setupCamera();
    setupType();
    
    string shader_program = "#version 120\n \
    #extension GL_ARB_texture_rectangle: enable\n \
    \
    uniform sampler2DRect tex0;\
    uniform sampler2DRect maskTex;\
    \
    void main(void) {\
    vec2 pos = gl_TexCoord[0].st;\
    vec3 src = texture2DRect(tex0,pos).rgb;\
    float mask = texture2DRect(maskTex,pos).r;\
    gl_FragColor = vec4( src, 1-mask);\
    }";
    
    
    transShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_program);
    transShader.linkProgram();
    
    
    
    transition = false;
    activeScene = 0;
    newScene = 1;
    
    scene[activeScene] = *new Scene();

    scene[activeScene].loadVideo("movie/Camo_01_pjpeg.mov");
                                                           //activeScene.loadShader("refraction"); // Test
    
    makeNewScene();
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
    updateTransition();
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
    ofxCv::threshold(lowerThresh,lowerBound*255,false);
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
//    curStitched = cv::Mat::zeros(curThresh.size(), curThresh.type());
    curStitched = toCv(stitched).clone();
        //copy(curStitched,curThresh);
    imitate(curThresh, curStitched);
    
    //background.update(curStitched, curThresh);
    //absdiff(curStitched, bgSub, curStitched);
    clamped.clear();
 //   blur(curStitched, panel.getValueI("cvBlur"));
    
    for(int i=0;i<4;i++) {
            //        imitate(curThresh,curStitched);
         curThresh = cv::Mat::ones(curThresh.size(), curThresh.type());
         cvClamp(curStitched,
                 curThresh, 
                 _thresh->at(i).min, 
                 _thresh->at(i).max); // Output curThresh

        if(i==2 || i== 3)
            blur(curThresh, abs(panel.getValueI("cvBlur")-10));
        else if(i > 3)
            blur(curThresh, panel.getValueI("cvBlur")+10);
        else
            blur(curThresh, panel.getValueI("cvBlur"));
           //
        ofxCv::toOf(curThresh, clamp[0]);
        clamp[0].reloadTexture();
        clamped.push_back(clamp[0]);
        /*
        if(debug) {
            ofxCv::toOf(curThresh, clamp[0]);
            clamp[i].reloadTexture();
        }
            //resize(stitched, fullSized);
        contourFinder.findContours(curThresh);
            //contourFinder.findContours(curThresh);
        ofPolyline theShape = getContour(&contourFinder);
        contours.push_back(theShape);*/
    }
    scene[activeScene].updateCrowd(&clamped);
    if(transActive)
        scene[newScene].updateCrowd(&clamped);
        //ofLog() << "There are " << ofToString(contours.size()) << "Contours";
    //activeScene.updateCrowd(&contours);

}

ofPolyline testApp::getContour(ofxCv::ContourFinder * _contourFinder) {
    ofPolyline poly;
    
    if(_contourFinder->size() != 0 ) {
        vector<ofPolyline> polylines;
        polylines = _contourFinder->getPolylines();
        for(int i=0; i<polylines.size(); i++) {
            if(i==0) poly = polylines[i];
           // if(polylines[i].getArea() > 20)
                if(polylines[i].size() >= poly.size())
                    poly = polylines[i];
               // poly.addVertices(polylines[i].getVertices());
        //    }
            
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
    //ofBackground(70, 70, 70);
    ofBackground(0);
        //glEnable(GL_DEPTH_TEST);
    ofPushMatrix();
    ofScale(scaleFactor,scaleFactor);
    //ofSetColor(255,255,255,255);

    
    glBlendFunc(GL_ONE, GL_ONE);
    if(transActive) {
        ofPushMatrix();
            ofSetColor(255);
            //glBlendFunc(GL_DST_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
          // transShader.begin();
                //ofClear(0,0,0,0);;
              //  transShader.setUniformTexture("maskTex",trans.getTextureReference(),1);
        //glBlendEquation(GL_FUNC_ADD);
       // glBlendFunc(GL_SRC_COLOR,GL_ONE);
      
        
        //        Alpha Layer
        glDisable(GL_BLEND);
        glColorMask(1,1,1,1);
        glColor4f(1.,1.,1.,1.);
        trans.draw(0,0);
        glColorMask(1,1,1,0);
        glEnable(GL_BLEND);
        
        glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR);
        scene[newScene].draw();
        
        glBlendFunc(GL_ONE, GL_DST_ALPHA);
        scene[activeScene].draw();
        
//        glDisable(GL_BLEND);
//        trans.draw(0,0);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_DST_ALPHA, GL_DST_ALPHA);
//        scene[activeScene].draw();
//        

//            transShader.printActiveAttributes();
//            transShader.printActiveUniforms();
           // transShader.end();
       // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
   /*    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glColorMask(1,1,1,0.0f);
        trans.draw(0,0);
        glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA);
        scene[activeScene].draw();
      //glBlendFuncSeparate(GL_SRC_ALPHA, GL_DST_COLOR, GL_ONE, GL_ONE);
        glColorMask(1,1,1,1.0f);
        glColor4f(1,1,1,1.0f);
        scene[newScene].draw(); */

        
        
        
        
        
        
//            glBlendFunc( GL_SRC_ALPHA, GL_DST_COLOR);
//            glColor4f(1,1,1,1.0f);
        
           // scene[newScene].draw();
        
        ofPopMatrix();
    }
    else {
        scene[activeScene].draw(0,0);
    }
    glDisable(GL_BLEND);

    ofPopMatrix();
    
    
    if(debug) {
        ofPushMatrix();
        stitched.draw(0,0);
        scene[activeScene].drawDebug();
        
        ofTranslate(ofGetWidth()-200,0);
        ofSetColor(255);
        for(int i=0;i<4;i++) {
            clamped[i].draw(0,i*100,200,100);
        }
        trans.draw(0,400,200,100);
        ofTranslate(0,500);
        ofScale(.1,.1);
        scene[activeScene].draw();
        ofTranslate(0,720);
        scene[newScene].draw();
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
    
    if(panel.getValueB("transition")) {
        transitionScene();
        panel.setValueB("transition",false);
    }
    
}

void testApp::setupPanel() {
        //Setup panel
    panelWidth = 200;
    panel.setup(panelWidth, 800);
    
    panel.addPanel("Setup");
    panel.addLabel("Debug switches");
    panel.addToggle("debug",true);
    panel.addToggle("transition", false);
    panel.addToggle("horiz_flip", false);
    panel.addToggle("vert_flip", false);
    panel.addToggle("flipKinect", false);
    panel.addToggle("mirrorKinect", false);
    panel.addToggle("subtractBG", false);
    panel.addSlider("cvBlur",40,0,80,true);
    
    panel.addLabel("Main Window");
    panel.addSlider("scaleFactor", .7, 0.005, 1.0, false);
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
    
    if( panel.getValueB("subtractBG")) {
        imitate(bgSub,stitched);
        bgSub = cv::Mat::ones(bgSub.size(), bgSub.type());
        bgSub = toCv(stitched).clone();
        panel.setValueB("subtractBG",false);
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
    
    background.setLearningRate(900);
    background.setThresholdValue(10);
    
    angle = 0;
    kinect.setRegistration(false);
    ofLog() << "Starting first kinect";
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    kinect.setCameraTiltAngle(angle);
    kinectPtr = &kinect;
    imitate(depthImg, kinect);
    
    //Second 
    kinect2.setRegistration(false);
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
        calibration2.undistort(toCv(depthImg2));
        
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
//        ofLog() << "Making the gradient";
//
//        
//        ofLog() << "Copying to grad ROI";
//
//       
//        ofLog() << "Cloning gradient to CV mat";
        if(kstitched.cols != gradientMat.cols
           || kstitched.rows != gradientMat.rows) {
            imitate(distGradient,kstitched);
            imitate(gradientMat, kstitched);
            createGradient(&distGradient,
                           panel.getValueF("grad_low"),
                           panel.getValueF("grad_mid"),
                           panel.getValueF("grad_high"));
//            ofLog() << "Copying to gradient mat";
            gradientMat = toCv(distGradient);
        }
//        ofLog() << "Adding gradientMat to stitched";
    //    kstitched += gradientMat;
        
        
        
        
            //        gradientMat = toCv(distGradient).clone();     
            // ofxCv::resize(gradientMat, resizedGrad); 
            //  ofLog() << "Done grad";
            //        ofLog() << "About to add grad";
            //        cvAnd( &toCorrect , &gradientMat, &stitchedMat );

            //        cv::add(kstitched, gradientMat, kstitched);
            //        toCorrect += gradientMat;
            //        ofxCv::resize(toCorrect, stitchedMat, INTER_NEAREST);
            //        ofxCv::toOf(stitchedMat, stitched);
        //ofLog() << "Imitate the stitched file";
        imitate(stitched, kstitched);
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
        
    
    //cv::Mat k1, k2;
    cv::Mat k1orig, k2orig;
//    copy(*_k1,k1);
//    copy(*_k2,k2);
       copy(*_k1,k1orig);
       copy(*_k2,k2orig);
    int crop = 100;
    
    
//    cv::Mat k1, k2;
//    k1.create(k1orig.rows-(crop*2), k1orig.cols-(crop*2), k1orig.type());
//    imitate(k2,k1);
//ofLog() << "Making ROIs";
    cv::Mat k1roi(k1orig, cv::Rect(
                             50,
                             50,
                             k1orig.cols-crop,
                             k1orig.rows-crop));

    cv::Mat k2roi(k2orig, cv::Rect(
                             50,
                             100,
                             k2orig.cols-crop,
                             k2orig.rows-crop));
   // ofLog() << "Clone ROIs to k1/k2";
    cv::Mat k1 = k1roi.clone();
    cv::Mat k2 = k2roi.clone();
    
//   k1roi.copyTo(k1);
//   k2roi.copyTo(k2);
     //   ofLog() << "Make stitched image";
    cv::Mat kstitch;
    kstitch.create(k1.rows,k1.cols+k2.cols,k1.type());
    
//       ofLog() << "Flip Image";
    if(flip) {
        
        cv::Mat leftRoi = kstitch(cv::Rect(0,0,k2.cols,k2.rows));
        cv::Mat rightRoi = kstitch(cv::Rect(k2.cols,0,k1.cols,k1.rows));
        k1.copyTo(rightRoi);
        k2.copyTo(leftRoi);
    } else {
        cv::Mat leftRoi = kstitch(cv::Rect(0,0,k2.cols,k2.rows));
        cv::Mat rightRoi = kstitch(cv::Rect(k2.cols,0,k1.cols,k1.rows));
        k1.copyTo(leftRoi);
        k2.copyTo(rightRoi);        
    }
        //ofLog() << "Copy image to destination";
    ofxCv::copy(kstitch, dst);
    
//    dst = kstitch;
}

void testApp::setDebug(bool _debug) {
    debug = _debug;
}

/******************************************
 Scene management functions 
 *******************************************/

void testApp::updateActiveScene() {
    scene[activeScene].update();
}

void testApp::transitionScene() {
    
    if(trans.isLoaded())
        trans.setFrame(0);
    trans.loadMovie("trans/transition_001.mov");
    trans.setLoopState(OF_LOOP_NONE);
    trans.setFrame(0);
    trans.play();
///    activeScene.fadeOut();
//    activeScene.fadeIn();
    makeNewScene();
    transActive = true;
    ofLog() << "Transitioned scene";
}

void testApp::updateTransition() {
    if(transActive) {
        
        trans.idleMovie();
        scene[newScene].update();
        //scene[activeScene].update();
        
        if(trans.getCurrentFrame() < trans.getTotalNumFrames()) {
            ofLog() << "Transitioning "
            << ofToString(trans.getCurrentFrame())
            << " of " << ofToString(trans.getTotalNumFrames());
        } else {
            transActive = false;
            if(activeScene == 1) {
                newScene = 1; activeScene = 0;
            } else {
                newScene = 0; activeScene = 1;
            }
            ofLog() << "Finished transition";
        }
    }
}

void testApp::makeNewScene() {
    int random = ofRandom(0,7);    
    vector<ofColor> colours;
    colours.clear();
    ofColor c1;
    ofColor c2;
    ofColor c3;
    ofColor c4;
    
    
    switch(random) {
        case 0: // Black Fire
            c1.set(255);
            c2.set(255);
            c3.set(255);
            c4.set(255);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_002_unionjack.mov");
            break;
        case 1: // Circles 2
            c1.set(0,0,0);
            c2.set(0,0,0);
            c3.set(0,0,0);
            c4.set(0,0,0);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/Glitch_01.mov");
            break;
        case 2: // Floating Suits
            c1.set(0,0,0);
            c2.set(0,0,0);
            c3.set(0,0,0);
            c4.set(0,0,0);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/Smoke_01.mov");
            break;
        case 3: // Glitch 01
            c1.set(255);
            c2.set(255);
            c3.set(255);
            c4.set(255);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/Smoke_02.mov");
            break;
        case 4: // Smokte 02
            c1.set(255);
            c2.set(255);
            c3.set(255);
            c4.set(255);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/Camo_01_pjpeg.mov");
            break;
        case 5: // Union Jack 01
            c1.set(0,0,0);
            c2.set(248,54,55);
            c3.set(3,0,93);
            c4.set(255,255,255);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_003_ink.mov");
            break;
            
        case 6: // Big Bank
            c1.set(32,1,1);
            c2.set(32,1,1);
            c3.set(32,1,1);
            c4.set(32,1,1);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_002_unionjack.mov");
            break;

        case 7: // Smoke 01 Gold
            c1.set(0),0,0;
            c2.set(36,7,6);
            c3.set(228,158,0);
            c4.set(0,0,0);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_003_ink.mov");
            break;
            
        case 8: // Glitch 02
            c1.set(0,0,0);
            c2.set(0,0,0);
            c3.set(0,0,0);
            c4.set(0,0,0);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_002_unionjack.mov");
            break;
            
        case 9: // Camo
            c1.set(189,185,167);
            c2.set(17,16,33);
            c3.set(91,72,62);
            c4.set(54,107,85);
            
            scene[newScene] = *new Scene();
            scene[newScene].loadVideo("movie/test_002_unionjack.mov");
            break;
            
            
    }
    
    colours.push_back(c1);
    colours.push_back(c2);
    colours.push_back(c3);
    colours.push_back(c4);
    scene[newScene].updateColours(&colours);
    
}
