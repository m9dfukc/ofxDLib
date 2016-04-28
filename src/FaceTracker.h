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
    
    enum DrawStyle {
        none, lines, circles
    };
    
    typedef struct {
        int label;
        int age;
        ofRectangle rect;
        ofVec2f velocity, leftEyeCenter, rightEyeCenter;
        ofPolyline leftEye, rightEye, innerMouth, outerMouth, leftEyebrow, rightEyebrow, jaw, noseBridge, noseTip;
        vector<ofVec3f> landmarks;
    } Face;
    
    float trackingDistance(const Face& a, const Face& b) {
        ofVec3f aCenter = a.rect.getCenter();
        ofVec3f bCenter = b.rect.getCenter();
        return aCenter.distance(bCenter);
    };
    
    class ShapeTracker : public Tracker<Face> {
    protected:
        float initSmoothingRate;
        std::map<unsigned int, float> smoothingRate;
        std::map<unsigned int, Face> smoothed;
    public:
        ShapeTracker()
        :initSmoothingRate(.5) {
        }
        void setSmoothingRate(float smoothingRate) {
            this->initSmoothingRate = smoothingRate;
        }
        void setSmoothingRate(unsigned int label, float smoothingRate) {
            this->smoothingRate[label] = smoothingRate;
        }
        float getSmoothingRate() const {
            return initSmoothingRate;
        }
        float getSmoothingRate(unsigned int label) {
            if (this->smoothingRate.count(label) > 0) return this->smoothingRate[label];
            else return initSmoothingRate;
        }
        const std::vector<unsigned int>& track(const std::vector<Face>& objects) {
            const std::vector<unsigned int>& labels = Tracker<Face>::track(objects);
            // add new objects, update old objects
            for(int i = 0; i < labels.size(); i++) {
                unsigned int label = labels[i];
                const Face& cur = getCurrent(label);
                const float smoothingRateCur = getSmoothingRate(label);
                if(smoothed.count(label) > 0) {
                    Face& smooth = smoothed[label];
                    smooth.rect.x = ofLerp(smooth.rect.x, cur.rect.x, smoothingRateCur);
                    smooth.rect.y = ofLerp(smooth.rect.y, cur.rect.y, smoothingRateCur);
                    smooth.rect.width = ofLerp(smooth.rect.width, cur.rect.width, smoothingRateCur);
                    smooth.rect.height = ofLerp(smooth.rect.height, cur.rect.height, smoothingRateCur);
                    for (int j=0; j<smooth.landmarks.size(); j++) {
                        ofVec3f & smoothLandmark = smooth.landmarks[j];
                        const ofVec3f & curLandmark = cur.landmarks[j];
                        smoothLandmark.x = ofLerp(smoothLandmark.x, curLandmark.x, smoothingRateCur);
                        smoothLandmark.y = ofLerp(smoothLandmark.y, curLandmark.y, smoothingRateCur);
                    }
                } else {
                    smoothingRate[label] = initSmoothingRate;
                    smoothed[label] = cur;
                }
            }
            std::map<unsigned int, Face>::iterator smoothedItr = smoothed.begin();
            while(smoothedItr != smoothed.end()) {
                unsigned int label = smoothedItr->first;
                if(!existsCurrent(label)) {
                    smoothed.erase(smoothedItr++);
                    smoothingRate.erase(label);
                } else {
                    ++smoothedItr;
                }
            }
            return labels;
        }
        const Face& getSmoothed(unsigned int label) const {
            return smoothed.find(label)->second;
        }
        ofVec2f getVelocity(unsigned int i) const {
            unsigned int label = getLabelFromIndex(i);
            if(existsPrevious(label)) {
                const Face& previous = getPrevious(label);
                const Face& current = getCurrent(label);
                ofVec2f previousPosition(previous.rect.x + previous.rect.width / 2, previous.rect.y + previous.rect.height / 2);
                ofVec2f currentPosition(current.rect.x + current.rect.width / 2, current.rect.y + current.rect.height / 2);
                return currentPosition - previousPosition;
            } else {
                return ofVec2f(0, 0);
            }
        }
    };
    
    class FaceTracker {
    protected:
        // face tracker
        dlib::frontal_face_detector detector;
        dlib::shape_predictor predictor;

        map<unsigned int, vector<ofVec3f>> shapeHistory;
        map<unsigned int, float> smoothingRatePerFace;
        float smoothingRate;
        DrawStyle drawStyle;
        ShapeTracker tracker;
        
        Face& assignFeatures(Face & face);
    public:
        FaceTracker();
        void setup(string predictorDatFilePath);
        void findFaces(const ofPixels& pixels, bool bUpscale = false);
        unsigned int size();
        ShapeTracker& getTracker();
        Face getFace(unsigned int i);
        vector<Face> getFaces();
        ofRectangle getRectangle(unsigned int i);
        vector<ofVec3f> getLandmarks(unsigned int i);
        ofPolyline getShape(unsigned int i, ShapeType t);
        unsigned int getLabel(unsigned int i);
        int getIndexFromLabel(unsigned int label);
        ofVec2f getVelocity(unsigned int i);
        void setSmoothingRate(float smoothingRate);
        void setSmoothingRate(unsigned int label, float smoothingRate);
        float getSmoothingRate();
        float getSmoothingRate(unsigned int label);
        void setDrawStyle(DrawStyle style);
        void draw();

    };
    
    template <class F>
    class FaceTrackerFollower : public FaceTracker {
    protected:
        std::vector<unsigned int> labels;
        std::vector<F> followers;
    public:
        FaceTrackerFollower() : FaceTracker() {};
        const std::vector<unsigned int>& findFaces(const ofPixels& pixels, bool bUpscale = false) {
            FaceTracker::findFaces(pixels);
            ShapeTracker & tracker = FaceTracker::getTracker();
            
            // kill missing, update old
            for(int i = 0; i < labels.size(); i++) {
                unsigned int curLabel = labels[i];
                F& curFollower = followers[i];
                if(!tracker.existsCurrent(curLabel)) {
                    curFollower.kill();
                } else {
                    curFollower.update(tracker.getCurrent(curLabel));
                }
            }
            // add new
            for(auto & curLabel : tracker.getNewLabels()) {
                labels.push_back(curLabel);
                followers.push_back(F());
                followers.back().setup(tracker.getCurrent(curLabel));
                followers.back().setLabel(curLabel);
            }
            // remove dead
            for(int i = labels.size() - 1; i >= 0; i--) {
                if(followers[i].getDead()) {
                    followers.erase(followers.begin() + i);
                    labels.erase(labels.begin() + i);
                }
            }
            return labels;
        };
        std::vector<F>& getFollowers() {
            return followers;
        };
    };
    
}