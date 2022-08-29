/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3.cpp
 * Author: lichaohe
 *
 * Created on March 18, 2020, 9:56 PM
 */
#include <stdlib.h>
#include <cstdlib>
#include <m3.h>
#include "m1.h"
#include "m2.h"
#include <iostream>
#include "math.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "intersection.h"
#include "city.h"
#include "m2.h"
#include <list>
#include <utility>
#include <bits/stl_list.h>
#include "global.h"
#include <algorithm>
#include <cassert>
#include <queue>
#include "m3_wave_elem.h"
#include "PathToOther.h"
#include "PDIntersection.h"
bool bfsPathMultiDest(vector<Intersection*> sourceNodes, vector< IntersectionIndex> endIntersections, const double turn_penalty, bool endEarly);
using namespace std;

static const int NO_SS_ID = -1;

/*
 M3:
 * Use Djikstra with priority queue to find shortest path
 * Tried A* but only slowed process
 */
/*
 * findPath calls bfsPath returning vector of streetSegmentIDS
 */
vector<int> findPath(vector<Intersection*>sourceNodes, int destID, double turn_penalty, bool endEarly);
/*
 * find_path_between_intersections_toggle_end_early: used for finding path with walking
 * find_shortest_path_between_intersections_with_multiple_sources: used for finding path with walking
 * find_walk_path_between_intersections: called by test
 * 
 * they all set up findPath based on different parameters 
 */
std::vector<StreetSegmentIndex> find_path_between_intersections_toggle_end_early(
        const IntersectionIndex intersect_id_start,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty, bool endEarly);
std::vector<StreetSegmentIndex> find_shortest_path_between_intersections_with_multiple_sources(
        vector< IntersectionIndex>startIntersections,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty);
/*
 * Find most optimal path with walking
 * First finds all walkableIntersections using bfsPath_walk
 * Then find shortest path from destID to walkableIntersections using find_shortest_path_between_intersections_with_multiple_sources
 */
std::pair<std::vector<StreetSegmentIndex>, std::vector < StreetSegmentIndex >>find_walk_path_between_intersections(
        const IntersectionIndex intersect_id_start,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty,
        const double walking_speed,
        const double walking_time_limit);
/*
 * Find Fastest path to destID from multiple source ID
 * Nodes: Intersections
 * Costs: Street Segments Travel Time and Turns penalty
 * Implementation: Djikstra with prioirity queue
 * Heuristic: Manhattan Approach (dy + dx)/MAX_SPEED 
 */
bool bfsPath(vector<Intersection*> sourceNodes, int destID, const double turn_penalty, bool endEarly);
/*
 * Find Fastest path to destID from multiple source ID
 * Nodes: Intersections
 * Costs: Street Segments Travel Time and Turns penalty
 * Implementation: Djikstra with prioirity queue
 * Usage: Find walkableIntersections by setting up intersections vector so that know the shortest path from sourceNode to each intersection in the walking_time_limit
 * Will be used in find_path_with_walk_to_pick_up to find shortest path to destID from walkableIntersections
 */
set<int> bfsPath_walk(Intersection* sourceNode, const double turn_penalty, const double walking_speed, const double walking_time_limit);
/*
 * Trace back: need two functions because do not walk to lose the walkableIntersection paths
 * bfsTraceBackWalk: used when you've found the driving path to one of the walkableIntersections
 */
list<StreetSegmentIndex> bfsTraceBack(int destID);
list<StreetSegmentIndex> bfsTraceBackWalk(int destID);


/*
 * Get intersection id at other side of the street segment
 */
IntersectionIndex intersectionIdOtherEdgeofStreetSeg(StreetSegmentIndex ssid, IntersectionIndex currentIntersectionID);




