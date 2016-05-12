#include "ofApp.h"

using namespace ofxDLib;

const float dyingTime = 1;

void FaceAugmented::setup(const Face & track) {
    color.setHsb(ofRandom(0, 255), 255, 255);
    cur = track.rect.getCenter();
    smooth = cur;
    roi = track.rect;
    face = track;
}

void FaceAugmented::update(const Face & track) {
    cur = track.rect.getCenter();
    roi = track.rect;
    smooth.interpolate(cur, .5);
    all.addVertex(smooth);
    face = track;
}

void FaceAugmented::setImage(const ofPixels & pixels) {
    pixels.cropTo(image.getPixels(), roi.getX(), roi.getY(), roi.getWidth(), roi.getHeight());
    image.update();
    // how about calculating fisherfaces at this point?!
}

void FaceAugmented::kill() {
    float curTime = ofGetElapsedTimef();
    if(startedDying == 0) {
        startedDying = curTime;
    } else if(curTime - startedDying > dyingTime) {
        dead = true;
    }
}

void FaceAugmented::draw() {
    float alpha = startedDying ?
        ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, 255, 0, true) :
        255;
    
    ofPushStyle();
    
    // draw image roi
    ofSetColor(255, 255, 255, alpha);
    image.draw(roi);
    
    // draw trail
    ofNoFill();
    ofDrawCircle(cur, 16);
    ofSetColor(color, alpha);
    all.draw();
    
    // draw face rect
    ofSetColor(255, 0, 0, alpha);
    ofDrawRectangle(face.rect);
    
    // draw face features
    face.leftEye.draw();
    face.rightEye.draw();
    face.innerMouth.draw();
    face.outerMouth.draw();
    face.leftEyebrow.draw();
    face.rightEyebrow.draw();
    face.jaw.draw();
    face.noseBridge.draw();
    face.noseTip.draw();
    ofDrawCircle(face.leftEyeCenter, 7);
    ofDrawCircle(face.rightEyeCenter, 7);;
    
    // draw label
    ofSetColor(255, 255, 255, alpha);
    ofDrawBitmapString(ofToString(label), cur);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::setup(){
    tracker.setup("shape_predictor_68_face_landmarks.dat");
    tracker.setSmoothingRate(0.5);
    tracker.setDrawStyle(ofxDLib::lines);
    tracker.getTracker().setPersistence(5);
    tracker.getTracker().setMaximumDistance(80);
    
    video.setDeviceID(0);
    video.setup(720, 480);
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    if(video.isFrameNew()){
        tracker.findFaces(video.getPixels());
        vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
        for (auto & face : facesAugmented) {
            face.setImage(video.getPixels());
        }
        // amount of movement regulates smoothing rate
        for(int i=0; i < tracker.size(); i++) {
            int label = tracker.getLabel(i);
            int length = tracker.getVelocity(i).length();
            length = length < 10 ? length : 10; // arbitrarily choosen
            float smoothingRate = ofMap(length, 0, 10, 0.35, 1);
            tracker.setSmoothingRate(label, smoothingRate);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(128);
    video.draw(0, 0);
    vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
    for (auto & face : facesAugmented) {
        face.draw();
    }
    //tracker.draw();
}