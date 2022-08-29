/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   intersection.cpp
 * Author: changry1
 * 
 * Created on March 20, 2020, 5:12 PM
 */

#include "intersection.h"

Intersection::Intersection() {
    intersectionID = -1;
}
Intersection::Intersection(int intID) {
    intersectionID = intID;
    reachingStreetSegID = -1;
    walkingReachSSID=-1;
    walkingBestTime = 100000;
    bestTime = 100000;
}

Intersection::~Intersection() {
}

