/* 
 * Copyright 2020 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARIStempStreetSegmentsING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <algorithm>
#include <unordered_map> 
#include <set>
#include<math.h>
#include<vector>
#include<map>
#include "m1.h"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include "city.h"
#include "street.h"
#include "helper.h"
#include "intersection.h"
#include "global.h"
#include "m4.h"
#include <time.h>
#include <stdlib.h>
using namespace std;

// City contains Streets, which each Street contains street segments and intersections
// Streets are sorted in two data structs: multimap (for search by partial prefix, indexed by StreetName) and vector (for quick access by streetID)
City myCity;
// Intersections contains street segments
//vector<vector<int>> Global_street_segments_of_intersection;
vector<double>Global_travel_time;
vector<Intersection> Global_intersection_nodes;
double Global_max_speed = 0;
// Every OSM way and OSM node needs a pointer to acccess it. Therefore can use an unordered map for quick access by the OSMID
// Used for find OSM way length
unordered_map<OSMID, const OSMWay *> wayPointers;
unordered_map<OSMID, const OSMNode *> nodePointers;
unordered_map<int, vector <int>> street_intersections;
multimap <string, Street> ::iterator findInMap(int streetID);
multimap<string, IntersectionIndex> intersectionMap;
/*
 Function for multimap
 Find if streetID exists in my city 
 */

multimap <string, Street> ::iterator findInMap(int streetID) {
    // Cleanse input
    string name = toLowerAndRemoveWhiteSpace(getStreetName(streetID));
    // In multimap find by the name
    auto street = myCity.streets.find(name);
    // If that street's street ID isn't the same then find the next one until it is correct
    if (streetID != street->second.streetID) {
        bool exists = false;
        while (street->first == name) {
            if (street->second.streetID == streetID) {
                exists = true;
                break;

            }
            {
                street++;
            }
        }
        if (!exists) {
            return myCity.streets.end();

        } else {
            return street;
        }
    } else {
        // If streetID matches then its correct
        return street;
    }

}

