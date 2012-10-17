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
    cv::Mat upperThresh = toCv(src).clone();
    cv::Mat lowerThresh = toCv(src).clone();

    imitate(dst,src);

    ofxCv::threshold(upperThresh,upperBound*255,true);
    ofxCv::threshold(lowerThresh,lowerBound*255,true);
    dst = upperThresh + lowerThresh;
        //    ofLog() << "Ending cvClamp";
}

void testApp::getScene( cv::Mat * _frame, vector<Range> * _thresh) {
    //0 = partial foreground
    //1 = primary focus
    //2 = background
    //3 = far background
    contours.clear();
    blur(stitched, 20);
    cv::Mat curThresh;
    cv::Mat curStitched;
    
    curStitched = toCv(stitched);
    imitate(curThresh,curStitched);
    
    for(int i=0;i<4;i++) {

         cvClamp(curStitched, 
                 curThresh, 
                 _thresh->at(i).min, 
                 _thresh->at(i).max); // Output curThresh
        if(debug) {
            blur(curThresh, 40);
            ofxCv::toOf(curThresh, clamp[i]);
            clamp[i].reloadTexture();
        }
        
        
            //        resize(stitched, fullSized);
        contourFinder.findContours(curThresh);
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
    panel.addToggle("changeGradient", false);
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
    if(sanityTest.isInitialized()) sanityTest.update(); // Just in case
    kinect.update(); 
    kinect2.update(); // Update both kinects
    

                                
    
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
        try {
            stitchKinect(&depthImg,&depthImg2);//.readToPixels(stitched);
        } catch (int fuck) {
            ofLog() << ofToString(fuck);
        }
        stitched.update();
        cv::Mat toCorrect; imitate(toCorrect,stitched); 
        toCorrect = toCv(stitched);
        
        cv::Mat gradientMat; imitate(gradientMat,distGradient);
        gradientMat = toCv(distGradient);
        
        cv:Mat stitchedMat; imitate(stitchedMat,stitched);
            //        ofLog() << "About to add grad";
            //        cvAnd( &toCorrect , &gradientMat, &stitchedMat );
        stitchedMat = toCorrect + gradientMat;
            //        ofLog() << "Done grad";
        ofxCv::toOf(stitchedMat, stitched);

        stitched.update();
    }
}

