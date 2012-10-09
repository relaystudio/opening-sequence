
#include "user.h"

Crowd::Crowd() {
    frame.allocate(1920/2,1080/2);
}

Crowd::~Crowd() {
    
}

void Crowd::updateCrowd(vector<ofPolyline> * _layers) {
    layers.clear();
    layers = *_layers;
        //frame.allocate(1920/2,1080/2);
        // path = *_path;
        //center = path.getCentroid2D();
        //    direction = path.get
        //   area = path.getArea();
}

void Crowd::loadMesh(string _path) {
    
}

void Crowd::loadShader(string _path) {
    shader.load(_path); //plzplzplz don'tcrash
}

void Crowd::loadVideo(string _path) {
    if(video.isLoaded()) video.close();
    video.loadMovie(_path);
    video.setLoopState(OF_LOOP_NORMAL);
    video.play();
}

void Crowd::update() {
    if(video.isLoaded() || video.isPlaying()) video.update();

    frame.begin();
        //glClearColor(0.0, 0.0, 0.0, 0.0);    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        ofClear(0,0,0,0);
        glPopMatrix();
        ofSetColor(255,0,0);
        //ofScale(frame.getWidth(),frame.getHeight());
        for(int i=0;i<layers.size();i++) {
            tess.tessellateToMesh(layers[i],OF_POLY_WINDING_NONZERO,mesh,true);
            ofSetColor(ceil(255*(1-(i/layers.size()))),0,0);
                //ofLog() << "Mesh has:" << ofToString(mesh.getNumVertices()) << " verts";
            mesh.draw();
        }
        glPushMatrix();
        //ofClearAlpha(); // Using this kills alpha channel of FBO
        glDisable(GL_DEPTH_TEST);
        ofClearAlpha();
    frame.end();
    
/*    shader.begin();
        frame.draw(0,0);
    shader.end();*/
}

void Crowd::draw(int _x, int _y) {
    ofPopMatrix();
        ofTranslate(_x,_y);
        frame.draw(0,0);
    ofPushMatrix();
}

void Crowd::draw() {
    draw(0,0);
}

ofTexture & Crowd::getTextureReference() {
    return frame.getTextureReference();
}

void Crowd::drawDebug() {
    for(int i=0;i<layers.size();i++) {
        ofSetColor(1-(i/layers.size()));
        drawPath(&layers[i]);
    }
}

void Crowd::drawPath(ofPolyline * path){
        for(int i=0; i< (int) path->size(); i++ ) {
            bool repeatNext = i == path->size() - 1;
            
            const ofPoint& cur = path->getVertices()[i];
            const ofPoint& next = repeatNext ? path->getVertices()[0] : path->getVertices()[i + 1];
            
            float angle = atan2f(next.y - cur.y, next.x - cur.x) * RAD_TO_DEG;
            float distance = cur.squareDistance(next);
            
            if(repeatNext) {
                ofSetColor(255, 0, 255);
            }
            glPushMatrix();
            glTranslatef(cur.x, cur.y, 0);
            ofRotate(angle);
            ofLine(0, 0, 0, distance);
            ofLine(0, 0, distance, 0);
            glPopMatrix();
        }
}

void Crowd::reset() {
    
}