bool load_map(std::string fn/*map_path*/) {

    bool load_successful = false; //Indicates whether the map has loaded 

    //Load your map related data structures here
    //

    load_successful = loadStreetsDatabaseBIN(fn); // &&
    if (!load_successful) {
        return false;
    }

    // change "streets" to "osm" in map file name
    int indBetweenDots = 0;
    for (indBetweenDots = 0; fn[indBetweenDots] != '.'; indBetweenDots++) {

    }
    string streets = fn;
    string osm = fn.replace(indBetweenDots + 1, 7, "osm");
    if (!loadOSMDatabaseBIN(osm)) {
        return false;
    }


    // Store the ways into a vector of pointers for easy access
    for (int wayIdx = 0; wayIdx < getNumberOfWays(); wayIdx++) {
        const OSMWay * way = getWayByIndex(wayIdx);
        wayPointers[way->id()] = way;
    }
    // Store the nodes into a vector of pointers for easy access
    for (int nodeIdx = 0; nodeIdx < getNumberOfNodes(); nodeIdx++) {
        const OSMNode * node = getNodeByIndex(nodeIdx);
        nodePointers[node->id()] = node;
    }
    for(int i = 0; i< getNumIntersections(); i++)
    {
        intersectionMap.insert(pair<string,IntersectionIndex>(toLowerAndRemoveWhiteSpace(getIntersectionName(i)), i));
    }
    // Load into the City Class the street's intersections and street segments
    // Store into two different places
    // 1. For searching a street by partial prefix: myCity.streets type-> Multimap
    // 2. For searching a street by index: myCity.streetQuick   type -> Vector
    // Use sets initially to ensure that there are no duplicates, and then push onto a vector in myCity.streetQuick
    myCity.streetsQuick.resize(getNumStreets());
    for (int streetSegmentIdx = 0; streetSegmentIdx < getNumStreetSegments(); streetSegmentIdx++) {

        // Get Street Segments information to put into the multimap of Streets
        InfoStreetSegment ss = getInfoStreetSegment(streetSegmentIdx);
        StreetIndex streetID = ss.streetID;
        string ob = getStreetName(streetID);
        string street_name = toLowerAndRemoveWhiteSpace(ob);
        IntersectionIndex fromIntersectionID = ss.from;
        IntersectionIndex toIntersectionID = ss.to;
        // Check if key exists inside multimap, need to ensure that duplicates streetName are accounted for
        multimap <string, Street> ::iterator street = findInMap(streetID);
        // If it doesn't exist, create a new street
        if (street == myCity.streets.end()) {
            // Create street with street segment information
            Street newStreet(streetID);
            newStreet.street_segments.insert(streetSegmentIdx);
            newStreet.intersections.insert(fromIntersectionID);
            newStreet.intersections.insert(toIntersectionID);
            // Push onto multimap and vector
            myCity.streets.insert(pair<string, Street>(street_name, newStreet));
            myCity.streetsQuick[streetID].intersections.insert(fromIntersectionID);
            myCity.streetsQuick[streetID].intersections.insert(toIntersectionID);
            myCity.streetsQuick[streetID].street_segments.insert(streetSegmentIdx);
        } else {
            // If it exists, then we can ensure that this is the correct streetID
            // because of findInMap helper function
            if (street->second.streetID == streetID) {
                // Push street segment information onto multimap and vector
                street->second.street_segments.insert(streetSegmentIdx);
                street->second.intersections.insert(fromIntersectionID);
                street->second.intersections.insert(toIntersectionID);
                myCity.streetsQuick[streetID].intersections.insert(fromIntersectionID);
                myCity.streetsQuick[streetID].intersections.insert(toIntersectionID);
                myCity.streetsQuick[streetID].street_segments.insert(streetSegmentIdx);
            } else {
                Street newStreet(streetID);
                newStreet.street_segments.insert(streetSegmentIdx);
                newStreet.intersections.insert(fromIntersectionID);
                newStreet.intersections.insert(toIntersectionID);

                myCity.streets.insert(pair<string, Street>(street_name, newStreet));
                myCity.streetsQuick[streetID].intersections.insert(fromIntersectionID);
                myCity.streetsQuick[streetID].intersections.insert(toIntersectionID);
                myCity.streetsQuick[streetID].street_segments.insert(streetSegmentIdx);
            }


        }
    }
    // This loop takes the set which has unique keys and stores them into a vector that will be returned quickly
    // for the functions: street's street segments and intersections for every street
    for (int x = 0; x < getNumStreets(); x++) {
        // Resize so that pushback isn't slow
        myCity.streetsQuick[x].intersections_v.resize(myCity.streetsQuick[x].intersections.size());
        myCity.streetsQuick[x].street_segments_v.resize(myCity.streetsQuick[x].street_segments.size());
        set <int> ::iterator itr, ptr;
        int intersectionIndex = 0;
        // Take from set the intersections and street segments and add to vector
        for (itr = myCity.streetsQuick[x].intersections.begin(); itr != myCity.streetsQuick[x].intersections.end(); ++itr) {
            myCity.streetsQuick[x].intersections_v[intersectionIndex] = (*itr);
            intersectionIndex++;
        }
        int ssIndex = 0;
        for (ptr = myCity.streetsQuick[x].street_segments.begin(); ptr != myCity.streetsQuick[x].street_segments.end(); ++ptr) {
            myCity.streetsQuick[x].street_segments_v[ssIndex] = (*ptr);
            ssIndex++;
        }
        // Sort for easy search
        std::sort(myCity.streetsQuick[x].intersections_v.begin(), myCity.streetsQuick[x].intersections_v.end());
        std::sort(myCity.streetsQuick[x].street_segments_v.begin(), myCity.streetsQuick[x].street_segments_v.end());
    }

    // Store Street segments into Intersections
    Global_intersection_nodes.resize(getNumIntersections());
    for (int i = 0; i < getNumIntersections(); i++) {
        //this loop stores all street segments of all intersections into a vector of vectors
        Intersection myIntersect(i);
        myIntersect.street_segments.resize(getIntersectionStreetSegmentCount(i));
        for (int j = 0; j < getIntersectionStreetSegmentCount(i); j++) {
            myIntersect.street_segments[j] = getIntersectionStreetSegment(i, j);
        }
        Global_intersection_nodes[i] = myIntersect;

    }
    //This for loop is to store each streets  travel time into vectors and this will help improving performance time a lot 
    for (int ssID = 0; ssID < getNumStreetSegments(); ssID++) {
        const double CONVERSION_SPEED = 0.277778;
        if(Global_max_speed<(getInfoStreetSegment(ssID).speedLimit * CONVERSION_SPEED)){
            Global_max_speed = (getInfoStreetSegment(ssID).speedLimit * CONVERSION_SPEED);
        }
        Global_travel_time.push_back((find_street_segment_length(ssID)) / (getInfoStreetSegment(ssID).speedLimit * CONVERSION_SPEED));

    }
//          vector<DeliveryInfo>  deliveries = {DeliveryInfo(50955, 114599, 3.64505), DeliveryInfo(13864, 84826, 150.28346), DeliveryInfo(106836, 40478, 132.79056), DeliveryInfo(9037, 47950, 133.22345), DeliveryInfo(56819, 91575, 29.19379), DeliveryInfo(114780, 2526, 13.61199), DeliveryInfo(66440, 3042, 64.46082), DeliveryInfo(43487, 146131, 188.11136), DeliveryInfo(48026, 115825, 122.91425), DeliveryInfo(9088, 13180, 194.16504), DeliveryInfo(135696, 89864, 140.79376), DeliveryInfo(70817, 46762, 55.83173), DeliveryInfo(128368, 75909, 53.42627), DeliveryInfo(7107, 38918, 139.53435), DeliveryInfo(131450, 69208, 62.54547), DeliveryInfo(54341, 50187, 132.25157), DeliveryInfo(55780, 131937, 115.65539), DeliveryInfo(18428, 22635, 1.23825), DeliveryInfo(148729, 29698, 171.64159), DeliveryInfo(26717, 126555, 111.32867), DeliveryInfo(90722, 88823, 146.61469), DeliveryInfo(89818, 53919, 37.15287), DeliveryInfo(56077, 99475, 5.99976), DeliveryInfo(57606, 120679, 14.50743), DeliveryInfo(36647, 74187, 88.43934), DeliveryInfo(36147, 120472, 55.23724), DeliveryInfo(26866, 53224, 157.20303), DeliveryInfo(7832, 78160, 147.67279), DeliveryInfo(35474, 100371, 113.03408), DeliveryInfo(49900, 91799, 68.13067), DeliveryInfo(110047, 105659, 135.47527), DeliveryInfo(65531, 147042, 126.80992), DeliveryInfo(92846, 73209, 41.46676), DeliveryInfo(76847, 96421, 41.83722), DeliveryInfo(147775, 116379, 124.71667), DeliveryInfo(99955, 88995, 198.43752), DeliveryInfo(24143, 51489, 177.14240), DeliveryInfo(115112, 98300, 40.57933), DeliveryInfo(91958, 124878, 4.58260), DeliveryInfo(60411, 58215, 6.86531), DeliveryInfo(147089, 80441, 195.18217), DeliveryInfo(113021, 46252, 39.01222), DeliveryInfo(88253, 56464, 159.00911), DeliveryInfo(110603, 12428, 128.93202), DeliveryInfo(116429, 120125, 77.88078), DeliveryInfo(95180, 105909, 183.02200), DeliveryInfo(145225, 24659, 55.50504), DeliveryInfo(137276, 56686, 83.74704), DeliveryInfo(99914, 68916, 153.42888), DeliveryInfo(28583, 104773, 165.24220)};
//        vector<int>  depots = {13, 51601, 61505, 85936, 132265};
//       double turn_penalty = 15.000000000;
//       double truck_capacity = 306.382446289;
//     vector<CourierSubpath>     result_path = traveling_courier(deliveries, depots, turn_penalty, truck_capacity);
;
//    vector<DeliveryInfo> deliveries = {DeliveryInfo(12329, 12927, 50.03543), DeliveryInfo(15162, 71331, 60.50912), DeliveryInfo(147022, 141242, 35.54058)};
//    vector<int> depots = {29107, 44932, 40220};
//    double turn_penalty = 15.000000000;
//    double truck_capacity = 13465.874023438;
//    vector<CourierSubpath> result_path = traveling_courier(deliveries, depots, turn_penalty, truck_capacity);    
srand(time(0));
    return load_successful;



 
}

