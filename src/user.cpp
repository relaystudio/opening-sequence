#include "user.h"

Layer::Layer() {
    path = ofPolyline();
    frame.allocate(1920,720);
}

Layer::~Layer() {
    
}

void Layer::update(ofPolyline * _path) {
    path = *_path;
    center = path.getCentroid2D();
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
    path.draw();
}

void Layer::reset() {
    
}