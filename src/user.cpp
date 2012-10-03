#include "user.h"

Layer::Layer() {
    path = ofPolyline();
    frame.allocate(1920,720);
}

Layer::~Layer() {
    
}

void Layer::updateLayer(ofPolyline * _path) {
    path = *_path;
        //center = path.getCentroid2D();
        //    direction = path.get
    area = path.getArea();
}

void Layer::loadMesh(string _path) {
    
}

void Layer::loadShader(string _path) {
    shader.load(_path); //plzplzplz don'tcrash
}

void Layer::loadVideo(string _path) {
    if(video.isLoaded()) video.close();
    video.loadMovie(_path);
    video.setLoopState(OF_LOOP_NORMAL);
    video.play();
}

void Layer::update() {
    if(video.isLoaded() || video.isPlaying()) video.update();

    frame.begin();
    video.draw(0, 0, frame.getWidth(), frame.getHeight());
    frame.end();
    
    shader.begin();
        frame.draw(0,0);
    shader.end();
}

void Layer::draw(int _x, int _y) {
    ofPopMatrix();
    ofTranslate(_x,_y);
    frame.draw(0,0);
    ofPushMatrix();
}

void Layer::draw() {
    draw(0,0);
}

void Layer::drawPath(){
        //    path.draw();
        for(int i=0; i< (int) path.size(); i++ ) {
            bool repeatNext = i == path.size() - 1;
            
            const ofPoint& cur = path.getVertices()[i];
            const ofPoint& next = repeatNext ? path.getVertices()[0] : path.getVertices()[i + 1];
            
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

void Layer::reset() {
    
}