double compute_path_travel_time(const std::vector<StreetSegmentIndex>& path,
        const double turn_penalty) {

    //declare local variables for return double type
    double total_travel_time = 0;
    int num_of_turns = 0;
    //    auto start = path.begin();

    //get the paths size 
    auto number_of_segments = path.size();

    //if the route is empty return 0
    if (number_of_segments == 0) {
        total_travel_time = 0;
        return 0;
    }
    //Using while loops to solve for street times each 
    int i = 0;
    while (i < number_of_segments) {
        total_travel_time += find_street_segment_travel_time(path[i]);
        ++i;
    }
    //Using while loops to solve for turns and add the turn penalties
    //auto j = path.begin();
    int j = 0;
    //get the street where the route begins 
    auto compare_Street = getInfoStreetSegment(path[j]).streetID;
    while (j < number_of_segments) {
        auto next_Street = getInfoStreetSegment(path[j]).streetID;
        if (next_Street != compare_Street) {
            num_of_turns++;
            compare_Street = next_Street;

        }
        ++j;
    }
    //    cout<<"end time"<<total_travel_time + num_of_turns*turn_penalty<<endl;
    return total_travel_time + num_of_turns*turn_penalty;
}

double compute_path_walking_time(const std::vector<StreetSegmentIndex>& path,
        const double walking_speed,
        const double turn_penalty) {

    //declare local variables for return double type 
    double total_travel_time = 0;
    int num_of_turns = 0;

    int number_of_segments = path.size();
    //    auto start = path.begin();
    if (number_of_segments == 0) {
        total_travel_time = 0;
        return 0;
    }
    int i = 0;
    while (i < number_of_segments) {
        total_travel_time += find_street_segment_length(path[i]) / walking_speed;
        ++i;
    }
    //Using while loops to solve for turns and add the turn penalties
    int j = 0;
    //get the street where the route begins 
    auto compare_Street = getInfoStreetSegment(path[j]).streetID;
    while (j < number_of_segments) {
        auto next_Street = getInfoStreetSegment(path[j]).streetID;
        if (next_Street != compare_Street) {
            num_of_turns++;
            compare_Street = next_Street;
        }
        ++j;
    }
    return total_travel_time + num_of_turns*turn_penalty;
}

vector<int> findPath(vector<Intersection*>sourceNodes, int destID, double turn_penalty, bool endEarly) {
    // BFS sets up intersections
    bool found = bfsPath(sourceNodes, destID, turn_penalty, endEarly);
    if (found) {
        // Traceback from destination node
        list<StreetSegmentIndex> data = bfsTraceBack(destID);
        // Convert list to vector
        vector<StreetSegmentIndex> path;
        path.resize(data.size());
        std::list<StreetSegmentIndex>::iterator it;
        int count = 0;
        for (it = data.begin(); it != data.end(); ++it) {
            path[count] = *it;
            count++;

        }
        return path;
    }
    return
    {
    };
}

std::pair<std::vector<StreetSegmentIndex>, std::vector < StreetSegmentIndex >>
find_path_with_walk_to_pick_up(
        const IntersectionIndex start_intersection,
        const IntersectionIndex end_intersection,
        const double turn_penalty,
        const double walking_speed,
        const double walking_time_limit) {
    // Check if already at the destination
    int sourceID = start_intersection;
    int destID = end_intersection;
    if (sourceID == destID) {
        return
        {
            {
            },
            {
            }

        };
    }
    // Check if can't walk only return driving path
    if (walking_speed == 0 || walking_time_limit == 0) {
        return
        {
            {
            }, find_path_between_intersections_toggle_end_early(start_intersection, end_intersection, turn_penalty, false)
        };
    }

    // Find all walkableIntersections
    Intersection* sourceNode = &Global_intersection_nodes[sourceID];
    set<int> walkableIntersections = bfsPath_walk(sourceNode, turn_penalty, walking_speed, walking_time_limit);
    // Convert set to vector
    vector<int> walkableVector;
    bool walkable = false;
    for (auto f : walkableIntersections) {
        if (f == end_intersection) {
            walkable = true;
        }
        walkableVector.push_back(f);
    }
    vector<StreetSegmentIndex> walkPath;
    vector<int> drivePath;
    // If walkable
    if (!walkable) {
        // Find shortest path to destID from one walkableIntersection
        drivePath = find_shortest_path_between_intersections_with_multiple_sources(walkableVector, end_intersection, turn_penalty);
        // Check if drivable to these intersections
        if (drivePath.size() > 0) {
            // Check which walkableIntersection is apart of drive path's first street segment
            set<int>::iterator from = walkableIntersections.find(getInfoStreetSegment(drivePath[0]).from);
            set<int>::iterator to = walkableIntersections.find(getInfoStreetSegment(drivePath[0]).to);
            if (from != walkableIntersections.end()) {
                // Call traceback from from to get walkPath
                list<StreetSegmentIndex> data = bfsTraceBackWalk(*from);
                walkPath.resize(data.size());
                std::list<StreetSegmentIndex>::iterator it;
                int count = 0;
                for (it = data.begin(); it != data.end(); ++it) {
                    walkPath[count] = *it;
                    count++;
                }
            }
            if (to != walkableIntersections.end()) {
                // Call traceback from to to get the walkPath
                list<StreetSegmentIndex> data = bfsTraceBackWalk(*to);
                walkPath.resize(data.size());
                std::list<StreetSegmentIndex>::iterator it;
                int count = 0;
                for (it = data.begin(); it != data.end(); ++it) {
                    walkPath[count] = *it;
                    count++;
                }
            }
        }

    }
    // Return empty drivePath if walkable otherwise return drivepath and walk path
    return
    {
        walkPath, drivePath
    };
}

