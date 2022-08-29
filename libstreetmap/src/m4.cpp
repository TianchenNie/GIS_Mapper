/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m4.h"
#include "m3.h"
#include "m2.h"
#include "m1.h"
#include <vector>
#include <set>
#include <list>
#include "PathToOther.h"
#include "PDIntersection.h"
#include <queue>
#include <iterator>
#include <map>
#include "global.h"
#include <chrono>
#include <stdlib.h>
#include <time.h>
using namespace std;
CourierSubpath nearest_legal_pickup_dropoff(int currentPDIndex, int currentPD, list<int>& finishedDeliveries,
        const std::vector<DeliveryInfo>& deliveries, const float truck_capacity, double& currentTruckCapacity, list<int>&deliveryIndexesInProcess, int random);
// This routine takes in a vector of N deliveries (pickUp, dropOff
// intersection pairs), another vector of M intersections that
// are legal start and end points for the path (depots), a turn 
// penalty in seconds (see m3.h for details on turn penalties), 
// and the truck_capacity in pounds.
//
// The first vector 'deliveries' gives the delivery information.  Each delivery
// in this vector has pickUp and dropOff intersection ids and the weight (also
// in pounds) of the delivery item. A delivery can only be dropped-off after
// the associated item has been picked-up. 
// 
// The second vector 'depots' gives the intersection ids of courier company
// depots containing trucks; you start at any one of these depots and end at
// any one of the depots.
//
// This routine returns a vector of CourierSubpath objects that form a delivery route.
// The CourierSubpath is as defined above. The first street segment id in the
// first subpath is connected to a depot intersection, and the last street
// segment id of the last subpath also connects to a depot intersection.  The
// route must traverse all the delivery intersections in an order that allows
// all deliveries to be made with the given truck capacity. Addionally, a package
// should not be dropped off if you haven't picked it up yet.
//
// The start_intersection of each subpath in the returned vector should be 
// at least one of the following (a pick-up and/or drop-off can only happen at 
// the start_intersection of a CourierSubpath object):
//      1- A start depot.
//      2- A pick-up location (and you must specify the indices of the picked 
//                              up orders in pickup_indices)
//      3- A drop-off location. 
//
// You can assume that N is always at least one, M is always at least one
// (i.e. both input vectors are non-empty), and truck_capacity is always greater
// or equal to zero.
//
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages, as long as the
// truck_capcity fits both of them and you properly set your pickup_indices in
// your courierSubpath.  One traversal of an intersection is sufficient to
// drop off all the (already picked up) packages that need to be dropped off at
// that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.
vector<list<PathToOther>> pdAllPaths;
/*[
 0:[//intersectionID:43234
    0:{
       id:23//p0-p3,
       path:{0,1,2,3},
       besttime:100ms
    },
    1:{
       id:145//p0-p1,
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
 1:[///intersectionID:23
    0:{
       id:43234//p0-p3,
       path:{0,1,2,3},
       besttime:100ms
    },
    1:{
       id:145//p0-p1,
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
 2:[//intersectionID:145
    0:{
       id:23//p0-p3,
       path:{0,1,2,3},
       besttime:100ms
    },
    1:{
       id:43234//p0-p1,
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
vector<PDIntersection> pdIntersections;

/*
 [
   0:{
    intersectionID:43234
    deliveriesWithThisIntersection:[DeliveryInfoIndex] //[0,53,21,2]
   },
   1:{
    intersectionID:23
    deliveriesWithThisIntersection:[DeliveryInfoIndex] //[0,1,21,2]
   },
   2:{
    intersectionID:145
    deliveriesWithThisIntersection:[DeliveryInfoIndex] //[0,5,4,2]
   }
]
 */