void close_map() {
    //    Global_street_segments_of_intersection.clear();vector<double>Global_travel_ti me;
    wayPointers.clear();
    nodePointers.clear();
    street_intersections.clear();
    closeStreetDatabase();
    closeOSMDatabase();
}


//Close the map (if loaded)

//Returns the distance between two coordinates in meters

double find_distance_between_two_points(std::pair<LatLon, LatLon> points) { //Kris

    double distance_between_two_points;
    double first_Lat, first_Lon, second_Lat, second_Lon;
    double Lat_average; //declare variables

    first_Lat = points.first.lat() * DEGREE_TO_RADIAN; //convert each coordinates to degree to radian
    first_Lon = points.first.lon() * DEGREE_TO_RADIAN; //convert each coordinates to degree to radian
    second_Lat = points.second.lat() * DEGREE_TO_RADIAN; //convert each coordinates to degree to radian
    second_Lon = points.second.lon() * DEGREE_TO_RADIAN; //convert each coordinates to degree to radian
    Lat_average = (second_Lat + first_Lat) / 2; //solve the lat average 

    distance_between_two_points = EARTH_RADIUS_METERS * sqrt(pow(second_Lat - first_Lat, 2) + pow(second_Lon * cos(Lat_average) - first_Lon * cos(Lat_average), 2)); //use formula to find the final distance 
    return distance_between_two_points;

}