std::vector<StreetSegmentIndex> find_path_between_intersections_toggle_end_early(
        const IntersectionIndex intersect_id_start,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty, bool endEarly) {
    int sourceID = intersect_id_start;
    int destID = intersect_id_end;
    Intersection* sourceNode = &Global_intersection_nodes[sourceID];
    vector<Intersection*> sourceNodes = {sourceNode};
    return findPath(sourceNodes, destID, turn_penalty, endEarly);



}

std::vector<StreetSegmentIndex> find_path_between_intersections(
        const IntersectionIndex intersect_id_start,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty) {
    int sourceID = intersect_id_start;
    int destID = intersect_id_end;
    Intersection* sourceNode = &Global_intersection_nodes[sourceID];
    vector<Intersection*> sourceNodes = {sourceNode};
    return findPath(sourceNodes, destID, turn_penalty, true);



}

std::vector<StreetSegmentIndex> find_shortest_path_between_intersections_with_multiple_sources(
        vector< IntersectionIndex>startIntersections,
        const IntersectionIndex intersect_id_end,
        const double turn_penalty) {
    // Add multiple sources
    vector<Intersection*> sourceNodes;
    sourceNodes.resize(startIntersections.size());
    for (int i = 0; i < startIntersections.size(); i++) {
        Intersection* node = &Global_intersection_nodes[startIntersections[i]];
        sourceNodes[i] = node;
    }
    int destID = intersect_id_end;
    return findPath(sourceNodes, destID, turn_penalty, false);


}

bool setupPDIntersectionPaths(
        const IntersectionIndex intersect_id_start,
        const double turn_penalty, list<PathToOther>& pdIntersectPaths) {
    // Add single sources
    vector<Intersection*> sourceNodes;
    vector< IntersectionIndex> endIntersections;
    sourceNodes.push_back(&Global_intersection_nodes[intersect_id_start]);
    // Add all ending nodes that aren't itself
    for (int i = 0; i < pdIntersections.size(); i++) {
        if (pdIntersections[i].intersectionID != intersect_id_start) {
            endIntersections.push_back(pdIntersections[i].intersectionID);
        }

    }
    // BFS sets up intersections
    bool found = bfsPathMultiDest(sourceNodes, endIntersections, turn_penalty, true);
    if (found) {
        // Traceback from each destination node
        for (int i = 0; i < endIntersections.size(); i++) {
            
            list<StreetSegmentIndex> data = bfsTraceBack(endIntersections[i]);
//            data.reverse();
            // Convert list to vector
            vector<StreetSegmentIndex> path;
            path.resize(data.size());
            std::list<StreetSegmentIndex>::iterator it;
            int count = 0;
            for (it = data.begin(); it != data.end(); ++it) {
                path[count] = *it;
                count++;

            }
            // Create path and insert into pdAllPaths based on bestTime
            PathToOther newPath(endIntersections[i], path, Global_intersection_nodes[endIntersections[i]].bestTime);
            list<PathToOther>::iterator pdi;
            int count2 = 0;
            bool placed = false;
            for (pdi = pdIntersectPaths.begin(); pdi != pdIntersectPaths.end(); pdi++) {
                if (newPath.bestTime < pdi->bestTime  ) {
                    pdIntersectPaths.emplace(pdi, newPath);
                    placed = true;
                    break;
                }
                count2++;
            }
            if(!placed){
                pdIntersectPaths.push_back(newPath);
            }
            
            
        }
        return true;

    }
    // Need to catch cases
    cout << "cannot return a path from source to one of the dests" << endl;
    int s;
    cin>>s;
    return false;

    //    return findPath(sourceNodes, destID, turn_penalty, false);


}

