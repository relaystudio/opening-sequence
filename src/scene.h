#ifndef _scene_h
#define _scene_h

#include "ofMain.h"
#include "user.h"

#define FBO_NUM 1

class Scene {

public:
    
    Scene();
    ~Scene();
    
    void loadVideo(string _path);
    void loadShader(string _path); 
    void loadRefraction();
    void drawRefraction();
    void loadDOF();
    int currentFbo;


    bool isActive();
    
    int lengthOfScene();
    int getPosition();
    
    void update();
    void fadeIn();
    void fadeOut();
    
    void draw(int _x, int _y);
    void draw();
    
    void drawDebug();
    void reset();
    
    void updateCrowd(vector<ofPolyline> * _layers);
    void updateCrowd(vector<ofImage> * _crowd);
    void updateColours(vector<ofVec3f> * color);

    
private:
    bool bActiveShader;
    bool bActive;
    Crowd crowd;
    int pong;
    float fade;
    ofShader shader;
    ofShader refract;
    ofFbo fbo[FBO_NUM];
    vector<ofVec2f> shaderVar;
    ofVideoPlayer video;
};
#endif
