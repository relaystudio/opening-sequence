#include "scene.h"

Scene::Scene() {
    bActive = false;
    for(int i=0;i<FBO_NUM;i++) {
        fbo[i].allocate(1920,1080);
    }
    crowd = *new Crowd();
    currentFbo=0;
    pong=0;
        //loadRefraction();
        //crowd.loadVideo("movie/test_002_unionjack.mov");
}

Scene::~Scene() {
    
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
    glDisable( GL_TEXTURE_2D );
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
    video.draw(0,0,fbo[0].getWidth(),fbo[0].getHeight());
    crowd.draw();
    fbo[0].end();
    
}

void Scene::draw(int _x, int _y) {
    ofSetColor(255);
    ofPopMatrix();
    ofTranslate(_x, _y);
    fbo[0].draw(0,0);
        //fbo[currentFbo].draw(0,0);
        //drawRefraction();
    ofPushMatrix();
    
    
}

void Scene::updateCrowd(vector<ofPolyline> * _crowd) {
    crowd.updateCrowd(_crowd);
}

void Scene::drawDebug() {
    crowd.drawDebug();
}

bool Scene::isActive() {
    return bActive;
}

void Scene::draw() {
    draw(0,0);
}

void Scene::reset() {
    
}