#pragma once

#include "ofMain.h"
#include "FaceTracker.h"

class FaceAugmented : public ofxDLib::Follower<ofxDLib::Face> {
protected:
    ofImage image;
    ofColor color;
    ofRectangle roi;
    ofVec2f cur, smooth;
    ofPolyline all;
    ofxDLib::Face face;
    float startedDying;
public:
    FaceAugmented(){
    };
    void setup(const ofxDLib::Face & track);
    void update(const ofxDLib::Face & track);
    void setImage(const ofPixels & pixels);
    void kill();
    void draw();
};


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    ofVideoGrabber video;
    ofxDLib::FaceTrackerFollower<FaceAugmented> tracker;
    
    
};
