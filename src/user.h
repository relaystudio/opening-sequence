#ifndef _user_h
#define _user_h

#include "ofMain.h"

// Render out 1.,0,0 RGB for blank mask

class Crowd {
    
public:
    
    Crowd();
    ~Crowd();

    void loadMesh(string _path);
    void loadShader(string _path);
    void loadVideo(string _path);
    
    void updateCrowd(vector<ofPolyline> * _layers);
    void updateCrowd(vector<ofImage> * _frames);
    void update();
    void draw(int _x, int _y);
    void draw();
    void drawDebug();
    void drawPath(ofPolyline * path);
    void reset();
    vector<ofMesh> & getMeshes();
    ofTexture & getTextureReference();
    
private:
    vector<ofPolyline> layers;
    vector<ofMesh> meshes;
    vector<ofImage> frames;
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
    ofTessellator tess;
};
#endif