//Returns the length of the given street segment in meters

double find_street_segment_length(int street_segment_id) { //Kris


    IntersectionIndex from = getInfoStreetSegment(street_segment_id).from; //get the from street point 
    IntersectionIndex to = getInfoStreetSegment(street_segment_id).to; //get the to street point 
    double street_segment_length = 0; //declare variables 
    double distance_between_curve_points = 0; //declare variables 
    double distance_between_intersection_1 = 0; //declare variables 
    double distance_between_intersection_2 = 0; //declare variables 
    double distance_between_intersection_3 = 0; //declare variables 

    int number_of_curve_points = getInfoStreetSegment(street_segment_id).curvePointCount; //get the curve point 


    if (number_of_curve_points == 0) { //if it is a street without any curve points just get the distance 

        street_segment_length = find_distance_between_two_points(make_pair(getIntersectionPosition(to), getIntersectionPosition(from)));
    } else {
        for (int i = 0; i < number_of_curve_points - 1; i++) { //if there is curve points 
            distance_between_curve_points += find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint(i, street_segment_id), getStreetSegmentCurvePoint(i + 1, street_segment_id)));

        }
        distance_between_intersection_1 = find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint(0, street_segment_id), getIntersectionPosition(from)));
        distance_between_intersection_2 = find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint(number_of_curve_points - 1, street_segment_id), getIntersectionPosition(to)));
        distance_between_intersection_3 = find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint(number_of_curve_points - 1, street_segment_id), getIntersectionPosition(from)));
        if (getIntersectionPosition(from).lat() == getIntersectionPosition(to).lat() && getIntersectionPosition(from).lon() == getIntersectionPosition(to).lon()) { //if the to and from are the same. this is a corner case 
            street_segment_length = distance_between_curve_points + distance_between_intersection_1 + distance_between_intersection_3;
        } else {
            street_segment_length = distance_between_curve_points + distance_between_intersection_1 + distance_between_intersection_2;
        }

    }

    return street_segment_length; //return the street length 


    //
}
//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)

double find_street_segment_travel_time(int street_segment_id) { //Kris


    return Global_travel_time[street_segment_id]; //store travel time into the vectors and this will help passing the performance test 
}

