//
//  FaceTracker.cpp
//  example_FaceTracker
//
//  Created by Roy Macdonald on 15-01-16.
//
//

#include "FaceTracker.h"
using namespace ofxDLib;

FaceTracker::FaceTracker() {
    smoothingRate = 0.5;
    drawStyle = lines;
    tracker.setSmoothingRate(smoothingRate);
}

//--------------------------------------------------------------
void FaceTracker::setup(string predictorDatFilePath) {
    detector = dlib::get_frontal_face_detector();
    if(predictorDatFilePath.empty()){
        predictorDatFilePath = ofToDataPath("shape_predictor_68_face_landmarks.dat");
    }
    ofFile f(predictorDatFilePath);
    if (f.exists()) {
        dlib::deserialize(f.getAbsolutePath()) >> predictor;
    } else {
        ofLogError("ofxDLib::FaceTracker","SHAPE PREDICTOR DAT FILE MISSING!!!");
    }
}

//--------------------------------------------------------------
void FaceTracker::findFaces(const ofPixels& pixels, bool bUpscale) {
    dlib::array2d<dlib::rgb_pixel> img;
    toDLib(pixels, img);
    if (bUpscale) pyramid_up(img);
    
    std::vector<dlib::rectangle> dets = detector(img);
    std::vector<Face> facesCur;
    for (int i=0; i<dets.size(); i++) {
        dlib::full_object_detection shapes = predictor(img, dets[i]);
        vector<ofVec3f> landmarks;
        for (int j=0; j<shapes.num_parts(); j++) {
            ofVec3f p(shapes.part(j).x(), shapes.part(j).y(), 0);
            landmarks.push_back(p);
        }
        Face face;
        face.rect = toOf(dets[i]);
        face.landmarks = landmarks;
        
        facesCur.push_back(face);
    }
    tracker.track(facesCur);
}

//--------------------------------------------------------------
unsigned int FaceTracker::size() {
    return tracker.getCurrentLabels().size();
}

//--------------------------------------------------------------
ShapeTracker & FaceTracker::getTracker() {
    return tracker;
}

//--------------------------------------------------------------
Face FaceTracker::getFace(unsigned int i) {
    unsigned int label = tracker.getLabelFromIndex(i);
    
    Face face = tracker.getSmoothed(label);
    face.label = label;
    face.age = tracker.getAge(label);
    face.velocity = tracker.getVelocity(i);
    face = assignFeatures(face);
    
    return face;
}

//--------------------------------------------------------------
vector<Face> FaceTracker::getFaces() {
    vector<Face> faces;
    for (int i=0; i < this->size(); i++) {
        faces.push_back(this->getFace(i));
    }
    return faces;
}

//--------------------------------------------------------------
ofRectangle FaceTracker::getRectangle(unsigned int i) {
    unsigned int label = tracker.getLabelFromIndex(i);
    return tracker.getCurrent(label).rect;
}

//--------------------------------------------------------------
vector<ofVec3f> FaceTracker::getLandmarks(unsigned int i) {
    unsigned int label = tracker.getLabelFromIndex(i);
    return tracker.getCurrent(label).landmarks;
}

ofPolyline FaceTracker::getShape(unsigned int i, ShapeType t) {
    Face face = this->getFace(i);
    ofPolyline out;
    
    switch (t) {
        case leftEye:
            out = face.leftEye;
            break;
        case rightEye:
            out = face.rightEye;
            break;
        case innerMouth:
            out = face.innerMouth;
            break;
        case outerMouth:
            out = face.outerMouth;
            break;
        case leftEyebrow:
            out = face.leftEyebrow;
            break;
        case rightEyebrow:
            out = face.rightEyebrow;
            break;
        case jaw:
            out = face.jaw;
            break;
        case noseBridge:
            out = face.noseBridge;
            break;
        case noseTip:
            out = face.noseTip;
            break;
        default:
            break;
    }
    
    return out;
}

//--------------------------------------------------------------
unsigned int FaceTracker::getLabel(unsigned int i) {
    return tracker.getLabelFromIndex(i);
}

