/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3_wave_elem.h
 * Author: changry1
 *
 * Created on March 25, 2020, 9:11 PM
 */

#ifndef M3_WAVE_ELEM_H
#define M3_WAVE_ELEM_H
struct WaveElem {
    Intersection * node;
    int ssID;
    double travelTime;
    float estimatedTimeToReachFromHere;
    WaveElem(Intersection *n, int id, float time, float estTime) {
        node = n;
        ssID = id;
        travelTime = time;
        estimatedTimeToReachFromHere = estTime;
    }
};
struct compareWaveFrontPrioirityQueue {

    bool operator()(const WaveElem& l, const WaveElem& r) {
        return l.estimatedTimeToReachFromHere > r.estimatedTimeToReachFromHere;
    }
};


#endif /* M3_WAVE_ELEM_H */