void testApp::drawSkew(ofImage * src, ofPoint * tl, ofPoint * tr, ofPoint * br, ofPoint * bl) {
    // tl,tr,br,bl = normalized 0.-1. 
    ofSetColor(255);
    ofTexture   & tex = src->getTextureReference(); 
        //    src->reloadTexture();
        //src->draw(0,0);
        //ofDisableArbTex();
        //    tex.bind();
   /* glEnable(GL_BLEND);  
        //    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_TEXTURE); //Set to Texture
        glPushMatrix(); 
            ofScale(src->getWidth(), src->getHeight(),0.0f);  
            glMatrixMode(GL_MODELVIEW);  
                    glBegin(GL_QUADS);  
                        glNormal3f(0.0f,0.0f,1.0f);
                        
                        //Top Left
                        glTexCoord3f(0.0f, 0.0f, 0.0f); 
                        glVertex3f( tl->x, tl->y, 0.0f);    
                        
                        // Top Right
                        glTexCoord3f(1.0f, 0.0f, 0.0f); 
                        glVertex3f( tr->x, tr->y, 0.0f);    

                        // Bottom Right
                        glTexCoord3f(1.0f, 1.0f, 0.0f); 
                        glVertex3f( br->x,br->y, 0.0f);    

                        // Bottom Left
                        glTexCoord3f(0.0f, 1.0f, 0.0f); 
                        glVertex3f( bl->x, bl->y, 0.0f);       

                    glEnd();  
            glMatrixMode(GL_TEXTURE);  
        glPopMatrix();  
    glMatrixMode(GL_MODELVIEW); // Reset to model view
        //    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
        //tex.unbind();
        //ofEnableArbTex();*/
    
    glActiveTexture(GL_TEXTURE0);
    
    glEnable(tex.getTextureData().textureTarget);
    
    glBindTexture( tex.getTextureData().textureTarget, (GLuint)tex.getTextureData().textureID );
    
    GLfloat offsetw = 0.0f;
    GLfloat offseth = 0.0f;
    
    if (tex.getTextureData().textureTarget == GL_TEXTURE_2D) {
        offsetw = 1.0f / (tex.getTextureData().tex_w);
        offseth = 1.0f / (tex.getTextureData().tex_h);
    }
        // -------------------------------------------------
    
    GLfloat tx0 = 0+offsetw;
    GLfloat ty0 = 0+offseth;
    GLfloat tx1 = tex.getTextureData().tex_t - offsetw;
    GLfloat ty1 = tex.getTextureData().tex_u - offseth;
    
    glPushMatrix(); 
    GLfloat tex_coords[] = {
        tx0,ty0,
        tx1,ty0,
        tx1,ty1,
        tx0,ty1
    };
    
    int w = 640;
    int h = 480;
    
    GLfloat verts[] = {
        tl->x*w, tl->y*h,
        tr->x*w, tr->y*h,
        br->x*w, br->y*h,
        bl->x*w, bl->y*h
    };
    
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords );
    glEnableClientState(GL_VERTEX_ARRAY);		
    glVertexPointer(2, GL_FLOAT, 0, verts );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    
    glPopMatrix();
    glDisable(tex.getTextureData().textureTarget);
    
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
    
    ofPoint k1tl = (panel.getValueF("k1_tl_x"), panel.getValueF("k1_tl_y"));
    ofPoint k1tr = (panel.getValueF("k1_tr_x"), panel.getValueF("k1_tr_y"));
    ofPoint k1br = (panel.getValueF("k1_br_x"), panel.getValueF("k1_br_y"));
    ofPoint k1bl = (panel.getValueF("k1_bl_x"), panel.getValueF("k1_bl_y"));
    
    ofPoint k2tl = (panel.getValueF("k2_tl_x"), panel.getValueF("k2_tl_y"));
    ofPoint k2tr = (panel.getValueF("k2_tr_x"), panel.getValueF("k2_tr_y"));
    ofPoint k2br = (panel.getValueF("k2_br_x"), panel.getValueF("k2_br_y"));
    ofPoint k2bl = (panel.getValueF("k2_bl_x"), panel.getValueF("k2_bl_y"));    
    
    
    if(mirror) {
        _k1->mirror(0,1);
        _k2->mirror(0,1);
    }
    
    stitchedImage.begin();
        ofClear(0);
    ofPushMatrix();
        ofSetColor(255); 
            ofTranslate(k1offset);

 /*   GLfloat myMatrix[16];
    
        //we set it to the default - 0 translation
        //and 1.0 scale for x y z and w
    for(int i = 0; i < 16; i++){
        if(i % 5 != 0) myMatrix[i] = 0.0;
        else myMatrix[i] = 1.0;
    }
    
        //we need our points as opencv points
        //be nice to do this without opencv?
    CvPoint2D32f cvsrc[4];
    CvPoint2D32f cvdst[4];	
    
        //we set the warp coordinates
        //source coordinates as the dimensions of our window
    cvsrc[0].x = 0;
    cvsrc[0].y = 0;
    cvsrc[1].x = stitchedImage.getWidth();
    cvsrc[1].y = 0;
    cvsrc[2].x = stitchedImage.getWidth();
    cvsrc[2].y = stitchedImage.getHeight();
    cvsrc[3].x = 0;
    cvsrc[3].y = stitchedImage.getHeight();			
    
    cvdst[0].x = k1tl.x * (float)stitchedImage.getWidth();
    cvdst[0].y = k1tl.y * (float)stitchedImage.getHeight();
    
    cvdst[1].x = k1tr.x * (float)stitchedImage.getWidth();
    cvdst[1].y = k1tr.y * (float)stitchedImage.getHeight();
    
    cvdst[2].x = k1br.x * (float)stitchedImage.getWidth();
    cvdst[2].y = k1br.y * (float)stitchedImage.getHeight();
    
    cvdst[3].x = k1bl.x * (float)stitchedImage.getWidth();
    cvdst[3].y = k1bl.y * (float)stitchedImage.getHeight();
    
    
    CvMat * translate = cvCreateMat(3,3,CV_32FC1);

    CvMat* src_mat = cvCreateMat( 4, 2, CV_32FC1 );
    CvMat* dst_mat = cvCreateMat( 4, 2, CV_32FC1 );
    
        //copy our points into the matrixes
    cvSetData( src_mat, cvsrc, sizeof(CvPoint2D32f));
    cvSetData( dst_mat, cvdst, sizeof(CvPoint2D32f));

    cvFindHomography(src_mat, dst_mat, translate);
    

    float *matrix = translate->data.fl;

    
    myMatrix[0]		= matrix[0];
    myMatrix[4]		= matrix[1];
    myMatrix[12]	= matrix[2];
    
    myMatrix[1]		= matrix[3];
    myMatrix[5]		= matrix[4];
    myMatrix[13]	= matrix[5];	
    
    myMatrix[3]		= matrix[6];
    myMatrix[7]		= matrix[7];
    myMatrix[15]	= matrix[8];	
    
    for(int i=0;i<15;i++) {
    ofLog() << "Matrix "<< ofToString(i) << ": " << ofToString(myMatrix[i]);        
    }
*/
        //finally lets multiply our matrix
        //wooooo hoooo!
        glPushMatrix();
        //glMultMatrixf(myMatrix);
          if(flip) _k1->drawSubsection(60, 40, _k1->getWidth()-60, _k1->getHeight()-40, 0, 0);
          else _k2->drawSubsection(60, 40, _k2->getWidth()-60, _k2->getHeight()-40, 0, 0);
        glPopMatrix();
    

            ofTranslate(k2offset);
        //tex2.bind();