bool bfsPathMultiDest(vector<Intersection*> sourceNodes, vector< IntersectionIndex> endIntersections, const double turn_penalty, bool endEarly) {
    // Reset all best time and reaching street segments
    for (int i = 0; i < getNumIntersections(); i++) {
        Global_intersection_nodes[i].bestTime = 100000;
        Global_intersection_nodes[i].reachingStreetSegID = -1;
    }
    bool foundIt = false;
    // Priority queue for Djikstra's to make everything look in a good direction
    std::priority_queue<WaveElem, vector<WaveElem>, compareWaveFrontPrioirityQueue> wavefront;
    // Init wave with source nodes
    for (int i = 0; i < sourceNodes.size(); i++) {
        WaveElem startingPoint(sourceNodes[i], NO_SS_ID, 0, 0);
        wavefront.push(startingPoint);
        sourceNodes[i]->reachingStreetSegID = -1;
        sourceNodes[i]->bestTime = 0;

    }
    int numDestFound = 0;
    while (!wavefront.empty()) {
        WaveElem wave = wavefront.top(); // Get next node
        wavefront.pop();
        Intersection *currNode = wave.node;
        // Check if found a new end destination 
        for (int i = 0; i < endIntersections.size(); i++) {
            if (currNode->intersectionID == endIntersections[i]) {
                endIntersections[i] = -1; // Ensure doesn't double count same endIntersection
                numDestFound++;
            }
        }
        if (endEarly && numDestFound == endIntersections.size()) {
            return true;
        }
        /*if (currNode->intersectionID == destID) {
            foundIt = true;
            // End early if don't want to check all paths
            if (endEarly) {
                return true;
            }
        }*/
        // Add the intersection's street segments that go out
        for (int i = 0; i < currNode->street_segments.size(); i++) {
            int streetSegmentID = currNode->street_segments[i];
            // check if one way
            if (getInfoStreetSegment(streetSegmentID).oneWay && currNode->intersectionID != getInfoStreetSegment(streetSegmentID).from) {
                continue;
            }
            // Get opposite side node
            int toIntersectionID = intersectionIdOtherEdgeofStreetSeg(streetSegmentID, currNode->intersectionID);
            Intersection* nextNode = &Global_intersection_nodes[toIntersectionID];


            // Account for turning
            double travelTimeFromLastNode = find_street_segment_travel_time(streetSegmentID);
            if (currNode->reachingStreetSegID != -1) {
                if ((getInfoStreetSegment(currNode->reachingStreetSegID).streetID != getInfoStreetSegment(streetSegmentID).streetID)) {
                    travelTimeFromLastNode += turn_penalty;
                }

            }

            double timeToReachNextNode = currNode->bestTime + travelTimeFromLastNode;

            // Check if this was the best path to this node from other times reach this node
            if (timeToReachNextNode < nextNode->bestTime) {
                // Update the node's best path
                nextNode->reachingStreetSegID = streetSegmentID;
                nextNode->bestTime = timeToReachNextNode;
                wavefront.push(
                        WaveElem(nextNode, streetSegmentID, currNode->bestTime + travelTimeFromLastNode, timeToReachNextNode));
            }

        }
    }
    //    } 
    // No more to search in wave front
    if (!foundIt) {
        cout<<"found"<<endl;
        for(int i = 0;i<endIntersections.size();i++){
            cout<<endIntersections[i]<<endl;
        }
        cout<<"Source:"<<sourceNodes[0]->intersectionID<<endl;
        cout<<numDestFound<<endl;
        cout<<endIntersections.size()<<endl;
        cout << "cannot return a path from source to one of the dests" << endl;
        int s;
        cin>>s;
    }
    return false;


}