std::vector<CourierSubpath> traveling_courier(
        const std::vector<DeliveryInfo>& deliveries,
        const std::vector<int>& depots,
        const float turn_penalty,
        const float truck_capacity) {
    auto t1 = chrono::high_resolution_clock::now();
    pdAllPaths.clear();
    pdIntersections.clear();
    for (int i = 0; i < deliveries.size(); i++) {
        int pickupID = deliveries[i].pickUp;
        int dropoffID = deliveries[i].dropOff;
        bool puExists = false;
        bool doExists = false;
        for (int j = 0; j < pdIntersections.size(); j++) {
            if (pdIntersections[j].intersectionID == pickupID) {
                pdIntersections[j].deliveriesWithThisIntersection.push_back(i);
                puExists = true;
            } else if (pdIntersections[j].intersectionID == dropoffID) {
                pdIntersections[j].deliveriesWithThisIntersection.push_back(i);
                doExists = true;
            }
        }
        if (!puExists) {
            pdIntersections.push_back(PDIntersection(pickupID,{i}));
        }
        if (!doExists) {
            pdIntersections.push_back(PDIntersection(dropoffID,{i}));

        }
    }
    pdAllPaths.resize(pdIntersections.size());
    for (int i = 0; i < pdIntersections.size(); i++) {
        setupPDIntersectionPaths(pdIntersections[i].intersectionID, turn_penalty, pdAllPaths[i]);
    }
    //    Connecting_intersections.resize(deliverie)
    double bestTime = 9999999999;
 
    CourierSubpath startPath;
    int start_depot=0;
    for(int it=0;it<depots.size();it++){
        double distance=find_distance_between_two_points(make_pair(getIntersectionPosition(depots[0]), getIntersectionPosition(deliveries[0].pickUp)));
        double compare_distance=find_distance_between_two_points(make_pair(getIntersectionPosition(depots[it]), getIntersectionPosition(deliveries[0].pickUp)));
        if(compare_distance<distance){
            distance=compare_distance;
            start_depot=it;
        }
    }
   
    startPath.start_intersection = depots[start_depot];
    startPath.end_intersection = deliveries[0].pickUp;
    startPath.subpath = find_path_between_intersections(startPath.start_intersection, startPath.end_intersection, turn_penalty);
    bool firstTime = true;
    //    auto t3 = chrono::high_resolution_clock::now();
    //    auto duration3 = chrono::duration_cast<chrono::milliseconds>(t3 - t1).count();
    //    cout << "dfasfas" << duration3 << endl;
    vector<CourierSubpath> FinalPath;
    auto t2 = chrono::high_resolution_clock::now();
    double startTime = compute_path_travel_time(startPath.subpath, turn_penalty);
    while (chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() < 45000) {
        double tempTime = startTime;
        vector<CourierSubpath> Path;
        list<int>finishedDeliveries;
        list<int>deliveryIndexesInProcess;

        Path.push_back(startPath);
        int currentPD = startPath.end_intersection;
        double currentTruckCapacity = 0;
        while (finishedDeliveries.size() != deliveries.size()) {
            int currentPDIndex = -1;
            for (int i = 0; i < pdIntersections.size(); i++) {
                if (pdIntersections[i].intersectionID == currentPD) {
                    currentPDIndex = i;
                }
            }
            if (currentPD == -1) {
                cout << "Ahh" << endl;
                return
                {
                }; // if cannot find a next legal dropoff 
            }
            int random;
            if (firstTime) {

                random = 0;

            } else {
                if(rand()%10==1){
                    random = 1;
                
                }else{
                    random = 0;
                }
//                                cout<<"random"<<random<<endl;

            }
            CourierSubpath subPath = nearest_legal_pickup_dropoff(currentPDIndex, currentPD, finishedDeliveries,
                    deliveries, truck_capacity, currentTruckCapacity, deliveryIndexesInProcess, random);

            Path.push_back(subPath);
            tempTime += compute_path_travel_time(subPath.subpath, turn_penalty);
            currentPD = subPath.end_intersection;
            //            cout<<Path.size()<<',';
        }
            CourierSubpath finalPath;
            int end_depot=0;
             for(int its=0;its<depots.size();its++){
                double distance1=find_distance_between_two_points(make_pair(getIntersectionPosition(currentPD), getIntersectionPosition(depots[0])));
                double compare_distance1=find_distance_between_two_points(make_pair(getIntersectionPosition(currentPD), getIntersectionPosition(depots[its])));
                if(compare_distance1<distance1){
                    distance1=compare_distance1;
                    end_depot=its;
                }
            }
    
            finalPath.end_intersection = depots[end_depot];

    
        finalPath.start_intersection = currentPD;
        finalPath.subpath = find_path_between_intersections(finalPath.start_intersection, finalPath.end_intersection, turn_penalty);
        tempTime += compute_path_travel_time(finalPath.subpath, turn_penalty);
        Path.push_back(finalPath);

        if (firstTime) {
                    cout<<"best"<<bestTime<<endl;
        cout<<"temp"<<tempTime<<endl;
            FinalPath = Path;
            bestTime = tempTime;
            firstTime = false;
        } else {
            
            if (tempTime < bestTime) {
                        cout<<"best"<<bestTime<<endl;
        cout<<"temp"<<tempTime<<endl;
                bestTime = tempTime;
                FinalPath = Path;
            }
        }
        t2 = chrono::high_resolution_clock::now();
//        cout << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << endl;
    }

    return FinalPath;


    //    for(int i = 0; i< deliveries.size() -1; i++)
    //    {
    //        subPath.start_intersection = subPath.end_intersection;
    //        subPath.end_intersection = deliveries[i].dropOff;
    //        subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
    //        subPath.pickUp_indices.push_back(i);
    //        Path.push_back(subPath);
    //        subPath.start_intersection = subPath.end_intersection;
    //        subPath.end_intersection = deliveries[i+1].pickUp;
    //        subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
    //        subPath.pickUp_indices.pop_back();
    //        Path.push_back(subPath);
    //        
    //    }
    //    int end = deliveries.size() -1;
    //    subPath.start_intersection = subPath.end_intersection;
    //    subPath.end_intersection = deliveries[end].dropOff;
    //    subPath.pickUp_indices.push_back(end);
    //    subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
    //    Path.push_back(subPath);
    //    subPath.start_intersection = subPath.end_intersection;
    //    subPath.pickUp_indices.pop_back();
    //    for(int j = 0; j < depots.size(); j++)
    //    {
    //        vector<int> endPath = find_path_between_intersections(subPath.start_intersection, depots[j], turn_penalty);
    //        double travelTime = compute_path_travel_time(endPath, turn_penalty);
    //        if(bestTime > travelTime)
    //        {
    //            bestTime = travelTime;
    //            endDepot = depots[j];
    //        }
    //    }
    //    subPath.end_intersection = endDepot;
    //    subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
    //    Path.push_back(subPath);
    //    return Path;
}
// Return next pd

