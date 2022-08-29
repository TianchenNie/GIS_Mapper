/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   courierIntersection.h
 * Author: changry1
 *
 * Created on April 5, 2020, 4:22 PM
 */

/*[
   {//intersectionId
    intersectionID:43234
    pickUp: true,
    dropOff:true,
    packagesHere,
    deliveriesWithThisIntersection:[DeliveryInfoIndex]
    ]
}*/

#ifndef PDINTERSECTION_H
#define PDINTERSECTION_H
#include <vector>
class PDIntersection {
public:
    int intersectionID;
//    bool pickUp;
//    bool dropOff;
    std::vector<int> deliveriesWithThisIntersection;//:[DeliveryInfoIndex]
    PDIntersection();
    PDIntersection(int _intersectionID, std::vector<int> _deliveriesWithThisIntersection);
    virtual ~PDIntersection();
private:

};

#endif /* PDINTERSECTION_H */

