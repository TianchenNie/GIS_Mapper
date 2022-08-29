/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   street.h
 * Author: changry1
 *
 * Created on January 31, 2020, 4:20 PM
 */

#ifndef STREET_H
#define STREET_H
#include<string>
#include<vector>
#include<set>
using namespace std;
/*
 This class contains 4 data structures in total.
 * 
 * 2 to store street segments within this street
 *       set is for ensuring that there are no duplicates
 *       vector is for quick return
 * 2 to store intersections within this street 
 *       set is for ensuring that there are no duplicates
 *       vector is for quick return
 */
class Street {
public:
    int streetID;
    std::set<int> street_segments;
    std::set<int> intersections;
    std::vector<int> street_segments_v;
    std::vector<int> intersections_v;
    Street();

    Street(int strID);
};

#endif /* STREET_H */

