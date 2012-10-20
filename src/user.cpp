
#include "user.h"

Crowd::Crowd() {
        //frame.allocate(1920/2,1080/2);
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

void Crowd::updateCrowd(vector<ofImage> * _frames) {
    frames.clear();
    frames = *_frames;
    //frame.allocate(1920/2,1080/2);
    // path = *_path;
    //center = path.getCentroid2D();
    //    direction = path.get
    //   area = path.getArea();
    //vector<ofImage> frames;
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

        //    frame.begin();
        //glClearColor(0.0, 0.0, 0.0, 0.0);    
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glEnable(GL_DEPTH_TEST);
//        ofClear(0,0,0,0);
//        glPopMatrix();
//        ofSetColor(255,0,0);
//        //ofScale(frame.getWidth(),frame.getHeight());
//        for(int i=0;i<layers.size();i++) {
//            tess.tessellateToMesh(layers[i],OF_POLY_WINDING_NONZERO,mesh,true);
//            ofSetColor(ceil(255*(1-(i/layers.size()))),0,0);
//                //ofLog() << "Mesh has:" << ofToString(mesh.getNumVertices()) << " verts";
//            mesh.draw();
//        }
//        glPushMatrix();
//        //ofClearAlpha(); // Using this kills alpha channel of FBO
//        glDisable(GL_DEPTH_TEST);
//        ofClearAlpha();
        // frame.end();
    
/*    shader.begin();
        frame.draw(0,0);
    shader.end();*/
    if(layers.size() > 0){
    meshes.clear();
    mesh.clear();
        //ofLog() << "Converting " << ofToString(layers.size()) << " meshes";
    for(int i=0;i<layers.size();i++) {
        tess.tessellateToMesh(layers[i],OF_POLY_WINDING_NONZERO,mesh,true);
            //ofLog() << "Mesh has:" << ofToString(mesh.getNumVertices()) << " verts";
            //ofTranslate(0,20);
        meshes.push_back(mesh);
            //mesh.draw();
    }
    }
    else if ( frames.size() > 0 ) {
        for(int i=0;i<frames.size();i++) {
        }
        
    }
}

void Crowd::draw(int _x, int _y) {
    ofEnableAlphaBlending();
    ofPushMatrix();
        ofTranslate(_x,_y);
        ofScale(1.5,2.);
        glEnable(GL_DEPTH_TEST);
        //        ofLog() << "Drawing " << ofToString(meshes.size()) << " meshes";
        for(int i=0;i < meshes.size();i++) {
                //ofLog() << "Drawing mesh " << ofToString(i);
            ofPushMatrix();
            ofSetColor(10,10,10); //i*63
            ofTranslate(0,0,i*100);
            meshes[i].drawFaces();
            ofPopMatrix();
        }
    if( frames.size() > 0 ) {
        for(int i=0;i<frames.size();i++) {
            ofLog() << "Drawing frame" << ofToString(i);
            ofPushMatrix();
            frames[i].reloadTexture();
           // frames[i].draw(0,0);
            ofPopMatrix();
        }
    }
        glDisable(GL_DEPTH_TEST);
        //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        //ofClear(0,0,0,0);

        //ofSetColor(255,0,0);
    ofPopMatrix();
}

void Crowd::draw() {
    draw(0,0);
}

vector<ofMesh> & Crowd::getMeshes() {
    return meshes;
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