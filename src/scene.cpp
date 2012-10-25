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
    //shader.load("shader/DOFLine.vert", "shader/DOFLine.frag");
    
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
    gl_FragColor = vec4( src, 1.0-mask);\
    }";
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_program);
    shader.linkProgram();
    
    ofLog() << "Loaded DOF";
}

void Scene::loadRefraction() {
    refract.load("shader/passthru.vert","shader/gpgpu.frag");
    ofLog() << "Loading refract";
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
    
//    if( video.getCurrentFrame()+100 == video.getTotalNumFrames())
//        fade = (video.getCurrentFrame() - (video.getTotalNumFrames() - 100))*.1;
  
    fbo[0].begin();
    //glClear(GL_COLOR_BUFFER_BIT);
//    ofClear(0,0,0,0);
    video.draw(0,0,fbo[0].getWidth(),fbo[0].getHeight());

    crowd.draw();
    //ofClearAlpha();
    fbo[0].end();
    
}


void Scene::draw(int _x, int _y) {
    ofPushMatrix();
    ofSetColor(255);
    //glEnable(GL_BLEND);
    ofTranslate(_x, _y);
    //    shader.begin();
  //      shader.setUniformTexture("maskTex",fbo[0].getTextureReference(),1);
//        fbo[0].draw(0,0);
        //        shader.end();

        fbo[0].draw(0,0);
        
        //fbo[currentFbo].draw(0,0);
        //drawRefraction();
    
//    glEnable(GL_DEPTH_TEST);
//    video.draw(0,0);
//    crowd.draw();
//    glDisable(GL_DEPTH_TEST);
   // glDisable(GL_BLEND);
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

void Scene::updateColours(vector<ofVec3f> * color) {
    crowd.updateColours(*color);
}


int Scene::lengthOfScene() {
    return video.getTotalNumFrames();
}

int Scene::getPosition() {
    return video.getCurrentFrame();
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