CourierSubpath nearest_legal_pickup_dropoff(int currentPDIndex, int currentPD, list<int>& finishedDeliveries,
        const std::vector<DeliveryInfo>& deliveries, const float truck_capacity, double& currentTruckCapacity, list<int>&deliveryIndexesInProcess, int random) {


    CourierSubpath finalPath;
    finalPath.start_intersection = currentPD;
    finalPath.end_intersection = -1;
    vector<unsigned int> pickUp_indices;
    // Reached currentPD, update deliveries
    pdIntersections;
    for (int i = 0; i < pdIntersections[currentPDIndex].deliveriesWithThisIntersection.size(); i++) {
        int deliveryIndex = pdIntersections[currentPDIndex].deliveriesWithThisIntersection[i];
        DeliveryInfo delivery = deliveries[deliveryIndex];
        // Only for picking up now
        if (delivery.pickUp != currentPD) {
            continue;
        }
        // Check if this delivery has been finished
        bool finishedDelivery = false;
        std::list<int>::iterator finishedDeliveryi;
        for (finishedDeliveryi = finishedDeliveries.begin(); finishedDeliveryi != finishedDeliveries.end(); finishedDeliveryi++) {
            if (*finishedDeliveryi == deliveryIndex) {
                finishedDelivery = true;
            }
        }
        // pdIntersections[j] is the nextPD
        // If not finished this delivery
        if (!finishedDelivery) {
            if (delivery.pickUp == pdIntersections[currentPDIndex].intersectionID && (currentTruckCapacity + delivery.itemWeight) < truck_capacity) {
                pickUp_indices.push_back(deliveryIndex);
                currentTruckCapacity += delivery.itemWeight;
                deliveryIndexesInProcess.push_back(deliveryIndex);
            }
        }

    }
    // Iterate through the currentPD paths sorted by closest distance/bestTime
//    list<PathToOther> myPath;// =  pdAllPaths[currentPDIndex];
//    std::list<PathToOther>::iterator currentpdPathi = ;
//        for(int i = 0;i<random;i++){
//            currentpdPathi++;
//        }
//    myPath.assign(currentpdPathi, pdAllPaths[currentPDIndex].end());
//    currentpdPathi= pdAllPaths[currentPDIndex].begin();
//        for(int i = 0;i<random ;i++){
//            myPath.push_back(*currentpdPathi);
//            currentpdPathi++;
//        }
//    if(random!=0){
//        
//        std::list<PathToOther>::iterator myPathz = myPath.begin();
//
//        cout<<"myPath"<<endl;
//      for(myPathz; myPathz!= myPath.end();myPathz++){
//          cout<<myPathz->bestTime<<",";
//      } 
//        myPathz = pdAllPaths[currentPDIndex].begin();
//
//        cout<<"original"<<endl;
//      for(myPathz; myPathz!= pdAllPaths[currentPDIndex].end();myPathz++){
//          cout<<myPathz->bestTime<<",";
//      } 
//        
//    }
    list<PathToOther> copy = pdAllPaths[currentPDIndex];
    if(rand()%100<10){
        PathToOther start = *pdAllPaths[currentPDIndex].begin();
        copy.pop_front();
        std::list<PathToOther>::iterator second = copy.begin();
        second++;
        copy.insert(second,start);
    }
    //    copy_n(currentpdPathi,pdAllPaths[currentPDIndex].end(),myPath.begin());
    //    copy_n(pdAllPaths[currentPDIndex].begin(),currentpdPathi--,myPath.end());
    std::list<PathToOther>::iterator myPathi = copy.begin();

    for (myPathi; myPathi != copy.end(); myPathi++) {
        int nextPD = myPathi->otherIntersectionID;
        // Get next pdintersection, which is closest to currentPD
        for (int j = 0; j < pdIntersections.size(); j++) {
            if (nextPD == pdIntersections[j].intersectionID) {
                // Only actually go to this intersection if an action can be completed
                bool goThere = false;
                // Complete all possible pick up and drop offs at next intersection.
                // if at least one pick up or drop off is completed, return this path
                for (int k = 0; k < pdIntersections[j].deliveriesWithThisIntersection.size(); k++) {
                    // Determine if a delivery associated with next intersection is valid
                    int deliveryIndex = pdIntersections[j].deliveriesWithThisIntersection[k];
                    DeliveryInfo delivery = deliveries[deliveryIndex];
                    // Check if this delivery has been finished
                    bool finishedDelivery = false;
                    std::list<int>::iterator finishedDeliveryi;
                    for (finishedDeliveryi = finishedDeliveries.begin(); finishedDeliveryi != finishedDeliveries.end(); finishedDeliveryi++) {
                        if (*finishedDeliveryi == deliveryIndex) {
                            finishedDelivery = true;
                        }
                    }
                    // pdIntersections[j] is the nextPD
                    // If not finished this delivery
                    if (!finishedDelivery) {
                        // If next intersection is a drop off
                        if (delivery.dropOff == pdIntersections[j].intersectionID) {
                            // Check if have package
                            bool havePackage = false;
                            std::list<int>::iterator deliveryOngoingi;
                            for (deliveryOngoingi = deliveryIndexesInProcess.begin(); deliveryOngoingi != deliveryIndexesInProcess.end(); deliveryOngoingi++) {
                                if (*deliveryOngoingi == deliveryIndex) {
                                    havePackage = true;
                                }
                            }
                            // if have package, add to finished deliveries list, remove from deliveries in process
                            if (havePackage) {
                                //                                cout<<"have package"<<endl;
                                finishedDeliveries.push_back(deliveryIndex);
                                deliveryIndexesInProcess.remove(deliveryIndex);
                                currentTruckCapacity -= delivery.itemWeight;
                                goThere = true;

                            } else {
                                //                                cout<<"Dont have package"<<endl;
                            }

                        }// If delivery's pick up is next intersection 
                            // Check if have enough capacity to pick up next intersection's package
                        else if (delivery.pickUp == pdIntersections[j].intersectionID && (currentTruckCapacity + delivery.itemWeight) < truck_capacity) {
                            bool havePackage = false;
                            std::list<int>::iterator deliveryOngoingi;
                            for (deliveryOngoingi = deliveryIndexesInProcess.begin(); deliveryOngoingi != deliveryIndexesInProcess.end(); deliveryOngoingi++) {
                                if (*deliveryOngoingi == deliveryIndex) {
                                    havePackage = true;
                                }
                            }
                            // if have package, add to finished deliveries list, remove from deliveries in process
                            if (!havePackage) {
                                goThere = true;

                            }

                        } else {
                            // Check next   
                        }
                    }
                }
                if (goThere) {
                    finalPath.end_intersection = (nextPD);
                    finalPath.pickUp_indices = (pickUp_indices);
                    finalPath.subpath = myPathi->path;
                    return finalPath;
                }

            }
        }
    }
    return finalPath;
}


