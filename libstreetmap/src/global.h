/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <vector>
#include <map>
#include "intersection.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include "PathToOther.h"
#include "PDIntersection.h"
#include <list>
#include "city.h"
using namespace std;
// m4
extern vector<list<PathToOther>> pdAllPaths;
extern vector<PDIntersection> pdIntersections;
bool setupPDIntersectionPaths(
        const IntersectionIndex intersect_id_start,
        const double turn_penalty, list<PathToOther>& pdIntersectPaths);

// City contains Streets, which each Street contains street segments and intersections
// Streets are sorted in two data structs: multimap (for search by partial prefix, indexed by StreetName) and vector (for quick access by streetID)
extern City myCity;
// Max speed for specific city 
extern double Global_max_speed;
extern vector<double>Global_travel_time;
// Intersections contains street segments
extern vector<Intersection> Global_intersection_nodes;

// OSM points globals
// Every OSM way and OSM node needs a pointer to acccess it. Therefore can use an unordered map for quick access by the OSMID
// Used for find OSM way length
extern unordered_map<OSMID, const OSMWay *> wayPointers;
extern unordered_map<OSMID, const OSMNode *> nodePointers;
extern unordered_map<int, vector <int>> street_intersections;

//Global Data Structures
struct intersection_data {
    LatLon position;
    std::string name;
    bool highlight = false;
};

struct feature {
    int featureID;
    vector<ezgl::point2d> points;
    enum FeatureType featureType;
    string fName;
    double wayArea;
    double wayLength;
    ezgl::color color;
    int isPolyLineOrPoint; //1 for Poly, 2 for line, 3 for point

    bool operator()(feature a, feature b) {
        return (a.wayArea > b.wayArea);
    }
};
extern vector<feature> Global_topLayerFeaturesList;
extern vector<feature> Global_lowerLayerFeaturesList;
extern vector<feature> Global_allFeatures;
extern std::vector<intersection_data> Global_intersections;
extern std::vector<IntersectionIndex> Global_from;
extern std::vector<IntersectionIndex> Global_to;
extern std::vector<int> Global_number_of_curve_points;
extern std::vector<string> Global_InterestName;
extern std::vector<string> Global_streetName;
extern std::vector<double> Global_fromx;
extern std::vector<double> Global_fromy;
extern std::vector<double> Global_tox;
extern std::vector<double> Global_toy;
extern std::vector<double> Global_degrees;
extern vector<const OSMNode*> Global_osm_subway_stations;
extern bool nightMode;

// Global coordinate conversion functions
float x_from_lon(float lon);
float y_from_lat(float lat);
float lon_from_x(float x);
float lat_from_y(float y);