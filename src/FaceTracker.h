//
//  FaceTracker.h
//  example_FaceTracker
//
//  Created by Roy Macdonald on 15-01-16.
//
//

#pragma once
#include "ofxDLib.h"
#include "Tracker.h"

namespace ofxDLib {
    
    enum ShapeType {
        leftEye, rightEye, innerMouth, outerMouth, leftEyebrow, rightEyebrow, jaw, noseBridge, noseTip
    };
    
    typedef struct {
        int label;
        int age;
        ofRectangle rect;
        ofVec2f velocity, leftEyeCenter, rightEyeCenter;
        ofPolyline leftEye, rightEye, innerMouth, outerMouth, leftEyebrow, rightEyebrow, jaw, noseBridge, noseTip;
        vector<ofVec3f> landmarks;
    } Face;
    
    class FaceTracker {
    public:
        bool showLines = true;
        
        void setup(string predictorDatFilePath);
        void findFaces(const ofPixels& pixels, bool bUpscale = false);
        unsigned int size();
        RectTracker& getTracker();
        Face getFace(unsigned int i);
        ofRectangle getRectangle(unsigned int i);
        vector<ofVec3f> getLandmarks(unsigned int i);
        ofPolyline getShape(unsigned int i, ShapeType t);
        unsigned int getLabel(unsigned int i);
        int getIndexFromLabel(unsigned int label);
        ofVec2f getVelocity(unsigned int i);
        void draw();
    protected:
    
        // face tracker
        dlib::frontal_face_detector detector;
        dlib::shape_predictor predictor;
        vector<Face> faces;
        
        // assign labels
        RectTracker tracker;
    };
    
}