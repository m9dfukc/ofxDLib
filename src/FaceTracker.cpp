//
//  FaceTracker.cpp
//  example_FaceTracker
//
//  Created by Roy Macdonald on 15-01-16.
//
//

#include "FaceTracker.h"
using namespace ofxDLib;

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
    faces.clear();
    
    dlib::array2d<dlib::rgb_pixel> img;
    toDLib(pixels, img);
    if (bUpscale) pyramid_up(img);
    
    std::vector<dlib::rectangle> dets = detector(img);
    tracker.track(toOf(dets));
    
    for (int i=0; i<dets.size(); i++) {
        dlib::full_object_detection shapes = predictor(img, dets[i]);
        unsigned int label = tracker.getLabelFromIndex(i);
        
        Face face;
        face.label = label;
        face.rect = toOf(dets[i]);
        face.age = tracker.getAge(label);
        face.velocity = tracker.getVelocity(i);
        for (int j=0; j<shapes.num_parts(); j++) {
            face.landmarks.push_back(toOf(shapes.part(j)));
        }
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
        faces.push_back(face);
    }
}

//--------------------------------------------------------------
unsigned int FaceTracker::size() {
    return faces.size();
}

//--------------------------------------------------------------
RectTracker& FaceTracker::getTracker() {
    return tracker;
}

//--------------------------------------------------------------
Face FaceTracker::getFace(unsigned int i) {
    return faces[i];
}

//--------------------------------------------------------------
ofRectangle FaceTracker::getRectangle(unsigned int i) {
    return faces[i].rect;
}

//--------------------------------------------------------------
vector<ofVec3f> FaceTracker::getLandmarks(unsigned int i) {
    return faces[i].landmarks;
}

ofPolyline FaceTracker::getShape(unsigned int i, ShapeType t) {
    Face face = faces[i];
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
    return faces[i].label;
}

//--------------------------------------------------------------
int FaceTracker::getIndexFromLabel(unsigned int label) {
    int index = -1;
    for (int i=0; i<faces.size(); i++) {
        if (faces[i].label == label) {
            index = i;
            break;
        }
    }
    return index;
}

//--------------------------------------------------------------
ofVec2f FaceTracker::getVelocity(unsigned int i) {
    return faces[i].velocity;
}

//--------------------------------------------------------------
void FaceTracker::draw() {
    ofPushStyle();
    
    ofSetColor(ofColor::red);
    ofNoFill();
    
    for (auto & face : faces) {
        ofDrawBitmapString(ofToString(face.label), face.rect.getTopLeft());
        ofDrawRectangle(face.rect);
        if (showLines) {
            face.leftEye.draw();
            face.rightEye.draw();
            face.innerMouth.draw();
            face.outerMouth.draw();
            face.leftEyebrow.draw();
            face.rightEyebrow.draw();
            face.jaw.draw();
            face.noseBridge.draw();
            face.noseTip.draw();
        } else {
            for (auto & landmark : face.landmarks) {
                ofDrawCircle(landmark, 3);
            }
        }
        
    }
    
    ofPopStyle();
}