//--------------------------------------------------------------
int FaceTracker::getIndexFromLabel(unsigned int label) {
    return tracker.getIndexFromLabel(label);
}

//--------------------------------------------------------------
ofVec2f FaceTracker::getVelocity(unsigned int i) {
    return tracker.getVelocity(i);
}

//--------------------------------------------------------------
void FaceTracker::setSmoothingRate(float smoothingRate) {
    tracker.setSmoothingRate(smoothingRate);
}

//--------------------------------------------------------------
void FaceTracker::setSmoothingRate(unsigned int label, float smoothingRate) {
    tracker.setSmoothingRate(label, smoothingRate);
}

//--------------------------------------------------------------
float FaceTracker::getSmoothingRate() {
    return tracker.getSmoothingRate();
}

//--------------------------------------------------------------
float FaceTracker::getSmoothingRate(unsigned int label) {
    return tracker.getSmoothingRate(label);
}

//--------------------------------------------------------------
void FaceTracker::setDrawStyle(DrawStyle style) {
    this->drawStyle = style;
}

//--------------------------------------------------------------
void FaceTracker::draw() {
    ofPushStyle();
    
    ofSetColor(ofColor::red);
    ofNoFill();
    
    for (auto & face : this->getFaces()) {
        ofDrawBitmapString(ofToString(face.label), face.rect.getTopLeft());
        ofDrawRectangle(face.rect);
        
        switch (drawStyle) {
            case lines:
                face.leftEye.draw();
                face.rightEye.draw();
                face.innerMouth.draw();
                face.outerMouth.draw();
                face.leftEyebrow.draw();
                face.rightEyebrow.draw();
                face.jaw.draw();
                face.noseBridge.draw();
                face.noseTip.draw();
                break;
            case circles:
                for (auto & landmark : face.landmarks) {
                    ofDrawCircle(landmark, 3);
                }
                break;
            case none:
                break;
        }
        
    }
    
    ofPopStyle();
}

//--------------------------------------------------------------
Face& FaceTracker::assignFeatures(Face & face) {
    if (face.landmarks.size() == 68) {
        for (int j=0; j<=16; j++) { // jaw
            face.jaw.addVertex(face.landmarks[j]);
        }
        
        for (int j=17; j<=21; j++) { // leftEyebrow
            face.leftEyebrow.addVertex(face.landmarks[j]);
        }
        
        for (int j=22; j<=26; j++) { // rightEyebrow
            face.rightEyebrow.addVertex(face.landmarks[j]);
        }
        
        for (int j=27; j<=30; j++) { // noseBridge
            face.noseBridge.addVertex(face.landmarks[j]);
        }
        
        for (int j=30; j<=35; j++) { // noseTip
            face.noseTip.addVertex(face.landmarks[j]);
        }
        face.noseTip.addVertex(face.landmarks[30]);
        face.noseTip.close();
        
        for (int j=36; j<=41; j++) { // leftEye
            face.leftEye.addVertex(face.landmarks[j]);
        }
        face.leftEye.addVertex(face.landmarks[36]);
        face.leftEye.close();
        face.leftEyeCenter = face.leftEye.getCentroid2D();
        
        for (int j=42; j<=47; j++) { // rightEye
            face.rightEye.addVertex(face.landmarks[j]);
        }
        face.rightEye.addVertex(face.landmarks[42]);
        face.rightEye.close();
        face.rightEyeCenter = face.rightEye.getCentroid2D();
        
        for (int j=48; j<=59; j++) { // outerMouth
            face.outerMouth.addVertex(face.landmarks[j]);
        }
        face.outerMouth.addVertex(face.landmarks[48]);
        face.outerMouth.close();
        
        for (int j=60; j<=67; j++) { // innerMouth
            face.innerMouth.addVertex(face.landmarks[j]);
        }
        face.innerMouth.addVertex(face.landmarks[60]);
        face.innerMouth.close();
    }
    
    return face;
}
