#ifndef _scene_h
#define _scene_h

#include "ofMain.h"
#include "user.h"

#define FBO_NUM 2

class Scene {

public:
    
    Scene();
    ~Scene();
    
    void load(string _path); // Video
    
    ofShader shader;
    ofFbo fbo[FBO_NUM];
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
