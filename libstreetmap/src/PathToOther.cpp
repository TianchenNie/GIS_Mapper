/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PathToOther.cpp
 * Author: changry1
 * 
 * Created on April 5, 2020, 6:16 PM
 */

#include "PathToOther.h"
/*[
 0:[//p0
    0:{
       id:45324//p0-p3,
       path:{0,1,2,3},
       besttime:100ms
    },
    1:{
       id:64364//p0-p1,
       path:{0,1,2,3},
       besttime:200ms
    },
    2:{
       id:1233//p0-d3,
       path:{0,1,2,3},
       besttime:300ms
    }
    ...
    2N-2:{
       id:13//p0-d23,
       path:{0,1,2,3},
       besttime:500ms
    }
 ],
 1:[//p1
    0:{
       id:45324//p0-p3,
       path:{0,1,2,3},
       besttime:100ms
    },
    1:{
       id:64364//p0-p1,
       path:{0,1,2,3},
       besttime:200ms
    },
    2:{
       id:1233//p0-d3,
       path:{0,1,2,3},
       besttime:300ms
    }
    ...
    2N-2:{
       id:13//p0-d23,
       path:{0,1,2,3},
       besttime:500ms
    }
 ]
]*/
PathToOther::PathToOther() {
    otherIntersectionID  = -1;
    path = {};
    bestTime = 1000000;
}

PathToOther::PathToOther(int _otherIntersectionID, std::vector<int>_path, double _bestTime){
    otherIntersectionID = _otherIntersectionID;
    path = _path;
    bestTime = _bestTime;
}
PathToOther::~PathToOther() {
}