bool bfsPath(vector<Intersection*> sourceNodes, int destID, const double turn_penalty, bool endEarly) {
    // Reset all best time and reaching street segments
    for (int i = 0; i < getNumIntersections(); i++) {
        Global_intersection_nodes[i].bestTime = 100000;
        Global_intersection_nodes[i].reachingStreetSegID = -1;
    }
    bool foundIt = false;
    // Priority queue for Djikstra's to make everything look in a good direction
    std::priority_queue<WaveElem, vector<WaveElem>, compareWaveFrontPrioirityQueue> wavefront;
    // Init wave with source nodes
    for (int i = 0; i < sourceNodes.size(); i++) {
        WaveElem startingPoint(sourceNodes[i], NO_SS_ID, 0, 0);
        wavefront.push(startingPoint);
        sourceNodes[i]->reachingStreetSegID = -1;
        sourceNodes[i]->bestTime = 0;

    }
    while (!wavefront.empty()) {
        WaveElem wave = wavefront.top(); // Get next node
        wavefront.pop();
        Intersection *currNode = wave.node;
        // Check if found destination 
        if (currNode->intersectionID == destID) {
            foundIt = true;
            // End early if don't want to check all paths
            if (endEarly) {
                return true;
            }
        }
        // Add the intersection's street segments that go out
        for (int i = 0; i < currNode->street_segments.size(); i++) {
            int streetSegmentID = currNode->street_segments[i];
            // check if one way
            if (getInfoStreetSegment(streetSegmentID).oneWay && currNode->intersectionID != getInfoStreetSegment(streetSegmentID).from) {
                continue;
            }
            // Get opposite side node
            int toIntersectionID = intersectionIdOtherEdgeofStreetSeg(streetSegmentID, currNode->intersectionID);
            Intersection* nextNode = &Global_intersection_nodes[toIntersectionID];


            // Account for turning
            double travelTimeFromLastNode = find_street_segment_travel_time(streetSegmentID);
            if (currNode->reachingStreetSegID != -1) {
                if ((getInfoStreetSegment(currNode->reachingStreetSegID).streetID != getInfoStreetSegment(streetSegmentID).streetID)) {
                    travelTimeFromLastNode += turn_penalty;
                }

            }

            double timeToReachNextNode = currNode->bestTime + travelTimeFromLastNode;

            // Check if this was the best path to this node from other times reach this node
            if (timeToReachNextNode < nextNode->bestTime) {
                // Update the node's best path
                nextNode->reachingStreetSegID = streetSegmentID;
                nextNode->bestTime = timeToReachNextNode;
                wavefront.push(
                        WaveElem(nextNode, streetSegmentID, currNode->bestTime + travelTimeFromLastNode, timeToReachNextNode));
            }

        }
    }
    //    } 
    // No more to search in wave front
    return foundIt;


}