//Returns the nearest intersection to the given position

int find_closest_intersection(LatLon my_position) { //Kris

    int closet_intersection = 0; //declare variables 

    for (int i = 1; i < getNumIntersections(); ++i) { //go through a loop to compare the distance between each intersection and my_position 


        if (find_distance_between_two_points(make_pair(my_position, getIntersectionPosition(i))) < find_distance_between_two_points(make_pair(my_position, getIntersectionPosition(closet_intersection)))) {
            closet_intersection = i; //get it 
        }

    }

    return closet_intersection;
}


//Returns the street segments for the given intersection 

std::vector<int> find_street_segments_of_intersection(int intersection_id) { //Jackson

    return Global_intersection_nodes[intersection_id].street_segments;
    //    return Global_street_segments_of_intersection[intersection_id];
}

//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)

std::vector<std::string> find_street_names_of_intersection(int intersection_id) { //Jackson
    vector <int> street_segment_ID_at_intersection = find_street_segments_of_intersection(intersection_id);
    vector<string> street_names_at_intersection;
    //loop through all street segments connected to the intersection and store in vector
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++) {
        InfoStreetSegment streetSegmentName = getInfoStreetSegment(street_segment_ID_at_intersection[i]);
        street_names_at_intersection.push_back(getStreetName(streetSegmentName.streetID));
    }
    return street_names_at_intersection;
}

//Returns true if you can get from intersection_ids.first to intersection_ids.second using a single 
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself

bool are_directly_connected(std::pair<int, int> intersection_ids) { //Jackson
    IntersectionIndex start = intersection_ids.first;
    IntersectionIndex end = intersection_ids.second;
    bool is_directly_connected = false;
    bool found_connection = false;
    StreetSegmentIndex connecting_segment_ID;
    vector<int> connections; //vector to store all street segments that are directly between two intersections

    if (start == end)//corner case
    {
        is_directly_connected = true;
    } else {
        vector<int> segments_connected_to_start = find_street_segments_of_intersection(start);
        vector<int> segments_connected_to_end = find_street_segments_of_intersection(end);
        //loop through all street segments connected to the intersections, and see if they have any street
        //segments in common, and store the common street segments in vector called connections and set
        //found_connection to true
        for (int i = 0; i < getIntersectionStreetSegmentCount(start); i++) {
            for (int j = 0; j < getIntersectionStreetSegmentCount(end); j++) {
                if (segments_connected_to_start[i] == segments_connected_to_end[j]) {
                    connecting_segment_ID = segments_connected_to_start[i];
                    connections.push_back(connecting_segment_ID);
                    found_connection = true;
                }
            }
        }
        //check for one way streets
        for (int j = 0; j < connections.size(); j++) {
            if (found_connection == true) {
                InfoStreetSegment connection = getInfoStreetSegment(connections[j]);
                //check for one way street
                if (connection.oneWay == false) {
                    is_directly_connected = true;
                } else {
                    if (connection.from == start && connection.to == end)//check starting and ending points of one way street
                    {
                        is_directly_connected = true;
                    }
                }
            }
        }
    }


    return is_directly_connected;
}

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections

