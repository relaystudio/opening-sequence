#include "scene.h"

Scene::Scene() {
    bActive = false;
    for(int i=0;i<FBO_NUM;i++) {
        ofLog() << "Allocating FBO" << ofToString(i);
                fbo[i].allocate(1920,720);
    }
    crowd = *new Crowd();
    currentFbo=0;
    pong=0;
    fade = 255;
    loadDOF();
        //loadRefraction();
        //crowd.loadVideo("movie/test_002_unionjack.mov");
}

Scene::~Scene() {
    
}

void Scene::loadDOF() {
    shader.load("shader/DOFLine.vert", "shader/DOFLine.frag");
    ofLog() << "Loaded DOF";
}

void Scene::loadRefraction() {
    refract.load("shader/passthru.vert","shader/gpgpu.frag");
    ofLog() << "Loading refract";
}

void Scene::drawRefraction() {
/*    refract.begin();
    refract.setUniformTexture("buffer", crowd.getTextureReference(),1);
    refract.setUniform2f("pixel",1,1);
//    fbo[0].getTextureReference().bind();
        fbo[currentFbo].draw(0,0);
//    fbo[0].getTextureReference().unbind();
    
    refract.end();
    */
    pong = (currentFbo+1)%FBO_NUM;
    
    fbo[pong].begin();
    
    ofClear(255);
    
    refract.begin();
    
    refract.setUniformTexture( "buffer", fbo[currentFbo].getTextureReference(), 0 ); 
    refract.setUniform2f("pixel", 1/fbo[0].getWidth(),1/fbo[0].getHeight());
    video.draw(0,0,fbo[0].getWidth(),fbo[0].getHeight());
    fbo[currentFbo].draw(0,0);
    
    refract.end();
    
    crowd.draw();
    
    fbo[pong].end();
    
    currentFbo = pong;    
    
    
    fbo[currentFbo].begin();    
    // Bind the FBO we last rendered as a texture
    glEnable( GL_TEXTURE_2D );
    shader.begin();

    shader.setUniformTexture( "tex", fbo[ pong ].getTextureReference(), 0 );
    shader.setUniform2f( "pixel", 1/fbo[0].getWidth(),1/fbo[0].getHeight() );
    shader.setUniformTexture( "buffer", crowd.getTextureReference(), 0 );

    video.draw(0,0,fbo[0].getWidth(),fbo[0].getHeight());

    shader.end();
        //glDisable( GL_TEXTURE_2D );
    fbo[currentFbo].end();
    fbo[currentFbo].draw(0,0);

}

void Scene::loadVideo(string _path) {
    if(video.isLoaded()) video.close();
    video.loadMovie(_path);
    video.setLoopState(OF_LOOP_NORMAL);
    video.play();
    bActive = true;
}

void Scene::loadShader(string _path) {
     shader.load("shader/passthru.vert","shader/"+_path+".frag"); //plzplzplz don'tcrash   
}

void Scene::update() {
    if(video.isLoaded()) video.idleMovie();
    else ofLog() << "No video loaded";
    crowd.update();
    
  
  
    fbo[0].begin();
    ofClear(0);
    video.draw(0,0,fbo[0].getWidth(),fbo[0].getHeight());
   // shader.begin();
    shader.setUniform1f("aspectRatio", ofGetWidth() / ofGetHeight());
    shader.setUniform1f("lineWidth", 1);
    shader.setUniform1f("focusDistance", 200+ofGetMouseX()/2);
    shader.setUniform1f("aperture", .03);

    crowd.draw();
 //   shader.end();
    fbo[0].end();
    
}

void Scene::draw(int _x, int _y) {
    ofPushMatrix();
    ofSetColor(255);
    ofTranslate(_x, _y);
        ofSetColor(255,255,255,fade);
        fbo[0].draw(0,0);
        //fbo[currentFbo].draw(0,0);
        //drawRefraction();
    
//    glEnable(GL_DEPTH_TEST);
//    video.draw(0,0);
//    crowd.draw();
//    glDisable(GL_DEPTH_TEST);
    ofPopMatrix();
    
}

void Scene::updateCrowd(vector<ofPolyline> * _crowd) {
        //ofLog() << "Passing" << ofToString(_crowd->size()) << "shapess";
    crowd.updateCrowd(_crowd);
}

void Scene::updateCrowd(vector<ofImage> * _crowd) {
    //ofLog() << "Passing" << ofToString(_crowd->size()) << "shapess";
    crowd.updateCrowd(_crowd);
}

void Scene::drawDebug() {
    crowd.drawDebug();
}

void Scene::fadeOut() {
    fade = 0;
}

void Scene::fadeIn() {
    fade = 255;
}

bool Scene::isActive() {
    return bActive;
}

void Scene::draw() {
    draw(0,0);
}

void Scene::reset() {
    
}