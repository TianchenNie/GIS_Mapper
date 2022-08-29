/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   helper.h
 * Author: changry1
 *
 * Created on January 31, 2020, 4:23 PM
 */

#ifndef HELPER_H
#define HELPER_H
#include<string>
#include <map>
#include<vector>
#include "street.h"
#include "m1.h"
#include "city.h"
using namespace std;
bool includes(vector<int> array, int id);
string toLowerAndRemoveWhiteSpace(string str);
LatLon xy_coord(std::pair<LatLon, LatLon> points);
pair<double, double> Convert_Coordinates(LatLon Point);

multimap <string, Street> ::iterator findInMap(int streetID, City myCity);
std::vector<int> find_intersection_ids_from_partial_intersection_name(string intersection_prefix);

#endif /* HELPER_H */

