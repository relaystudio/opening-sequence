#ifndef _scene_h
#define _scene_h

#include "ofMain.h"
#include "user.h"

class Scene {
    
public:
    
    Scene();
    ~Scene();
    
    ofShader shader;
    ofFbo fbo[3];
    vector<Layer*> layers;
    
    bool bActive;
    float fade;
    
    void update();
    void draw(int _x, int _y);
    void draw();
    void reset();
    
private:

};
#endif
