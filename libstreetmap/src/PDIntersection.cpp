/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   courierIntersection.cpp
 * Author: changry1
 * 
 * Created on April 5, 2020, 4:22 PM
 */

#include "PDIntersection.h"
#include <vector>
PDIntersection::PDIntersection() {
    intersectionID = -1;
//    pickUp  = false;
//    dropOff = false;
    deliveriesWithThisIntersection = {};
}

PDIntersection:: PDIntersection(int _intersectionID, std::vector<int> _deliveriesWithThisIntersection){
    intersectionID = _intersectionID;
//    pickUp = _pickUp;
//    dropOff = _dropOff;
    deliveriesWithThisIntersection = _deliveriesWithThisIntersection;
}


PDIntersection::~PDIntersection() {
}

