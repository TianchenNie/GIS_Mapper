/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   street.cpp
 * Author: changry1
 * 
 * Created on January 31, 2020, 4:20 PM
 */

#include "street.h"

Street::Street() {
        streetID = -1;
        street_segments = {};
        intersections = {};
        street_segments_v = {};
        intersections_v = {};
    }
Street::Street(int strID) {
        streetID = strID;
        street_segments = {};
        intersections = {};
        street_segments_v = {};
        intersections_v = {};
}