set<int> bfsPath_walk(Intersection* sourceNode, const double turn_penalty, const double walking_speed,
        const double walking_time_limit) {
    // Reset all for just walking times and reaching street ids
    set<int>walkableIntersections;
    for (int i = 0; i < getNumIntersections(); i++) {
        Global_intersection_nodes[i].walkingBestTime = 100000;
        Global_intersection_nodes[i].walkingReachSSID = -1;
    }
    // Priority queue for Djikstra's to make everything look in a good direction
    std::priority_queue<WaveElem, vector<WaveElem>, compareWaveFrontPrioirityQueue> wavefront;
    // Init first wave
    WaveElem startingPoint(sourceNode, NO_SS_ID, 0, 0);
    wavefront.push(startingPoint);
    sourceNode->walkingReachSSID = -1;
    sourceNode->walkingBestTime = 0;
    walkableIntersections.insert(sourceNode->intersectionID);
    while (!wavefront.empty()) {
        WaveElem wave = wavefront.top(); // Get next node
        wavefront.pop();
        Intersection *currNode = wave.node;
        // Check if this was the best path to this node from other times reach this node
        // Add the intersection's street segments that go out
        for (int i = 0; i < currNode->street_segments.size(); i++) {
            // Do not need to account for one way
            int streetSegmentID = currNode->street_segments[i];
            // get opposite side intersection]
            int toIntersectionID = intersectionIdOtherEdgeofStreetSeg(streetSegmentID, currNode->intersectionID);
            Intersection* nextNode = &Global_intersection_nodes[toIntersectionID];

            // Cost 
            double travelTimeFromLastNode = find_street_segment_length(streetSegmentID) / walking_speed;
            if (currNode->walkingReachSSID != -1) {
                if ((getInfoStreetSegment(currNode->walkingReachSSID).streetID != getInfoStreetSegment(streetSegmentID).streetID)) {
                    travelTimeFromLastNode += turn_penalty;
                }

            }

            // push to wavefront if within the walking_time_limit
            double timeToReachNextNode = currNode->walkingBestTime + travelTimeFromLastNode;
            // Check if this is the best way to get to this node
            if (timeToReachNextNode < nextNode->walkingBestTime) {
                // Update path to this node
                nextNode->walkingReachSSID = streetSegmentID;
                nextNode->walkingBestTime = timeToReachNextNode;

                if (timeToReachNextNode <= walking_time_limit) {
                    walkableIntersections.insert(nextNode->intersectionID);
                    wavefront.push(
                            WaveElem(nextNode, streetSegmentID, timeToReachNextNode, timeToReachNextNode));
                }
            }

        }
    }
    // Return set of all walkable intersections no duplicates
    return walkableIntersections;

}

list<StreetSegmentIndex> bfsTraceBack(int destID) {
    // Traceback from destination ID until the previous streetsegment is -1
    // which is init at start of algorithm
    list<StreetSegmentIndex> path;
    Intersection *currNode = &Global_intersection_nodes[destID];
    StreetSegmentIndex prevEdge = currNode->reachingStreetSegID;
    const int NO_EDGE = -1;
    while (prevEdge != NO_EDGE) {
        path.push_front(prevEdge);
        int oppositeIntersectionID = intersectionIdOtherEdgeofStreetSeg(prevEdge, currNode->intersectionID);
        currNode = &Global_intersection_nodes[oppositeIntersectionID];
        prevEdge = currNode->reachingStreetSegID;
    }
    return (path);
}

list<StreetSegmentIndex> bfsTraceBackWalk(int destID) {
    // Traceback from destination ID until the previous streetsegment is -1
    // which is init at start of algorithm
    list<StreetSegmentIndex> path;
    Intersection *currNode = &Global_intersection_nodes[destID];
    StreetSegmentIndex prevEdge = currNode->walkingReachSSID;
    const int NO_EDGE = -1;
    while (prevEdge != NO_EDGE) {

        path.push_front(prevEdge);
        int oppositeIntersectionID = intersectionIdOtherEdgeofStreetSeg(prevEdge, currNode->intersectionID);
        currNode = &Global_intersection_nodes[oppositeIntersectionID];
        prevEdge = currNode->walkingReachSSID;
    }
    return (path);
}

IntersectionIndex intersectionIdOtherEdgeofStreetSeg(StreetSegmentIndex ssid, IntersectionIndex currentIntersectionID) {
    IntersectionIndex from = getInfoStreetSegment(ssid).from;
    IntersectionIndex to = getInfoStreetSegment(ssid).to;
    if (currentIntersectionID == from) {
        return to;
    } else {
        return from;
    }
}
