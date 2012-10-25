
#include "user.h"

Crowd::Crowd() {
        //frame.allocate(1920/2,1080/2);
    //shader.load("shader/water");
    string shader_program = "#version 120\n \
    #extension GL_ARB_texture_rectangle: enable\n \
    \
    uniform sampler2DRect tex0;\
    uniform sampler2DRect maskTex;\
    uniform vec3 colour;\
    \
    void main(void) {\
    vec2 pos = gl_TexCoord[0].st;\
    vec3 src = texture2DRect(tex0,pos).rgb;\
    float mask = texture2DRect(maskTex,pos).r;\
    \
    gl_FragColor = vec4( colour.rgb, mask);\
    }";
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_program);
    shader.linkProgram();
    
    ofVec3f c1, c2, c3, c4;

    c1.set(189,185,167);
    c2.set(17,16,33);
    c3.set(91,72,62);
    c4.set(54,107,85);
    colour.push_back(c1);
    colour.push_back(c2);
    colour.push_back(c3);
    colour.push_back(c4);
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
        ofTranslate(_x,_y-100);
        ofScale(2,2.4);
        //glEnable(GL_DEPTH_TEST);
        //        ofLog() << "Drawing " << ofToString(meshes.size()) << " meshes";
//        for(int i=0;i < meshes.size();i++) {
//                //ofLog() << "Drawing mesh " << ofToString(i);
//            ofPushMatrix();
//            ofSetColor(10,10,10); //i*63
//            ofTranslate(0,0,i*100);
//            meshes[i].drawFaces();
//            ofPopMatrix();
//        }
    if( frames.size() > 0 ) {
        for(int i=0;i<frames.size();i++) {
            //ofLog() << "Drawing frame" << ofToString(i);
            ofPushMatrix();
            //shader.begin();
            frames[i].reloadTexture();
            //shader.setUniformTexture("tex", frames[i].getTextureReference(),0);
/*            glPushMatrix();
            glDisable(GL_BLEND);
            glColorMask(1,0,0,1);
            glColor4f(1,1,1,1.0f);
            //ofSetHexColor(0xff0000);
            frames[i].draw(0,0);
            glColorMask(1,1,1,1);
            glEnable(GL_BLEND);
//            glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_CONSTANT_COLOR_EXT);
            glBlendFunc( GL_SRC_ALPHA, GL_DST_COLOR);
//            ofSetHexColor(0xffffff);
            glColor4f(1,1,1,1.0f);
            frames[i].draw(0,0);
            glPopMatrix();
            glDisable(GL_BLEND);
            //shader.end();


                
 */
//            glDisable(GL_BLEND);
//            glColor4f((float)(colour[i].r/255), (float)(colour[i].g/255), (float)(colour[i].b/255),.4f);
//            ofRect(0,0,frames[i].getWidth(),frames[i].getHeight());
            glEnable(GL_BLEND);
            
            shader.begin();
            shader.setUniformTexture("maskTex",frames[i].getTextureReference(),1);
            shader.setUniform3f("colour", (colour[i].x/255), (colour[i].y/255), (colour[i].z/255));
            //glColor4f(colour[i].r/255, colour[i].g/255,colour[i].b/255,1.0f);
  //          ofEnableAlphaBlending();
            //glColorMask(1,1,1,1);
           /* ofLog() << "Clr" << ofToString((colour[i].x))
            << ofToString((colour[i].y))
            << ofToString((colour[i].z)); */
      //     ofSetColor(colour[i]);
            //glBlendFunc(GL_ONE, GL_DST_COLOR);
          //  ofSetColor(colour[i].x, colour[i].y, colour[i].z,0.);

            frames[i].draw(0,0);
           // ofRect(0,0,frames[i].getWidth(),frames[i].getHeight());
        //    ofSetColor(255);
           glColorMask(1,1,1,0);
           glColor4f(1,1,1,0.0f);
//            ofSetColor(colour[i], 0);
           // ofRect(0,0, frames[i].getWidth(), frames[i].getHeight());
//            ofDisableAlphaBlending();
//            ofRect(0,0,frames[i].getWidth(),frames[i].getHeight());
           // ofRect(0,0, frames[i].getWidth(), frames[i].getHeight());
            shader.end();
            glDisable(GL_BLEND);
            ofPopMatrix();
        }

        
    
        
    }
       // glDisable(GL_DEPTH_TEST);
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

void Crowd::updateColours( vector<ofVec3f> _colours) {
    //ofLog() << "Reseting color";
    colour.clear();
    colour = _colours;
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