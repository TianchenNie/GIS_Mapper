/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   intersection.h
 * Author: changry1
 *
 * Created on March 20, 2020, 5:12 PM
 */

#ifndef INTERSECTION_H
#define INTERSECTION_H
#include <vector>
using namespace std;
//Nodes used in A* Algorithm
class Intersection {
public:
    int intersectionID;
    int reachingStreetSegID;
    int walkingReachSSID;
    double walkingBestTime;
    double bestTime;
    vector <int> street_segments;
    Intersection();
    Intersection(int intersectionID);
    virtual ~Intersection();
private:

};

#endif /* INTERSECTION_H */

