/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PathToOther.h
 * Author: changry1
 *
 * Created on April 5, 2020, 6:16 PM
 */

#ifndef PATHTOOTHER_H
#define PATHTOOTHER_H
#include <vector>
class PathToOther {
public:
    int otherIntersectionID;
    std::vector<int>path;
    double bestTime;
    PathToOther();
    PathToOther(int _otherIntersectionID, std::vector<int>_path, double _bestTime);
    virtual ~PathToOther();
private:

};

#endif /* PATHTOOTHER_H */

