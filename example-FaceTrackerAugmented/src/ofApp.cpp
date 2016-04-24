#include "ofApp.h"

using namespace ofxDLib;

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
    dead = true;
}

void FaceAugmented::draw() {
    ofPushStyle();
    ofSetColor(255);
    image.draw(roi);
    ofSetColor(255);
    ofNoFill();
    ofDrawCircle(cur, 16);
    ofSetColor(color);
    all.draw();
    ofSetColor(255);
    ofDrawBitmapString(ofToString(label), cur);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::setup(){
    ft.setup("shape_predictor_68_face_landmarks.dat");
    ft.setSmoothingRate(0.5);
    ft.setDrawStyle(ofxDLib::lines);
    
    video.setDeviceID(0);
    video.setup(720, 480);
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    if(video.isFrameNew()){
        ft.findFaces(video.getPixels());
        tracker.track(ft.getFaces());
        vector<FaceAugmented>& facesAugmented = tracker.getFollowers();
        for (auto & face : facesAugmented) {
            face.setImage(video.getPixels());
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
    ft.draw();
}