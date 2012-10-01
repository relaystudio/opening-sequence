#ifndef _user_h
#define _user_h

#include "ofMain.h"

class Layer {
    
public:
    
    Layer();
    ~Layer();
    
    void update(ofPolyline * _path);
    void update();
    void draw(int _x, int _y);
    void draw();
    void reset();
    
private:
    ofPolyline path;
    ofVec3f center;
    ofVec3f direction;
    float area; // Area computed from Polyline
    long life; // How long active
    
    bool bActive; // Active tracking
    bool active; // Facing object
};
#endif