//kris implementation 
//std::vector<CourierSubpath> traveling_courier(
//		            const std::vector<DeliveryInfo>& deliveries,
//	       	        const std::vector<int>& depots, 
//		            const float turn_penalty, 
//		            const float truck_capacity)
//{
//    
//    double bestTime = 9999999999;
//    int endDepot;
//    int startPickUp;
//    vector<CourierSubpath> Path;
//    CourierSubpath subPath;
//  
//    int i=0;
//    while(!deliveries.empty()){
//        
//        
//        vector<int> shortPath = find_path_between_intersections(depots[0],deliveries[i].pickUp,turn_penalty);
//        double travelTime=compute_path_travel_time(shortPath, turn_penalty);
//         if(bestTime > travelTime)
//        {
//            bestTime = travelTime;
//            startPickUp=deliveries[i].pickUp;
//        }
//        i++;     
//    }
//   
//    subPath.start_intersection = depots[0];
//    subPath.end_intersection = startPickUp;
//    
//    
//    
//    subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
//    Path.push_back(subPath);
//    for(int i = 0; i< deliveries.size() -1&&i!=startPickUp; i++)
//    {
//        subPath.start_intersection = subPath.end_intersection;
//        subPath.end_intersection = deliveries[i].dropOff;
//        subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
//        subPath.pickUp_indices.push_back(i);
//        Path.push_back(subPath);
//        subPath.start_intersection = subPath.end_intersection;
//        subPath.end_intersection = deliveries[i+1].pickUp;
//        subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
//        subPath.pickUp_indices.pop_back();
//        Path.push_back(subPath);
//        
//    }
//    int end = deliveries.size() -1;
//    subPath.start_intersection = subPath.end_intersection;
//    subPath.end_intersection = deliveries[end].dropOff;
//    subPath.pickUp_indices.push_back(end);
//    subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
//    Path.push_back(subPath);
//    subPath.start_intersection = subPath.end_intersection;
//    subPath.pickUp_indices.pop_back();
//    for(int j = 0; j < depots.size(); j++)
//    {
//        vector<int> endPath = find_path_between_intersections(subPath.start_intersection, depots[j], turn_penalty);
//        double travelTime = compute_path_travel_time(endPath, turn_penalty);
//        if(bestTime > travelTime)
//        {
//            bestTime = travelTime;
//            endDepot = depots[j];
//        }
//    }
//    subPath.end_intersection = endDepot;
//    subPath.subpath = find_path_between_intersections(subPath.start_intersection, subPath.end_intersection, turn_penalty);
//    Path.push_back(subPath);
// return Path;




//}