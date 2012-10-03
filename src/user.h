#ifndef _user_h
#define _user_h

#include "ofMain.h"

class Layer {
    
public:
    
    Layer();
    ~Layer();
    
    void update(ofPolyline * _path);

    void loadMesh(string _path);
    void loadShader(string _path);
    void loadVideo(string _path);
    
    void update();
    void draw(int _x, int _y);
    void draw();
    void drawPath();
    void reset();
    
private:
    ofPolyline path;
    ofVec3f center;
    ofVec3f direction;
    float area; // Area computed from Polyline
    long life; // How long active
    
    bool bActive; // Active tracking
    bool active; // Facing object
    
    ofFbo frame;
    
    ofVideoPlayer video;
    ofShader shader;
    ofMesh mesh;
};
#endif