std::vector<int> find_adjacent_intersections(int intersection_id) { //Jackson
    vector<int> adjacent_intersections;
    vector<int> connected_street_segment_IDs = find_street_segments_of_intersection(intersection_id);
    bool duplicate = false;
    //check for duplicates and if the intersection was not a duplicate of previous adjacent intersections, store
    //the intersection into adjacent intersection. Also account for one way streets.
    for (int i = 0; i < getIntersectionStreetSegmentCount(intersection_id); i++) {
        duplicate = false;
        for (int j = 0; j < adjacent_intersections.size(); j++)//check for duplicates
        {
            if (getInfoStreetSegment(connected_street_segment_IDs[i]).from == adjacent_intersections[j] ||
                    getInfoStreetSegment(connected_street_segment_IDs[i]).to == adjacent_intersections[j]) {
                duplicate = true;
            }
        }
        if (duplicate == false) {//store intersections if not duplicate, and check if one way street
            if (getInfoStreetSegment(connected_street_segment_IDs[i]).from == intersection_id && getInfoStreetSegment(connected_street_segment_IDs[i]).oneWay == false) {
                adjacent_intersections.push_back(getInfoStreetSegment(connected_street_segment_IDs[i]).to);
            } else if (getInfoStreetSegment(connected_street_segment_IDs[i]).to == intersection_id && getInfoStreetSegment(connected_street_segment_IDs[i]).oneWay == false) {
                adjacent_intersections.push_back(getInfoStreetSegment(connected_street_segment_IDs[i]).from);
            } else if (getInfoStreetSegment(connected_street_segment_IDs[i]).oneWay == true) {
                if (getInfoStreetSegment(connected_street_segment_IDs[i]).from == intersection_id) {
                    adjacent_intersections.push_back(getInfoStreetSegment(connected_street_segment_IDs[i]).to);
                }
            }
        }
    }

    return adjacent_intersections;

}

//Returns all street segments for the given street

std::vector<int> find_street_segments_of_street(int street_id) { //Ryan


    return myCity.streetsQuick[street_id].street_segments_v;
}

//Returns all intersections along the a given street