//            if(flip) drawSkew(_k2,&k2tl,&k2tr,&k2br,&k2bl);
//            else     drawSkew(_k1,&k1tl,&k1tr,&k1br,&k1bl);
//    
//            if(flip) _k2->getTextureReference().draw(k2tl,k2tr,k2br,k2bl);
//            else     _k1->getTextureReference().draw(k1tl,k1tr,k1br,k1bl);
        //tex2.unbind();
        //   if(flip) _k2->draw(0,0);
        //  else _k1->draw(0,0);
  /*  for(int i = 0; i < 16; i++){
        if(i % 5 != 0) myMatrix[i] = 0.0;
        else myMatrix[i] = 1.0;
    }
                                                                    
    
    cvdst[0].x = k2tl.x * (float)stitchedImage.getWidth();
    cvdst[0].y = k2tl.y * (float)stitchedImage.getHeight();
    
    cvdst[1].x = k2tr.x * (float)stitchedImage.getWidth();
    cvdst[1].y = k2tr.y * (float)stitchedImage.getHeight();
    
    cvdst[2].x = k2br.x * (float)stitchedImage.getWidth();
    cvdst[2].y = k2br.y * (float)stitchedImage.getHeight();
    
    cvdst[3].x = k2bl.x * (float)stitchedImage.getWidth();
    cvdst[3].y = k2bl.y * (float)stitchedImage.getHeight();
    
    cvSetData( src_mat, cvsrc, sizeof(CvPoint2D32f));
    cvSetData( dst_mat, cvdst, sizeof(CvPoint2D32f));
    cvFindHomography(src_mat, dst_mat, translate);
    matrix = translate->data.fl;
    myMatrix[0]		= matrix[0];
    myMatrix[4]		= matrix[1];
    myMatrix[12]	= matrix[2];
    
    myMatrix[1]		= matrix[3];
    myMatrix[5]		= matrix[4];
    myMatrix[13]	= matrix[5];	
    
    myMatrix[3]		= matrix[6];
    myMatrix[7]		= matrix[7];
    myMatrix[15]	= matrix[8];	
    ofPushMatrix();
        glMultMatrixf(myMatrix);*/
    
    if(flip) _k2->drawSubsection(60, 40, _k2->getWidth()-60, _k2->getHeight()-40, 0, 0);
    else _k1->drawSubsection(60, 40, _k1->getWidth()-60, _k1->getHeight()-40, 0, 0);
    // ofPopMatrix();
    
    
    
    ofPopMatrix();
    stitchedImage.end();
    
    stitchedImage.getTextureReference().readToPixels(stitched);
    
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