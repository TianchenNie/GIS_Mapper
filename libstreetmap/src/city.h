/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   city.h
 * Author: changry1
 *
 * Created on January 31, 2020, 4:19 PM
 */

#ifndef CITY_H
#define CITY_H
#include<string>
#include<vector>
#include<map>
#include"street.h"
using namespace std;
/*
 This City stores the streets in two different data structures
 * one is sorted by name and is all in lowercase and without spaces, used for searching by partial names
 * one is sorted by streetIndex for quick access
 
 */
class City {
public:
    std::multimap<string, Street> streets;
    vector<Street> streetsQuick;
};

#endif /* CITY_H */