std::vector<int> find_intersections_of_street(int street_id) { //Ryan
    return myCity.streetsQuick[street_id].intersections_v;
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.

std::vector<int> find_intersections_of_two_streets(std::pair<int, int> street_ids) { //Ryan


    vector<int> commonIntersections; //This variable is for return the vector variables 
    int StreetOne = street_ids.first; //Get the first street ID
    int StreetTwo = street_ids.second; //Get the second street ID 

    vector<int> IntersectionsFirst = find_intersections_of_street(StreetOne); //Get all of the intersections from streetOne
    vector<int> IntersectionsSecond = find_intersections_of_street(StreetTwo); //Get all of the intersections from streetTwo


    int iterator = 0; //use iterator instead of i to make coding style better 
    //use while loop to iterate 
    while (iterator < IntersectionsSecond.size()) {
        //compare each street's intersection if they intersect each other 
        auto twoIntersection = find(IntersectionsFirst.begin(), IntersectionsFirst.end(), IntersectionsSecond[iterator]);

        if (twoIntersection != IntersectionsFirst.end()) {
            commonIntersections.push_back(IntersectionsSecond[iterator]);
        } else if ((twoIntersection == IntersectionsFirst.end())) {
            //nothing to do here, just to check conditions 

        }

        iterator = iterator + 1; //increment iterator 
    }

    return commonIntersections;






}

//Returns all street ids corresponding to street names that start with the given prefix
//The function should be case-insensitive to the street prefix. You should ignore spaces.
//For example, both "bloor " and "BloOrst" are prefixes to "Bloor Street East".
//If no street names match the given prefix, this routine returns an empty (length 0) 
//vector.
//You can choose what to return if the street prefix passed in is an empty (length 0) 
//string, but your program must not crash if street_prefix is a length 0 string.
std::vector<int> find_intersection_ids_from_partial_intersection_name(string intersection_prefix)
{
    vector<int> possible_intersection_ids;
    bool duplicate = false;
    if(intersection_prefix.length() == 0)
    {
        return {};
    }
    
    string lowercase = toLowerAndRemoveWhiteSpace(intersection_prefix);
    multimap<string, IntersectionIndex> ::iterator itr, itlow;
    itlow = intersectionMap.lower_bound(lowercase);
    for (itr = itlow; itr != intersectionMap.end() ; itr++)
    {
        duplicate = false;
        string intName = itr->first;
        int count = 0, scount = 0;
        bool isPrefix = true;
        while(count<lowercase.length())
        {
            if(intName[scount] == lowercase[count])
            {
                scount++;
                count++;
            }else
            {
                isPrefix = false;
                break;
            }
        }
        if(isPrefix)
        {
            for(int j = 0; j < possible_intersection_ids.size(); j++)
            {
                string temp = getIntersectionName(possible_intersection_ids[j]);
                if(getIntersectionName(itr->second) == temp)
                {
                    duplicate = true;
                }
            }
            if(!duplicate)
            {
            possible_intersection_ids.push_back(itr->second);
            }
        }else
        {
            break;
        }
    }
    return possible_intersection_ids;
}
std::vector<int> find_street_ids_from_partial_street_name(std::string street_prefix) { //Ryan
    // Check if street prefix is empty
    vector<int> street_ids_containing;
    if (street_prefix.length() == 0) {
        return street_ids_containing;
    }

    // Turn prefix to lower case
    string lowercase = toLowerAndRemoveWhiteSpace(street_prefix);

    // Iterate through the multimap starting at prefix itself as lowerbound
    std::multimap<string, Street>::iterator itr, itlow;
    itlow = myCity.streets.lower_bound(lowercase);
    for (itr = itlow; itr != myCity.streets.end(); ++itr) {
        string streetName = itr->first;
        int count = 0, scount = 0;
        bool isPrefix = true;
        while (count < lowercase.length()) {
            if (streetName[scount] == ' ') {
                scount++;
            } else {
                if (tolower(streetName[scount]) == lowercase[count]) {
                    scount++;
                    count++;
                } else {
                    // If it isn't a prefix then break
                    isPrefix = false;
                    break;
                }
            }
        }
        
        if (isPrefix) {
    
            
                street_ids_containing.push_back(itr->second.streetID);
            
        } else {
            // if it isn't a prefix then return because everything afterwards isnt one as well
            break;
        }
    }

    return street_ids_containing;
}

//Returns the area of the given closed feature in square meters
//Assume a non self-intersecting polygon (i.e. no holes)
//Return 0 if this feature is not a closed polygon.

double find_feature_area(int feature_id) { //Kris

    bool isPolygon; //to see if the area is polygon


    int feature_count = getFeaturePointCount(feature_id); //get the number of feature point 
    LatLon first_point = getFeaturePoint(0, feature_id); //Get the first point of the feature area
    LatLon last_point = getFeaturePoint(feature_count - 1, feature_id); //Get the last point of the feature area 

    if (last_point.lat() == first_point.lat() && last_point.lon() == first_point.lon()) {
        isPolygon = true;
    } else {//convert x_coordinates
        isPolygon = false;
    }
    double feature_area = 0;

    if (isPolygon == false) { //when the feature area is not a closed area 
        feature_area = 0;
    } else if (isPolygon == true) {
        int j = feature_count - 1;
        for (int i = 0; i < feature_count; i++) { //use shoelace formula 
            feature_area += ((Convert_Coordinates(getFeaturePoint(j, feature_id)).first)+(Convert_Coordinates(getFeaturePoint(i, feature_id)).first))*
                    ((Convert_Coordinates(getFeaturePoint(j, feature_id)).second) - Convert_Coordinates(getFeaturePoint(i, feature_id)).second);
            j = i;
        }
        feature_area = abs(feature_area / 2.0); //shoelace formula 

    }
    return feature_area; //return feature area 

}

//Returns the length of the OSMWay that has the given OSMID, in meters.
//To implement this function you will have to  access the OSMDatabaseAPI.h 
//functions.

double find_way_length(OSMID way_id) { //Together 

    int count = 0;
    double length = 0;
    vector<const OSMNode*> nodes;
    const OSMWay * way = wayPointers[way_id];
    vector<OSMID> nodeOSMIDs = getWayMembers(way); //OSMID nodes
    for (int k = 0; k < nodeOSMIDs.size(); k++) {
        // Use extra vector for easy addingtempStreetSegments
        if (count > 0) {
            // Convert to lat and long and then call other function to find distance
            LatLon p1 = getNodeCoords(nodes[count - 1]);
            LatLon p2 = getNodeCoords(nodePointers[nodeOSMIDs[k]]);
            pair<LatLon, LatLon> pairPoints(p1, p2);
            length = length + (find_distance_between_two_points(pairPoints));
        }
        count++;
        nodes.push_back(nodePointers[nodeOSMIDs[k]]); // Push back 

    }


    return length;
}
