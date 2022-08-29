/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   streetSegment.h
 * Author: changry1
 *
 * Created on March 21, 2020, 5:52 PM
 */

#ifndef STREETSEGMENT_H
#define STREETSEGMENT_H

class StreetSegment {
public:
    int id;
    int fromIntersection;
    int toIntersection;
    bool oneWay;
    int speedLimit; 
    bool isTurn;
    StreetSegment();
    StreetSegment(int ID,
    int from,
    int to,
    bool _oneWay,
    int _speedLimit,
    bool turn
    );
    virtual ~StreetSegment();
private:

};

#endif /* STREETSEGMENT_H */

