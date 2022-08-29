/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <map>
#include <math.h>
#include "helper.h"
#include "LatLon.h"
#include "street.h"
#include "m1.h"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include "city.h"
/*
 Loop through the string and remove white spaces and make every char lowercase
 */
string toLowerAndRemoveWhiteSpace(string str) {
    int length = str.length();
    for (int i = length - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            str.erase(i, 1);
        } else {
            str[i] = tolower(str[i]);
        }
    }
    return str;
}
/*
 Quick function to check if a vector includes an int
 */
bool includes(vector<int> array, int id) {
    for (int i = 0; i < array.size(); i++) {
        if (array[i] == id) {
            return true;
        }
    }
    return false;
}

/*
 Function to convert coordinates from latitude and longitude to easy values to calculate
 */
pair<double, double> Convert_Coordinates(LatLon Point) {

    double LatToRad = Point.lat() * DEGREE_TO_RADIAN;   //convert  coordinates with radian
    double LonToRad = Point.lon() * DEGREE_TO_RADIAN;   //convert  coordinates with radian 

    double x_coordiantes = LonToRad * cos(LatToRad) * EARTH_RADIUS_METERS;   //convert x_coordinates
    double y_coordiantes = LatToRad*EARTH_RADIUS_METERS;                     //convert x_coordinates
 
    return make_pair(x_coordiantes, y_coordiantes);     //return the pair 

}
