#include "user.h"

Layer::Layer() {
    path = ofPolyline();
}

Layer::~Layer() {
    
}

void Layer::update(ofPolyline * _path) {
    path = *_path;
    center = path.getCentroid2D();
        //    direction = path.get
    area = path.getArea();
}

void Layer::update() {
    
}

void Layer::draw(int _x, int _y) {
    
}

void Layer::draw() {
    
}

void Layer::reset() {
    
}