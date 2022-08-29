/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   streetSegment.cpp
 * Author: changry1
 * 
 * Created on March 21, 2020, 5:52 PM
 */

#include "streetSegment.h"

StreetSegment::StreetSegment(int ID,
    int from,
    int to,
    bool _oneWay,
    int _speedLimit,
        bool turn
    ) {
    id= ID;
    fromIntersection = from;
    toIntersection = to;
    oneWay = _oneWay;
    speedLimit = _speedLimit;
    isTurn = turn;
    //    subFrom = sFrom;

    //    subTo = sTo;
}


StreetSegment::StreetSegment(){
    id= -100; 
    fromIntersection = -100; 
    toIntersection = -100; 
    oneWay = -100; 
    speedLimit = -100;    
    isTurn = true;
}
StreetSegment::~StreetSegment() {
}

