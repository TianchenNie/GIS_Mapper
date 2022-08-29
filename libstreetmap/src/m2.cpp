/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m1.h"
#include "m2.h"
#include <iostream>
#include "math.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "city.h"
#include "helper.h"
#include "global.h"
#include <chrono>
#include "m3.h"
#include <algorithm>
using namespace std;
//Functions. Functionality of functions are commented on top of the 
//function implementation
void draw_main_canvas(ezgl::renderer *g);
void initial_setup(ezgl::application *application, bool /*new_window*/);
void act_on_mouse_click(ezgl::application*app, GdkEventButton*event, double x, double y);
void act_on_mouse_move(ezgl::application*app, GdkEventButton*event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name);
bool drawThisFeatureOrNot(double feature_area, double currentPercentageOfInitialScreen); //Ryan
bool drawThisWayLengthOrNot(double feature_length, double currentPercentageOfInitialScreen); //Ryan
void find_button(GtkWidget */*widget*/, ezgl::application *application); //Kris and Jackson
void find_features(GtkWidget */*widget*/, ezgl::application *application);
void find_POI(GtkWidget */*widget*/, ezgl::application *application);
ezgl::color color_decoder(FeatureType featureType); //Ryan
void drawFeatures(ezgl::renderer *g, double currentPercentageOfInitialScreen); //Ryan
void drawStreets(ezgl::renderer *g, double currentPercentageOfInitialScreen); //Kris
void drawPointOfIntersts(ezgl::renderer *g); //Kris
void drawIntersections(ezgl::renderer *g); //Kris
void subwaystations(GtkWidget */*widget*/, ezgl::application *application);
ezgl::rectangle findMaxRectanglesOfIntersections(vector<int> intersection_ids); //ryan
ezgl::rectangle findMaxRectanglesOfPoints(vector<LatLon> points); //ryan
void drawIntersectionGraphics(ezgl::renderer *g);
void setUpFeatures();
void SetColor(int value);
void drawSubway(ezgl::renderer *g);
string printIntersectionInformation(int intersection_id); //Ryan 
//GtkWidget * gtk_dialog_new_with_buttons(const gchar *title, GtkWindow *parent, GtkDialogFlags flags, const gchar *first_button_text);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void toggleNightMode(GtkWidget */*widget*/, ezgl::application *application);

//Functions needed to implement milestone3 
void drawWalkPath(ezgl::renderer* g);
void walkDrive(GtkWidget */*widget*/, ezgl::application *application);
void routeInformation(GtkWidget */*widget*/, ezgl::application *application);
void UserGuide(GtkWidget */*widget*/, ezgl::application *application);
void Drive(GtkWidget */*widget*/, ezgl::application *application);
void drawIntersectionPointsForPath(ezgl::renderer *g);
void selectFirstIntersection(GtkWidget */*widget*/, ezgl::application *application);
void selectSecondIntersection(GtkWidget */*widget*/, ezgl::application *application);
void driveOnlyClick(GtkWidget */*widget*/, ezgl::application *application);
void donePathFinding(GtkWidget */*widget*/, ezgl::application *application);
void drawDrivePath(ezgl::renderer *g);
//Global Data Structures
vector<feature> Global_topLayerFeaturesList;
vector<feature> Global_lowerLayerFeaturesList;
vector<feature> Global_allFeatures;
std::vector<intersection_data> Global_intersections;
std::vector<IntersectionIndex> Global_from;
std::vector<IntersectionIndex> Global_to;
std::vector<int> Global_number_of_curve_points;
std::vector<string> Global_InterestName;
std::vector<string> Global_streetName;
std::vector<double> Global_fromx;
std::vector<double> Global_fromy;
std::vector<double> Global_tox;
std::vector<double> Global_toy;
std::vector<double> Global_degrees;
std::vector<double> Global_degrees2;

const string blue="\033[1;34m";
const string color2="\033[1;27m";
const string color3="\033[1;25m";
const string color="\033[1;20m";

vector<const OSMNode*> Global_osm_subway_stations;
bool nightMode = false;
vector<int> walkPath;
vector<int>drivePath;
//Global Variables
float Global_lat_avg = 0;
double Global_initialArea;
double Global_currentArea;
double Global_zoom;
double Global_REC_AREA_OF_CITY;
double Global_DIAGONAL_LENGTH_OF_CITY;
ezgl::color lightGrey(53, 53, 53, 255);

int firstIntersection = -1;
int secondIntersection = -1;
int currentlySelecting = 0;
bool pathSearch = true;

void draw_map() {
    //clear data structures so that they do not go out of range when loading
    //other maps
    ezgl::application::settings settings;
    Global_initialArea = 0;
    Global_intersections.clear();
    Global_from.clear();
    Global_to.clear();
    Global_number_of_curve_points.clear();
    Global_InterestName.clear();
    Global_streetName.clear();
    Global_fromx.clear();
    Global_fromy.clear();
    Global_tox .clear();
    Global_toy.clear();
    Global_degrees.clear();
    Global_degrees2.clear();
    Global_osm_subway_stations.clear();
    Global_allFeatures.clear();
    Global_topLayerFeaturesList.clear();
    Global_lowerLayerFeaturesList.clear();
    walkPath.clear();
    drivePath.clear();
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    ezgl::application application(settings);
    setUpFeatures();
    double max_lat = getIntersectionPosition(0).lat();
    double min_lat = max_lat;
    double max_lon = getIntersectionPosition(0).lon();
    double min_lon = max_lon;

    Global_intersections.resize(getNumIntersections());

    for (int id = 0; id < getNumIntersections(); ++id) {
        Global_intersections[id].position = getIntersectionPosition(id);
        Global_intersections[id].name = getIntersectionName(id);

        max_lat = std::max(max_lat, Global_intersections[id].position.lat());
        min_lat = std::min(min_lat, Global_intersections[id].position.lat());
        max_lon = std::max(max_lon, Global_intersections[id].position.lon());
        min_lon = std::min(min_lon, Global_intersections[id].position.lon());

    }

    Global_lat_avg = (max_lat + min_lat) / 2;


    ezgl::rectangle initial_world({x_from_lon(min_lon), min_lat},
    {
        x_from_lon(max_lon), max_lat
    });

    // Ryan: Adding Global Variables
    LatLon l1(x_from_lon(max_lon), max_lat);
    LatLon l2(x_from_lon(min_lon), min_lat);
    pair<double, double> p1 = Convert_Coordinates(l1);
    pair<double, double> p2 = Convert_Coordinates(l2);
    double height = p1.second - p2.second;
    double length = p1.first - p2.first;
    double area = height*length;
    Global_REC_AREA_OF_CITY = area;
    Global_DIAGONAL_LENGTH_OF_CITY = sqrt(pow(height, 2) + pow(length, 2));
    // End of adding global variables

    application.add_canvas("MainCanvas",
            draw_main_canvas,
            initial_world);


    application.run(initial_setup, act_on_mouse_click, act_on_mouse_move, act_on_key_press);
}
//Draw the Main Canvas including street, features, intersections etc.

void draw_main_canvas(ezgl::renderer *g) {
    if (nightMode) {
        cout << "in night mode" << endl;
        g->set_color(lightGrey);
        g->fill_rectangle(g->get_visible_world());
    }

    // Ryan Adding Global variables
    if (!Global_initialArea) {
        Global_initialArea = g->get_visible_world().area();
    } else {
        Global_currentArea = g->get_visible_world().area();
    }
    Global_zoom = Global_currentArea / Global_initialArea * 100;
    cout << "zoom " << Global_zoom << endl;
    // End of adding global variables


    drawFeatures(g, Global_zoom);


    drawStreets(g, Global_zoom);

    drawPointOfIntersts(g);

    drawIntersections(g);






    drawIntersectionGraphics(g);

    if (pathSearch) {
        drawIntersectionPointsForPath(g);
        drawDrivePath(g);
        drawWalkPath(g);
    }
    drawSubway(g);
}
//React to mouse click on the map by printing closest intersection

void act_on_mouse_click(ezgl::application*app, GdkEventButton*event, double x, double y) {
    
    gpointer events = event->axes;
    if(events == event->axes)
    {
        
    }
    if(x == y)
    {
        
    }
    std::cout << "Mouse clicked at (" << x << "," << y << ")" << endl;
    LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
    int id = find_closest_intersection(pos);

    string info = printIntersectionInformation(id);

    cout << info;

        if(pathSearch){
    if (currentlySelecting == 1) {
        firstIntersection = id;
    } else if (currentlySelecting == 2) {
        secondIntersection = id;
    }
        driveOnlyClick(NULL, app);
    }
    
    app->refresh_drawing();



}
//function to print info about the desired intersection

string printIntersectionInformation(int intersection_id) {

    string intersection_name = "Intersection: " + to_string(intersection_id) + " :" + Global_intersections[intersection_id].name + "\n";
    double lon = Global_intersections[intersection_id].position.lon();
    double lat = Global_intersections[intersection_id].position.lat();
    string lat_lon = "\nLocated at (Lat, Lon): " + to_string(lat) + ", " + to_string(lon) + "\n";
    string nearby_intersections = "\nNearby Intersections: \n";
    std::vector<int> adjacent_intersections = find_adjacent_intersections(intersection_id);
    for (int i = 0; i < adjacent_intersections.size(); i++) {
        nearby_intersections += " - " + Global_intersections[adjacent_intersections[i]].name + "\n";
    }
    string nearby_streets = "\nNearby Streets: \n";
    std::vector<string> adjacent_streets_names = find_street_names_of_intersection(intersection_id);
    for (int i = 0; i < adjacent_streets_names.size(); i++) {
        nearby_streets += " - " + adjacent_streets_names[i] + "\n";
    }


    return intersection_name + lat_lon + nearby_intersections + nearby_streets;
}
//This function is called whenever mouse moves 

void act_on_mouse_move(ezgl::application */*application*/, GdkEventButton */*event*/, double x, double y) {
    if(x == y)
    {
        
    }

}

//This function is called whenever key presses 

void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name) {
    application->update_message("Key Pressed");

    std::cout << key_name << " key is pressed" << std::endl;
}
//This function is for initial setup and includes extra buttons

void initial_setup(ezgl::application *application, bool /*new_window*/) {


    application->create_button("Find Intersection", 8, find_button);
    application->create_button("Find Feature", 9, find_features);
    application->create_button("Find Point Of Interest", 10, find_POI);
    application->create_button("Subway Stations", 11, subwaystations);
    application->create_button("Night Mode", 12, toggleNightMode);
    application->create_button("Drive Only", 13, Drive);
    application->create_button("Walk and Drive", 14, walkDrive);
    application->create_button("User Guide", 17, UserGuide);
    application->create_button("Select First Intersection", 15, selectFirstIntersection);
    application->create_button("Select Second Intersection", 16, selectSecondIntersection);
    application->create_button("Toggle Path Search", 17, donePathFinding);


}
//convert longitude to x coordinate

float x_from_lon(float lon) {
    float x = 0;
    //convert  coordinates with radian 

    x = lon * cos(Global_lat_avg * DEGREE_TO_RADIAN);
    return x;
}
//convert latitude to y coordinate

float y_from_lat(float lat) {

    return lat;
}
//convert x coordinate to longitude

float lon_from_x(float x) {
    float lon = 0;
    lon = x / (cos(Global_lat_avg * DEGREE_TO_RADIAN));
    return lon;
}
//convert y coordinate to latitude

float lat_from_y(float y) {

    return y;
}
//enables search of intersection between two streets after clicking the
//'Find' button on the map interface. Also zooms in and highlights the 
//found intersection

void find_button(GtkWidget */*widget*/, ezgl::application *application) {


    // Update the status bar message
    application->update_message("Find Intersections Button Pressed");
    string firststreet;
    bool exit = false;
    string secondstreet;
    bool foundFirst = false;
    bool foundSecond = false;
    cout << ">Please enter name of first street (enter 'exit' to exit): ";
    getline(cin, firststreet);
    firststreet = toLowerAndRemoveWhiteSpace(firststreet);
    if (firststreet != "exit") {
        cout << ">Please enter name of second street (enter 'exit' to exit): ";
        getline(cin, secondstreet);
        secondstreet = toLowerAndRemoveWhiteSpace(secondstreet);
        ;
    }

    if (firststreet == "exit" || secondstreet == "exit") {
        exit = true;
    }
    vector <int> first_matches = find_street_ids_from_partial_street_name(firststreet);
    vector <int> second_matches = find_street_ids_from_partial_street_name(secondstreet);
    //search for matches of user input until one specific match is found.
    while (first_matches.size() != 1 && second_matches.size() != 1 && exit == false) {
        if (first_matches.size() != 1) {
            if (first_matches.size() == 0) {
                cout << ">No match found for your first input, please try again." << endl;

            }
            if (first_matches.size() > 1) {
                cout << ">Matches found for your first input: " << endl;
                for (int i = 0; i < first_matches.size(); i++) {
                    cout << getStreetName(first_matches[i]) << endl;


                }

                cout << ">Please specify desired street." << endl;
            }
        } else {
            foundFirst = true;
        }

        if (second_matches.size() != 1) {



            if (second_matches.size() == 0) {
                cout << ">No match found for your second input, please try again." << endl;

            }

            if (second_matches.size() > 1) {
                cout << ">Matches found for your second input: " << endl;
                for (int i = 0; i < second_matches.size(); i++) {
                    cout << getStreetName(second_matches[i]) << endl;

                }

                cout << ">Please specify desired street." << endl;
            }
        } else {
            foundSecond = true;
        }
        if (foundFirst == false && exit == false) {
            cout << ">Please enter name of first street (enter 'exit' to exit): ";
            getline(cin, firststreet);
            firststreet = toLowerAndRemoveWhiteSpace(firststreet);
            if (firststreet == "exit") {
                exit = true;
            } else {
                first_matches.resize(find_street_ids_from_partial_street_name(firststreet).size());
                first_matches = find_street_ids_from_partial_street_name(firststreet);
            }
        } else if (foundFirst == true && exit == false) {
            cout << ">Found match for first street: " << first_matches[0] << endl;

        }
        if (foundSecond == false && exit == false) {

            cout << ">Please enter name of second street (enter 'exit' to exit): ";
            getline(cin, secondstreet);
            secondstreet = toLowerAndRemoveWhiteSpace(secondstreet);

            if (secondstreet == "exit") {
                exit = true;
            } else {
                second_matches.resize(find_street_ids_from_partial_street_name(secondstreet).size());
                second_matches = find_street_ids_from_partial_street_name(secondstreet);
            }
        } else if (foundSecond == true && exit == false) {
            cout << ">Found match for second street: " << second_matches[0] << endl;
        }

    }
    //find the intersection between the two found streets and highlight it
    if (!exit) {
        pair<int, int> streets;
         streets.first = first_matches[0];
        streets.second = second_matches[0];
        vector <int> intersectionsBetween = find_intersections_of_two_streets(streets);
        if (intersectionsBetween.size() == 0) {
            cout << ">There are no intersections between " << getStreetName(first_matches[0]) << " and " << getStreetName(second_matches[0]) << endl;
        } else if (intersectionsBetween.size() == 1) {
            cout << ">The intersection between " << getStreetName(first_matches[0]) << " and " << getStreetName(second_matches[0]) << "is: " << intersectionsBetween[0] << endl;
            Global_intersections[intersectionsBetween[0]].highlight = true;

        } else {
            cout << ">The intersections between " << getStreetName(first_matches[0]) << " and " << getStreetName(second_matches[0]) << " are: ";
            for (int j = 0; j < intersectionsBetween.size(); j++) {
                cout << getIntersectionName(intersectionsBetween[j]) << endl;
                Global_intersections[intersectionsBetween[j]].highlight = true;
            }




            // Redraw the main canvas 
        }
        if (intersectionsBetween.size() != 0) {
            ezgl::renderer* g = application->get_renderer();
            ezgl::rectangle rec = findMaxRectanglesOfIntersections(intersectionsBetween);
            g->set_visible_world(rec);
            application->refresh_drawing();
        }
    } else {
        application->refresh_drawing();
    }
}

void find_features(GtkWidget */*widget*/, ezgl::application *application) {
    application->update_message("Find Features Button Pressed");
    string featureName;
    string originalName;
    int featureNum=-1;
    bool found = false;
    cout << "Please enter the name of the feature you would like to find (enter 'exit' to exit): ";
    getline(cin, featureName);
    originalName = featureName;
    featureName = toLowerAndRemoveWhiteSpace(featureName);
    if (featureName != "exit") {
        while (found == false && featureName != "exit") {
            Global_allFeatures = Global_lowerLayerFeaturesList;
            Global_allFeatures.insert(Global_allFeatures.end(), Global_topLayerFeaturesList.begin(), Global_topLayerFeaturesList.end());
            for (int i = 0; i < Global_allFeatures.size(); i++) {
                if (featureName == toLowerAndRemoveWhiteSpace(Global_allFeatures[i].fName)) {
                    cout << "Found feature, zooming map to feature.. " << endl;
                    featureNum = Global_allFeatures[i].featureID;
                    found = true;
                }
                if (found == true)
                    break;
            }
            if (found == false) {
                cout << "Could not find feature with name: " << originalName << ". Please try again." << endl;
                cout << "Please enter the name of the feature you would like to find (enter 'exit' to exit): ";
                getline(cin, featureName);
                originalName = featureName;
                featureName = toLowerAndRemoveWhiteSpace(featureName);
            }
        }

    }
    if (featureName == "exit") {
        application->refresh_drawing();
    } else {
        vector<LatLon> points;
        for (int pntInd = 0; pntInd < getFeaturePointCount(featureNum); pntInd++) {

            LatLon pointCoord = getFeaturePoint(pntInd, featureNum);
            points.push_back(pointCoord);

        }
        ezgl::renderer* g = application->get_renderer();
        ezgl::rectangle rec = findMaxRectanglesOfPoints(points);
        g->set_visible_world(rec);
        application->refresh_drawing();
    }

}



void drawIntersectionPointsForPath(ezgl::renderer *g) {
    if (firstIntersection != -1) {
        LatLon pos = Global_intersections[firstIntersection].position;
        ezgl::point2d point(x_from_lon(pos.lon()), pos.lat());
        ezgl::surface *png_surface = ezgl::renderer::load_png("depa.png");
        g->draw_surface(png_surface, point);
        ezgl::renderer::free_surface(png_surface);
    }
    if (secondIntersection != -1) {
        LatLon pos = Global_intersections[secondIntersection].position;
        ezgl::point2d point(x_from_lon(pos.lon()), pos.lat());
        ezgl::surface *png_surface = ezgl::renderer::load_png("dest.png");
        g->draw_surface(png_surface, point);
        ezgl::renderer::free_surface(png_surface);
    }
    

}

/*
 Go through every feature and determine whether it belongs in top or bottom layer
 * Top Layer: Buildings, Non-Polygons (points and ways)
 * Bottom Layer: Polygons that aren't buildings such as greenspace
 */
void setUpFeatures() {
    for (FeatureIndex feature_id = 0; feature_id < getNumFeatures(); feature_id++) {
        int numPointsInFeature = getFeaturePointCount(feature_id);
        // Check if feature is a point
        bool isPoint = false;
        bool isPolygon = false;

        if (numPointsInFeature == 1) {
            isPoint = true;
        } else {
            // Check if feature is a polygon
            LatLon first_point = getFeaturePoint(0, feature_id); //Get the first point of the feature area
            LatLon last_point = getFeaturePoint(numPointsInFeature - 1, feature_id); //Get the last point of the feature area 
            if (last_point.lat() == first_point.lat() && last_point.lon() == first_point.lon()) {
                // Only more than three points is a polygon
                if (numPointsInFeature > 3) {
                    isPolygon = true;
                } else {
                    isPolygon = false;
                }
            } else {
                isPolygon = false;
            }
        }
        // Push to data structures
        if (isPolygon) {
            if (getFeatureType(feature_id) == FeatureType(Building)) {
                feature newFeature;
                newFeature.featureID = feature_id;
                newFeature.isPolyLineOrPoint = 1;
                newFeature.fName = getFeatureName(feature_id);
                newFeature.featureType = getFeatureType(feature_id);
                newFeature.wayArea = find_feature_area(feature_id);
                newFeature.color = color_decoder(newFeature.featureType);
                Global_topLayerFeaturesList.push_back(newFeature);
            } else {
                feature newFeature;
                newFeature.featureID = feature_id;
                newFeature.isPolyLineOrPoint = 1;
                newFeature.fName = getFeatureName(feature_id);
                newFeature.featureType = getFeatureType(feature_id);
                newFeature.wayArea = find_feature_area(feature_id);
                newFeature.color = color_decoder(newFeature.featureType);

                Global_lowerLayerFeaturesList.push_back(newFeature);
            }
        } else if (!isPolygon) {
            feature newFeature;

            newFeature.featureID = feature_id;

            newFeature.isPolyLineOrPoint = 2;
            newFeature.fName = getFeatureName(feature_id);
            newFeature.featureType = getFeatureType(feature_id);
            newFeature.wayLength = find_way_length(getFeatureOSMID(feature_id));
            newFeature.color = color_decoder(newFeature.featureType);

            Global_topLayerFeaturesList.push_back(newFeature);

        } else if (isPoint) {
            feature newFeature;
            newFeature.featureID = feature_id;

            LatLon pointCoord = getFeaturePoint(0, feature_id);
            Global_topLayerFeaturesList[feature_id].points.push_back({x_from_lon(pointCoord.lon()), pointCoord.lat()});
            newFeature.isPolyLineOrPoint = 3;
            newFeature.fName = getFeatureName(feature_id);
            newFeature.featureType = getFeatureType(feature_id);
            newFeature.color = color_decoder(newFeature.featureType);

            Global_topLayerFeaturesList.push_back(newFeature);
        }

    }
    // Sort lower so that only the biggest one's are on the bottom, sorted by
    // way area
    sort(Global_lowerLayerFeaturesList.begin(), Global_lowerLayerFeaturesList.end(), Global_lowerLayerFeaturesList[0]);
}

/*
 Draw the bottom layer first and then the top layer after
 */
void drawFeatures(ezgl::renderer *g, double currentPercentageOfInitialScreen) {

    // Bottom layer
    for (FeatureIndex i = 0; i < Global_lowerLayerFeaturesList.size(); i++) {
        if (Global_lowerLayerFeaturesList[i].wayArea == 0) {

        } else {
            // Check if should draw based on zoom
            if (drawThisFeatureOrNot(Global_lowerLayerFeaturesList[i].wayArea, currentPercentageOfInitialScreen)) {
                int numFeaturePoints = getFeaturePointCount(Global_lowerLayerFeaturesList[i].featureID);

                vector<ezgl::point2d> points;
                vector<LatLon> latLonPoints;
                // Add points 
                for (int pntInd = 0; pntInd < numFeaturePoints; pntInd++) {
                    LatLon pointCoord = getFeaturePoint(pntInd, Global_lowerLayerFeaturesList[i].featureID);
                    latLonPoints.push_back(getFeaturePoint(pntInd, Global_lowerLayerFeaturesList[i].featureID));
                    points.push_back({x_from_lon(pointCoord.lon()), pointCoord.lat()});
                }
                // Draw text
                if (Global_lowerLayerFeaturesList[i].fName != "<noname>") {
                    // Add formula to ensure drawing text based on zoom
                    double value = 0.00000001 * Global_lowerLayerFeaturesList[i].wayArea*currentPercentageOfInitialScreen;
                    if (value > 1) {
                        value = 1;
                    } else if (value < 0.005) {
                        value = 0.005;
                    }
                    // Draw polygon
                    g->set_color(Global_lowerLayerFeaturesList[i].color);
                    g->fill_poly(points);
                    g->set_color(0, 0, 0, 255);
                    if (nightMode) {
                        g->set_color(255, 255, 255, 255);
                    }
                    ezgl::rectangle rec = findMaxRectanglesOfPoints(latLonPoints);
                    g->draw_text(rec.center(), Global_lowerLayerFeaturesList[i].fName, value, value);

                } else {
                    // Draw polygon
                    g->set_color(Global_lowerLayerFeaturesList[i].color);
                    g->fill_poly(points);
                }

            }
        }

    }

    for (FeatureIndex i = 0; i < Global_topLayerFeaturesList.size(); i++) {
        // Draw Buildings
        if (Global_topLayerFeaturesList[i].isPolyLineOrPoint == 1) {
            // Check if should draw based on zoom
            if (drawThisFeatureOrNot(Global_topLayerFeaturesList[i].wayArea, currentPercentageOfInitialScreen)) {
                int numFeaturePoints = getFeaturePointCount(Global_topLayerFeaturesList[i].featureID);

                vector<ezgl::point2d> points;
                vector<LatLon> latLonPoints;
                for (int pntInd = 0; pntInd < numFeaturePoints; pntInd++) {
                    LatLon pointCoord = getFeaturePoint(pntInd, Global_topLayerFeaturesList[i].featureID);
                    latLonPoints.push_back(getFeaturePoint(pntInd, Global_topLayerFeaturesList[i].featureID));
                    points.push_back({x_from_lon(pointCoord.lon()), pointCoord.lat()});
                }
                // Draw Text
                if (Global_topLayerFeaturesList[i].fName != "<noname>") {
                    // Add formula to ensure drawing text based on zoom
                    double value = 0.00000001 * Global_topLayerFeaturesList[i].wayArea*currentPercentageOfInitialScreen;
                    if (value > .1) {
                        value = .1;
                    } else if (value < 0.0005) {
                        value = 0.0005;
                    }
                    g->set_color(Global_topLayerFeaturesList[i].color);
                    g->fill_poly(points);
                    g->set_color(0, 0, 0, 255);
                    if (nightMode) {
                        g->set_color(255, 255, 255, 255);
                    }
                    ezgl::rectangle rec = findMaxRectanglesOfPoints(latLonPoints);
                    g->draw_text(rec.center(), Global_topLayerFeaturesList[i].fName, value, value);

                } else {
                    g->set_color(Global_topLayerFeaturesList[i].color);
                    g->fill_poly(points);
                }
            }
        } else {
            if (Global_topLayerFeaturesList[i].isPolyLineOrPoint == 3) {
                // Points draw rectangle around it
                LatLon pointCoord = getFeaturePoint(0, Global_topLayerFeaturesList[i].featureID);
                g->set_color(Global_topLayerFeaturesList[i].color);
                g->draw_rectangle({x_from_lon(pointCoord.lon()), pointCoord.lat()}, 0.0002, 0.0002);
            } else if (Global_topLayerFeaturesList[i].isPolyLineOrPoint == 2) {
                // Draw ways
                // Check if should draw based on zoom

                if (drawThisWayLengthOrNot(Global_topLayerFeaturesList[i].wayLength, currentPercentageOfInitialScreen)) {
                    // Draw all line in way length
                    int numFeaturePoints = getFeaturePointCount(Global_topLayerFeaturesList[i].featureID);
                    for (int pntInd = 0; pntInd < getFeaturePointCount(Global_topLayerFeaturesList[i].featureID) - 1; pntInd++) {
                        LatLon pointCoord = getFeaturePoint(pntInd, Global_topLayerFeaturesList[i].featureID);
                        LatLon nextPointCoord = getFeaturePoint(pntInd + 1, Global_topLayerFeaturesList[i].featureID);
                        g->set_color(Global_topLayerFeaturesList[i].color);
                        g->draw_line({x_from_lon(pointCoord.lon()), pointCoord.lat()},
                        {
                            x_from_lon(nextPointCoord.lon()), nextPointCoord.lat()
                        });

                    }
                    if (Global_topLayerFeaturesList[i].fName != "<noname>") {
                        // Determine center point of lineto draw text
                        LatLon centerPoint;
                        if (numFeaturePoints % 2 == 0) {
                            LatLon p1 = getFeaturePoint(numFeaturePoints / 2 - 1, Global_topLayerFeaturesList[i].featureID);
                            LatLon p2 = getFeaturePoint(numFeaturePoints / 2, Global_topLayerFeaturesList[i].featureID);
                            centerPoint = LatLon(p2.lon() - p1.lon(), p2.lat() - p1.lat());
                        } else {
                            centerPoint = getFeaturePoint(numFeaturePoints / 2, Global_topLayerFeaturesList[i].featureID);
                        }
                        ezgl::point2d centerXY(x_from_lon(centerPoint.lon()), centerPoint.lat());
                        // Add formula to ensure drawing text based on zoom
                        double value = 0.00000001 * Global_topLayerFeaturesList[i].wayLength*currentPercentageOfInitialScreen;
                        if (value > 1) {
                            value = 1;
                        } else if (value < 0.005) {
                            value = 0.005;
                        }
                        g->set_color(ezgl::BLACK);
                        if (nightMode) {
                            g->set_color(255, 255, 255, 255);
                        }
                        g->draw_text(centerXY, Global_topLayerFeaturesList[i].fName, value, value);
                    }
                }
            } else {

            }



        }
    }

}
// Determine whether the feature's area is big enough to display based on the zoom

bool drawThisFeatureOrNot(double feature_area, double currentPercentageOfInitialScreen) {
    double feature_area_percentage_of_city = 100 * feature_area / Global_REC_AREA_OF_CITY;
    if (currentPercentageOfInitialScreen <= .25) {
        //<1%
        return true;
    } else if (currentPercentageOfInitialScreen <= .5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.00005) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 2.5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.00015) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.00025) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 10) {
        // 5-10
        if (feature_area_percentage_of_city > .001) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 25) {
        //10-25
        if (feature_area_percentage_of_city > .01) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 50) {
        //25-50
        if (feature_area_percentage_of_city > .025) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 75) {
        //50-75
        if (feature_area_percentage_of_city > .04) {
            return true;
        }
    } else {
        //75+
        if (feature_area_percentage_of_city > .05) {
            return true;
        }
    }
    return false;

}
// determine a rectangle around several points of LatLon

ezgl::rectangle findMaxRectanglesOfPoints(vector<LatLon> points) {
    double max_lat = points[0].lat();
    double min_lat = max_lat;
    double max_lon = points[0].lon();
    double min_lon = max_lon;
    // if size is 1 create a rectangle around it
    if (points.size() == 1) {
        const double REC_LENGTH = .002;
        ezgl::point2d bottomLeft(x_from_lon(min_lon - REC_LENGTH), min_lat - REC_LENGTH);
        ezgl::point2d topRight(x_from_lon(min_lon + REC_LENGTH), max_lat + REC_LENGTH);
        ezgl::rectangle rec(bottomLeft, topRight);
        return rec;
    }
    for (int id = 0; id < points.size(); ++id) {
        max_lat = std::max(max_lat, points[id].lat());
        min_lat = std::min(min_lat, points[id].lat());
        max_lon = std::max(max_lon, points[id].lon());
        min_lon = std::min(min_lon, points[id].lon());

    }
    ezgl::point2d bottomLeft(x_from_lon(min_lon), min_lat);
    ezgl::point2d topRight(x_from_lon(max_lon), max_lat);
    ezgl::rectangle rec(bottomLeft, topRight);

    return rec;


}
// Determine whether the feature's distance is big enough to display based on the zoom

bool drawThisWayLengthOrNot(double feature_length, double currentPercentageOfInitialScreen) {
    double feature_area_percentage_of_city = 100 * feature_length / Global_DIAGONAL_LENGTH_OF_CITY;
    if (currentPercentageOfInitialScreen <= .5) {
        //<1%
        return true;
    } else if (currentPercentageOfInitialScreen <= 1) {
        //1-5  
        if (feature_area_percentage_of_city > 0.1) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 1.5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.01) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 2.5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.1) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 5) {
        //1-5  
        if (feature_area_percentage_of_city > 0.25) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 10) {
        // 5-10
        if (feature_area_percentage_of_city > .5) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 25) {
        //10-25
        if (feature_area_percentage_of_city > 1.5) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 50) {
        //25-50
        if (feature_area_percentage_of_city > 2.0) {
            return true;
        }
    } else if (currentPercentageOfInitialScreen <= 75) {
        //50-75
        if (feature_area_percentage_of_city > 4.5) {
            return true;
        }
    } else {
        //75+
        if (feature_area_percentage_of_city > 5) {
            return true;
        }
    }
    return false;
}
void drawWalkPath(ezgl::renderer *g)

{
    
 
    if(walkPath.size() != 0)
    {
        
        int a=walkPath.size()-1;
        
         ezgl::point2d point(x_from_lon(getIntersectionPosition(getInfoStreetSegment((walkPath[0])).from).lon()), getIntersectionPosition(getInfoStreetSegment(walkPath[0]).from).lat());
         ezgl::surface *png_surface = ezgl::renderer::load_png("depa.png");
         g->draw_surface(png_surface, point);
         
         ezgl::point2d point2(x_from_lon(getIntersectionPosition(getInfoStreetSegment((walkPath[a])).to).lon()), getIntersectionPosition(getInfoStreetSegment(walkPath[a]).to).lat());
         ezgl::surface *png_surface2 = ezgl::renderer::load_png("dest.png");
         g->draw_surface(png_surface2, point2);
         
        for(int i = 0; i < walkPath.size(); i++)
        {
            
            int current = walkPath[i];
                g->set_color(ezgl::GREEN);
                g->set_line_width(3);
            if(Global_number_of_curve_points[current] == 0)
            {
                g->draw_line({x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()
            });
            }
            else if(Global_number_of_curve_points[current] == 1)
            {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0,current).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
            });
            g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()
            });
            }else {
            //corner case when there is a street that starts and ends in the exactly same point 
            if (getIntersectionPosition(Global_from[current]).lat() == getIntersectionPosition(Global_to[current]).lat() && getIntersectionPosition(Global_from[current]).lon() == getIntersectionPosition(Global_to[current]).lon()) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });

                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });
            }//If not the corner case connect from and to to the first and last curve point and connect all together 
            else {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()

                });
            }
            //connect the curve points from 0 to number of curve points of the segment -1
            for (int j = 0; j < Global_number_of_curve_points[current] - 1; ++j) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(j, current).lon()), getStreetSegmentCurvePoint(j, current).lat()},
                {

                    x_from_lon(getStreetSegmentCurvePoint(j + 1, current).lon()), getStreetSegmentCurvePoint(j + 1, current).lat()
                });
            }
        }
    }
}
}
//This function is to draw streets and streets names 

void drawStreets(ezgl::renderer *g, double currentPercentageOfInitialScreen) {

    //use loop to store streets from and to into vectors 
    for (int i = 0; i < getNumStreetSegments(); ++i) {
        Global_streetName.push_back(getStreetName(getInfoStreetSegment(i).streetID));
        Global_from.push_back(getInfoStreetSegment(i).from); //get the from street point 
        Global_to.push_back(getInfoStreetSegment(i).to); //get the to street point 
        Global_number_of_curve_points.push_back(getInfoStreetSegment(i).curvePointCount); //get the curve point   
        //If the roads are one way then highlight colours 
        if (getInfoStreetSegment(i).oneWay == true) {
            if ((getIntersectionPosition(getInfoStreetSegment(i).from).lat()) > getIntersectionPosition(getInfoStreetSegment(i).to).lat()) {

                g->set_color(ezgl::ORANGE, 255 * 2 / 3);



            }
        }
        bool drawIt = false;
        if (getInfoStreetSegment(i).speedLimit >= 80) {
            g->set_color(ezgl::color(255, 140, 0, 255));
            drawIt = true;
        } else {
            g->set_color(ezgl::BLACK);
            if (nightMode) {
                g->set_color(255, 255, 255, 255);
            }
            if (drawThisWayLengthOrNot(find_street_segment_length(i), currentPercentageOfInitialScreen)) {
                drawIt = true;
            }
        }
        if (!drawIt) {
            continue;
        }
        //if there a staight line just connect two end points together 
        if (Global_number_of_curve_points[i] == 0) {
            //            g->set_color(ezgl::DARK_GREEN);
            g->draw_line({x_from_lon(getIntersectionPosition(Global_from[i]).lon()), getIntersectionPosition(Global_from[i]).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[i]).lon()), getIntersectionPosition(Global_to[i]).lat()
            });

        }//if there is only one curve point,connect from and the curve point and to then connect all together 
        else if (Global_number_of_curve_points[i] == 1) {
            g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, i).lon()), getStreetSegmentCurvePoint(0, i).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_from[i]).lon()), getIntersectionPosition(Global_from[i]).lat()
            });
            g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, i).lon()), getStreetSegmentCurvePoint(0, i).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[i]).lon()), getIntersectionPosition(Global_to[i]).lat()
            });
        }//if there are more than one curve point,need to loop through then connecting all of the points together 
        else {
            //corner case when there is a street that starts and ends in the exactly same point 
            if (getIntersectionPosition(Global_from[i]).lat() == getIntersectionPosition(Global_to[i]).lat() && getIntersectionPosition(Global_from[i]).lon() == getIntersectionPosition(Global_to[i]).lon()) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[i] - 1, i).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[i] - 1, i).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[i]).lon()), getIntersectionPosition(Global_from[i]).lat()
                });

                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, i).lon()), getStreetSegmentCurvePoint(0, i).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[i]).lon()), getIntersectionPosition(Global_from[i]).lat()
                });
            }//If not the corner case connect from and to to the first and last curve point and connect all together 
            else {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, i).lon()), getStreetSegmentCurvePoint(0, i).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[i]).lon()), getIntersectionPosition(Global_from[i]).lat()
                });
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[i] - 1, i).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[i] - 1, i).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_to[i]).lon()), getIntersectionPosition(Global_to[i]).lat()

                });
            }
            //connect the curve points from 0 to number of curve points of the segment -1
            for (int j = 0; j < Global_number_of_curve_points[i] - 1; ++j) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(j, i).lon()), getStreetSegmentCurvePoint(j, i).lat()},
                {

                    x_from_lon(getStreetSegmentCurvePoint(j + 1, i).lon()), getStreetSegmentCurvePoint(j + 1, i).lat()
                });
            }
        }
    }
    //use loop to store streets from and to into vectors 
    for (int i = 0; i < getNumStreetSegments(); ++i) {

        Global_fromx.push_back(x_from_lon(getIntersectionPosition(Global_from[i]).lon()));
        Global_tox.push_back(x_from_lon(getIntersectionPosition(Global_to[i]).lon()));
        Global_fromy.push_back(getIntersectionPosition(Global_from[i]).lat());
        Global_toy.push_back(getIntersectionPosition(Global_to[i]).lat());
    
        
    }
    for (int i = 0; i < getNumStreetSegments(); ++i) {
        
        if (Global_streetName[i] != "<unknown>") //do not draw the unknown roads on the screen 
        {
            if (getInfoStreetSegment(i).oneWay == true) { //if it is one way make the texting red 
        
                g->set_color(ezgl::BLACK);
                
                //add arrows to indicate the directions of the one way road 
                if(Global_fromx[i]<=Global_tox[i]){
                        g->set_text_rotation(atan2((Global_toy[i] - Global_fromy[i]),(Global_tox[i] - Global_fromx[i])) * (180 / 3.14159265358979));
                        g->draw_text({(x_from_lon(getIntersectionPosition(Global_from[i]).lon()) + x_from_lon(getIntersectionPosition(Global_to[i]).lon())) / 2,
                    (getIntersectionPosition(Global_from[i]).lat() + getIntersectionPosition(Global_to[i]).lat()) / 2}, Global_streetName[i] + "--> --> -->",
                        0.00043, 0.00043);
                }
                 else {
                        g->set_text_rotation(atan2((Global_fromy[i] - Global_toy[i]) , (Global_fromx[i] - Global_tox[i])) * (180 / 3.14159265358979));
                 
                     g->draw_text({(x_from_lon(getIntersectionPosition(Global_from[i]).lon()) + x_from_lon(getIntersectionPosition(Global_to[i]).lon())) / 2,
                    (getIntersectionPosition(Global_from[i]).lat() + getIntersectionPosition(Global_to[i]).lat()) / 2}, "< --< --<--"+Global_streetName[i] ,
                        0.00043, 0.00043);
                }
            }
      //if the street segment is not one way then just draw the street names out without putting the arrows 
            else {
                if(Global_fromx[i]<=Global_tox[i]){
                        g->set_text_rotation(atan2((Global_toy[i] - Global_fromy[i]),(Global_tox[i] - Global_fromx[i])) * (180 / 3.14159265358979));
                }
                 else{
                        g->set_text_rotation(atan2((Global_fromy[i] - Global_toy[i]) , (Global_fromx[i] - Global_tox[i])) * (180 / 3.14159265358979));
                    }
                g->draw_text({(x_from_lon(getIntersectionPosition(Global_from[i]).lon()) + x_from_lon(getIntersectionPosition(Global_to[i]).lon())) / 2,
                    (getIntersectionPosition(Global_from[i]).lat() + getIntersectionPosition(Global_to[i]).lat()) / 2}, Global_streetName[i],
                        0.00043, 0.00043);
            }
        }
    }
}


void drawPointOfIntersts(ezgl::renderer *g) {

    for (int i = 0; i < getNumPointsOfInterest(); ++i) {
        float radius = 0.000003;
        g->fill_arc({x_from_lon(getPointOfInterestPosition(i).lon()), getPointOfInterestPosition(i).lat()}, radius, 0, 360);
        Global_InterestName.push_back(getPointOfInterestName(i));
    }
    //store all of the names into vector to increase speed 
    //drawing point of interest names out 
    for (int i = 0; i < getNumPointsOfInterest(); ++i) {
        if (Global_InterestName[i] != "<unknown>") {
            g->set_text_rotation(0);
            g->set_color(ezgl::FIRE_BRICK);
            g->draw_text({x_from_lon(getPointOfInterestPosition(i).lon()), getPointOfInterestPosition(i).lat()}, Global_InterestName[i], 0.0005, 0.0005);
        }
    }
}
//Draw all intersections onto the map

void drawIntersections(ezgl::renderer *g) {

    for (int i = 0; i < getNumIntersections(); ++i) {
        g->set_color(ezgl::SADDLE_BROWN); //setting the intersections name to be saddle brown 
        if (Global_intersections[i].name != "<unknown>") //only showing the intersections which are not unkown 
            g->draw_text({x_from_lon(getIntersectionPosition(i).lon()), getIntersectionPosition(i).lat()}, Global_intersections[i].name, 0.0002, 0.0002);
    }
}
//Draw all one way roads including the directions

void drawIntersectionGraphics(ezgl::renderer *g) {
    for (size_t i = 0; i < Global_intersections.size(); ++i) {
        float x = x_from_lon(Global_intersections[i].position.lon());
        float y = y_from_lat(Global_intersections[i].position.lat());
        float width = 0.000002;
        float height = width;

        if (Global_intersections[i].highlight) {
            g->set_color(ezgl::GREEN);
        } else {
            g->set_color(ezgl::PINK);
        }

        g->fill_rectangle({x - width, y - height},
        {
            x + width, y + height
        });

    }
}

ezgl::rectangle findMaxRectanglesOfIntersections(vector<int> intersection_ids) {

    double max_lat = getIntersectionPosition(intersection_ids[0]).lat();
    double min_lat = max_lat;
    double max_lon = getIntersectionPosition(intersection_ids[0]).lon();
    double min_lon = max_lon;
    if (intersection_ids.size() == 1) {
        const double REC_LENGTH = 0.002;
        ezgl::point2d bottomLeft(x_from_lon(min_lon - REC_LENGTH), min_lat - REC_LENGTH);
        ezgl::point2d topRight(x_from_lon(min_lon + REC_LENGTH), max_lat + REC_LENGTH);
        ezgl::rectangle rec(bottomLeft, topRight);
        return rec;
    }
    for (int id = 0; id < intersection_ids.size(); ++id) {
        max_lat = std::max(max_lat, Global_intersections[intersection_ids[id]].position.lat());
        min_lat = std::min(min_lat, Global_intersections[intersection_ids[id]].position.lat());
        max_lon = std::max(max_lon, Global_intersections[intersection_ids[id]].position.lon());
        min_lon = std::min(min_lon, Global_intersections[intersection_ids[id]].position.lon());

    }
    ezgl::point2d bottomLeft(x_from_lon(min_lon), min_lat);
    ezgl::point2d topRight(x_from_lon(max_lon), max_lat);
    ezgl::rectangle rec(bottomLeft, topRight);

    return rec;


}

//GtkWidget * gtk_dialog_new_with_buttons(const gchar *title, GtkWindow *parent, GtkDialogFlags flags, const gchar *first_button_text) {
//
//
//}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    gpointer user = user_data;
    if(user == user_data)
    {
        
    }
    std::cout << "response is ";
    switch (response_id) {
        case GTK_RESPONSE_ACCEPT:
            std::cout << "Finding button ";
            break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ’X’ button) ";
            break;
        case GTK_RESPONSE_REJECT:
            std::cout << "GTK_RESPONSE_REJECT ";
            break;
        default:
            std::cout << "UNKNOWN ";
            break;
    }

    std::cout << "\n";

    gtk_widget_destroy(GTK_WIDGET(dialog));

}
//assigns different colors to different features

ezgl::color color_decoder(FeatureType featureType) {
    if (featureType == FeatureType(Unknown)) {
        return ezgl::color(0, 0, 205, 100);
    } else if (featureType == FeatureType(Park)) {
        return ezgl::color(144, 238, 144, 100);

    } else if (featureType == FeatureType(Beach)) {
        return ezgl::color(255, 242, 175, 100);

    } else if (featureType == FeatureType(Lake)) {
        return ezgl::color(135, 206, 235, 100);

    } else if (featureType == FeatureType(River)) {
        return ezgl::color(135, 206, 235, 100);

    } else if (featureType == FeatureType(Island)) {
        return ezgl::color(144, 238, 144, 100);

    } else if (featureType == FeatureType(Building)) {
        return ezgl::color(192, 192, 192, 100);

    } else if (featureType == FeatureType(Greenspace)) {
        return ezgl::color(144, 238, 144, 100);

    } else if (featureType == FeatureType(Golfcourse)) {
        return ezgl::color(144, 238, 144, 100);

    } else if (featureType == FeatureType(Stream)) {
        return ezgl::color(135, 206, 235, 150);

    } else {
        cout << "Critical Error" << endl;
        return ezgl::color(0,0,0,0);
    }


}

void toggleNightMode(GtkWidget */*widget*/, ezgl::application *application) {

    ezgl::renderer *g = application->get_renderer();
    ezgl::renderer *h = application->get_renderer();
    if(g == h)
    {
    
    }
    if (!nightMode) {
        nightMode = true;
    } else {
        nightMode = false;
    }
}

void subwaystations(GtkWidget */*widget*/, ezgl::application *application) {

    ezgl::renderer *g = application->get_renderer();
    ezgl::renderer *h = application->get_renderer();
    if(g == h)
    {
    
    }

    for (int i = 0; i < getNumberOfNodes(); i++) {
        const OSMNode *currNode = getNodeByIndex(i);
        for (int j = 0; j < getTagCount(currNode); j++) {
            pair<string, string> tagPair = getTagPair(currNode, j);
            if (tagPair.first == "station" && tagPair.second == "subway") {
                Global_osm_subway_stations.push_back(currNode);
                break;
            }
        }
    }


        for (int i = 0; i < Global_osm_subway_stations.size(); i++) {
            for (int j = 0; j < getTagCount(Global_osm_subway_stations[i]); j++) {
                pair<string, string> tagPair = getTagPair(Global_osm_subway_stations[i], j);

                if (tagPair.first == "name") {
                    cout << "\t" << tagPair.second << endl;
                    break;
                }
            }
        }
//    }
}
//This function is used to draw subway icons on the map when user want to 

void drawSubway(ezgl::renderer *g) {


    for (int i = 0; i < Global_osm_subway_stations.size(); i++) {
        ezgl::surface *png_surface = ezgl::renderer::load_png("subway.png");
        g->draw_surface(png_surface,{x_from_lon(getNodeCoords(Global_osm_subway_stations[i]).lon()), getNodeCoords(Global_osm_subway_stations[i]).lat()});
        ezgl::renderer::free_surface(png_surface);


    }

}

void find_POI(GtkWidget */*widget*/, ezgl::application *application) {
    application->update_message("Find Point Of Interest Button Pressed");
    string POIName;
    int POINum =-1;
    string originalName;

    bool found = false;
    cout << "Please enter the name of the point of interest you would like to find (enter 'exit' to exit): ";
    getline(cin, POIName);
    originalName = POIName;
    POIName = toLowerAndRemoveWhiteSpace(POIName);
    if (POIName != "exit") {
        while (found == false && POIName != "exit") {

            for (int i = 0; i < Global_InterestName.size(); i++) {
                if (POIName == toLowerAndRemoveWhiteSpace(Global_InterestName[i])) {
                    cout << "Found point of interest, zooming map to location.. " << endl;
                    POINum = i;
                    found = true;
                }
                if (found == true)
                    break;
            }
            if (found == false) {
                cout << "Could not find point of interest with name: " << originalName << ". Please try again." << endl;
                cout << "Please enter the name of the point of interest you would like to find (enter 'exit' to exit): ";
                getline(cin, POIName);
                originalName = POIName;
                POIName = toLowerAndRemoveWhiteSpace(POIName);
            }
        }

    }
    if (POIName == "exit") {
        application->refresh_drawing();
    } else {
        vector<LatLon> points;


        LatLon pointCoord = getPointOfInterestPosition(POINum);
        points.push_back(pointCoord);


        ezgl::renderer* g = application->get_renderer();
        ezgl::rectangle rec = findMaxRectanglesOfPoints(points);
        g->set_visible_world(rec);
        application->refresh_drawing();
    }

}

//Draw path to walk a bit and drive the rest in different colours
void walkDrive(GtkWidget */*widget*/, ezgl::application *application) {

    application->update_message("Walk and Drive Button Pressed");
    pair<vector<StreetSegmentIndex>, vector<StreetSegmentIndex>> path;
    int departure;
    int destination;
    string start;
    string end;
    double turn_penalty = 15;
    double walking_speed = 1.4;
    double walking_time_limit;
    bool FoundDeparture = false;
    bool FoundDestination = false;
    bool cancel = false;
    bool isDuplicate = false;
    bool validSpeed = false;
    bool validTime = false;
    vector <IntersectionIndex> possible_departures;
    vector <IntersectionIndex> possible_destinations;
    vector <string> departure_names;
    vector <string> destination_names;
    string duplicate;
    string currentStreet;
    while(!FoundDeparture)
    {


    cout << color <<  "Please enter your starting point(enter 'cancel'to cancel): ";

    getline(cin, start);
    
    if(toLowerAndRemoveWhiteSpace(start) == "cancel")
    {
        cout<<"Returning to map......"<<endl;
        cancel = true;
        return;
    }
    possible_departures = find_intersection_ids_from_partial_intersection_name(start);
    if(possible_departures.size() == 0)
    {
        cout<<"***Could not find departure, please ensure the input is correct. The input you entered: "<<start<<endl;
        
    }
    else if(possible_departures.size() > 1 )
    {
        
        cout<<"***Found multiple matching departures: "<<endl;
        for(int i = 0; i < possible_departures.size(); i++)
        {
            string possibles = getIntersectionName(possible_departures[i]);

            cout<<possibles<<endl;
        }
        
    }
    else if(possible_departures.size() == 1)
    {
        cout<<"***Found matching departure: "<<start<<endl;
        FoundDeparture = true;
    }
    }

    while(!FoundDestination)
    {

        cout << "Please enter your ending point(enter'cancel' to cancel): ";

        getline(cin, end);
        if(toLowerAndRemoveWhiteSpace(end) == "cancel")
        {
            cout<<"Returning to map......"<<endl;
            cancel = true;
            return;
        }
        possible_destinations = find_intersection_ids_from_partial_intersection_name(end);
    

    
    if(possible_destinations.size() == 0)
    {
         cout<<"***Could not find destination, please ensure the input is correct. The input you entered: "<<end<<endl;
    }
    
    if(possible_destinations.size() > 1)
    {
        cout<<"***Found multiple matching destinations: "<<endl;
        for(int j = 0; j < possible_destinations.size(); j++)
        {
            string possibles = getIntersectionName(possible_destinations[j]);
            cout<<possibles<<endl;
 
        }
    }
    
    if(possible_destinations.size() == 1)
    {
        cout<<"***Found matching destination: "<<end<<endl;
        FoundDestination = true;
    }
    }
    departure = possible_departures[0];
    destination = possible_destinations[0];
    if(!cancel)
    {
                      firstIntersection = departure;
        secondIntersection = destination;
    while(!validSpeed)
    {
    cout<<"Please enter your walking speed, enter 0 for default (default walking speed is 1.4 m/s): "<<endl;
    cin>>walking_speed;
    if(walking_speed == 0)
    {
        walking_speed = 1.4;
        validSpeed = true;
    }else if(walking_speed > 0)
    {
        validSpeed = true;
    }else
    {
        cout<<"Invalid walking speed, please try again"<<endl;
    }
    }
    while(!validTime)
    {
    cout<<"How long would you like to walk(in seconds): ";
  
    cin>>walking_time_limit;
    
    if(walking_time_limit >= 0)
    {
        validTime = true;
    }else
    {
        cout<<"Invalid walking time, please try again"<<endl;
    }
    }
    cout<<"Please follow the walk directions or the highlighted route: "<<endl;
    path = find_path_with_walk_to_pick_up(departure, destination, turn_penalty, walking_speed, walking_time_limit);
    cout<<"departure: "<<departure<<"destination: "<<destination<<endl;
    double walk_time=int(compute_path_walking_time(path.first,walking_speed,turn_penalty)/60);
    double drive_time=int(compute_path_travel_time(path.second,turn_penalty)/60);
 
    cout<<"Walking time will take approximately "<<walk_time<<"minutes"<<endl;
    cout<<"Driving time will take approximately "<<drive_time<<"minutes"<<endl;
    
    cout<<"It will take approximately " << walk_time+drive_time << "minutes to reach your destination" << endl; 
    for(int i = 0; i < path.first.size(); i++)
    {
        double distance=int(find_street_segment_length(path.first[i]));
        isDuplicate = false;
        currentStreet = getStreetName(getInfoStreetSegment(path.first[i]).streetID);
        if(i == 0)
        {
            cout<<"Start by walking down "<<currentStreet<<endl;
            cout <<" for " << distance << "m" << endl;
            duplicate = currentStreet;
        }else
        {
        
                if(currentStreet == duplicate)
                {
                    isDuplicate = true;
                }
            
            if(isDuplicate == false && i < path.first.size() -1)
            {
                
                 if(getIntersectionPosition(getInfoStreetSegment(path.first[i]).from).lat()<getIntersectionPosition(getInfoStreetSegment(path.first[i+1]).to).lat()){
                    cout<< color2 << "Then, turn left on "<<currentStreet<<endl;
                     cout <<" for " << distance << "m" << endl;
                    duplicate = currentStreet;
                }
                else {
                    cout<<"Then, turn right on "<<currentStreet<<endl;
                     cout <<" for " << distance << "m" << endl;
                    duplicate = currentStreet;   
                }
            }
        }
        
    }
    cout<<"You are now at your destination for pick up, now please follow the drive directions or the highlighted route: "<<endl;
    for(int j = 0; j < path.second.size(); j++)
    {
        double distance=int(find_street_segment_length(path.second[j]));
        
        isDuplicate = false;
        currentStreet = getStreetName(getInfoStreetSegment(path.second[j]).streetID);
        if(j == 0)
        {
            cout<<"Drive down "<<currentStreet<<endl;
             cout <<" for " << distance << "m" << endl;
            duplicate = currentStreet;
        }else
        {
        
                if(currentStreet == duplicate)
                {
                    isDuplicate = true;
                }
            
            if(isDuplicate == false && j < path.second.size() -1)
            {
                
                if(getIntersectionPosition(getInfoStreetSegment(path.second[j]).from).lat()<getIntersectionPosition(getInfoStreetSegment(path.second[j]).to).lat()){
                    cout<<"Then, turn left on "<<currentStreet<<endl;
                     cout <<" for " << distance << "m" << endl;
                    duplicate = currentStreet;
                }
                else {
                    cout<<"Then, turn right on "<<currentStreet<<endl;
                     cout <<" for " << distance << "m" << endl;
                    duplicate = currentStreet;   
                }
            }
        }
}
    cout<<"You are now at your destination!"<<endl;
    }
    walkPath = path.first;
    drivePath = path.second;
    vector <LatLon> points;
    if(walkPath.size() != 0)
    {
    
    for(int i = 0; i< walkPath.size() ;i++)
    {

        points.push_back(getIntersectionPosition(Global_from[walkPath[i]]));
        points.push_back(getIntersectionPosition(Global_to[walkPath[i]]));
        
    }
    }
    if(drivePath.size() != 0)
    {
        for(int i = 0; i< drivePath.size() ;i++)
    {

        points.push_back(getIntersectionPosition(Global_from[drivePath[i]]));
        points.push_back(getIntersectionPosition(Global_to[drivePath[i]]));
        
    }
    }
    if(points.size() != 0)
    {
    ezgl::renderer* g = application->get_renderer();
    ezgl::rectangle rec = findMaxRectanglesOfPoints(points);
    g->set_visible_world(rec);
    application->refresh_drawing();
    }
    
    

}

//button for user to select departure by click
void selectFirstIntersection(GtkWidget */*widget*/, ezgl::application *application) {
    ezgl::renderer *g = application->get_renderer();
    ezgl::renderer *h = application->get_renderer();
    if(g == h)
    {
    
    }
        application->update_message("Selecting First Intersection");

    currentlySelecting = 1;
    cout<<"Selecting First"<<endl;

}
//button for user to select destination by click
void selectSecondIntersection(GtkWidget */*widget*/, ezgl::application *application) {
    ezgl::renderer *g = application->get_renderer();
    ezgl::renderer *h = application->get_renderer();
    if(g == h)
    {
    
    }
    application->update_message("Selecting Second Intersection");

    currentlySelecting = 2;
    cout<<"Selecting Second"<<endl;

}

//ui for drawing path when user clicks two intersections
void driveOnlyClick(GtkWidget */*widget*/, ezgl::application *application) {
    bool isDuplicate = false;
    string duplicate;
    string currentStreet;
    if (firstIntersection != -1 && secondIntersection != -1) {
        drivePath = find_path_between_intersections(firstIntersection, secondIntersection, 0);
        walkPath = {};
        cout<<"Please follow the drive directions or the highlighted route: "<<endl;
    for(int j = 0; j < drivePath.size(); j++)
    {
        isDuplicate = false;
        currentStreet = getStreetName(getInfoStreetSegment(drivePath[j]).streetID);
        if(j == 0)
        {
            cout<<"Drive down "<<currentStreet<<endl;
            duplicate = currentStreet;
        }else
        {
        
                if(currentStreet == duplicate)
                {
                    isDuplicate = true;
                }
            
            if(isDuplicate == false)
            {
                cout<<"Then, drive down "<<currentStreet<<endl;
                duplicate = currentStreet;
            }
        }
}
    cout<<"You are now at your destination!"<<endl;
        application->refresh_drawing();
     
    }


}


void donePathFinding(GtkWidget */*widget*/, ezgl::application *application) {
    ezgl::renderer *g = application->get_renderer();
    ezgl::renderer *h = application->get_renderer();
    if(g == h)
    {
    
    }
    if (pathSearch) {
        pathSearch = false;
        cout<<"Toggle Path Search Off"<<endl;
    } else {
        pathSearch = true;
        cout<<"Toggle Path Search On"<<endl;
    }
}

// display info to help user
void UserGuide(GtkWidget */*widget*/, ezgl::application *application)
{
    string button;
    bool error = false;
    do{
    application->update_message("user guide button pressed");
    cout<<"Welcome to Second Home Maps. Here is a detailed guide to our interface. Our functionality is mainly delivered by our buttons. If you would like to learn about a button, type the button name. If you would like to learn about all our buttons, type 'all'. Type 'exit' to exit."<<endl;
    getline(cin, button);
    button = toLowerAndRemoveWhiteSpace(button);
    if(button == "findintersection")
    {
        cout<<"The Find Intersection button requires you to type in two street names, and the map will then find the intersection of these to streets. The intersection will also be highlighted."<<endl;
        
    }
    else if(button == "findfeature")
    {
        cout<<"The 'Find Feature' button requires you to type in a feature name, and the map will then zoom to the feature."<<endl;
        
    }
    else if(button == "proceed")
    {
        cout<<"The 'Proceed' button allows you to load a new map."<<endl;
    }
    else if(button == "findpointofinterest")
    {
        cout<<"The 'Find Point Of Interest' button requires you to type in a Point Of Interest name, and the map will then zoom to the point."<<endl;
    }
    else if(button == "subwaystations")
    {
        cout<<"The 'Subway Stations' button displays all subway stations in the city."<<endl;
    }
    else if(button == "nightmode")
    {
        cout<<"The 'Night Mode' button changes the color scheme to dark gray to protect your eyes at night."<<endl;
    }
    else if(button == "driveonly")
    {
        cout<<"The 'Drive Only' button finds an efficient path between two intersections that you can drive on."<<endl;
    }
    else if(button == "walkanddrive")
    {
        cout<<"The 'Walk and Drive' button finds an efficient path between two intersections where you walk a portion, and drive the other portion."<<endl;
    }
    else if(button == "exit")
    {
        cout<<"Exiting....."<<endl;
    }
    else if(button == "selectfirstintersection")
    {
        cout<<"Click on the screen to select starting point of desired drive path."<<endl;
    }
    else if(button == "selectsecondintersection")
    {
        cout<<"Click on the screen to select ending point of desired drive path, and then path will appear."<<endl;
    }
    else if(button == "all")
    {
        cout<<"Below are the functionalities of each button:"<<endl;
        cout<<"Proceed: allows you to load a new map."<<endl;
        cout<<"Find Intersection: requires you to type in two street names, and the map will then find the intersection of these to streets. The intersection will also be highlighted."<<endl;
        cout<<"Find Feature: requires you to type in a feature name, and the map will then zoom to the feature."<<endl;
        cout<<"Find Point Of Interest: requires you to type in a Point Of Interest name, and the map will then zoom to the point."<<endl;
        cout<<"Subway Stations: displays all subway stations in the city."<<endl;
        cout<<"Night Mode: changes the color scheme to dark gray to protect your eyes at night."<<endl;
        cout<<"Drive Only: Finds an efficient path between two intersections that you can drive on."<<endl;
        cout<<"Walk and Drive: Finds an efficient path between two intersections where you walk a portion, and drive the other portion."<<endl;
        cout<<"Select First Intersection: click on the screen to select starting point of desired drive path."<<endl;
        cout<<"Select Second Intersection: click on the screen to select ending point of desired drive path, and then path will appear."<<endl;
        
    }
    else
    {
        cout<<"Couldn't find desired button. Please ensure button name is correct"<<endl;
        error = true;
    }
    
    }while(error == true);
    
}
//ui for user to input intersections and find driving path
void Drive(GtkWidget */*widget*/, ezgl::application *application) {
    
    application->update_message("Drive Button Pressed");
    
    
  
    int departure;
    int destination;
    string start;
    string end;
    double turn_penalty = 15;
  
    bool FoundDeparture = false;
    bool FoundDestination = false;
    bool cancel = false;
    bool isDuplicate = false;
    vector <IntersectionIndex> possible_departures;
    vector <IntersectionIndex> possible_destinations;
    vector <string> departure_names;
    vector <string> destination_names;
    string duplicate;
    string currentStreet;
    while(!FoundDeparture)
    {


    cout << color <<  "Please enter your starting point(enter 'cancel'to cancel): ";
//    getline(cin, departure);
//    departure = toLowerAndRemoveWhiteSpace(departure);
    getline(cin, start);
    
    if(toLowerAndRemoveWhiteSpace(start) == "cancel")
    {
        cout<<"Returning to map......"<<endl;
        cancel = true;
        return;
    }
    possible_departures = find_intersection_ids_from_partial_intersection_name(start);
    if(possible_departures.size() == 0)
    {
        cout<<"***Could not find departure, please ensure the input is correct. The input you entered: "<<start<<endl;
        
    }
    else if(possible_departures.size() > 1 )
    {
        
        cout<<"***Found multiple matching departures: "<<endl;
        for(int i = 0; i < possible_departures.size(); i++)
        {
            string possibles = getIntersectionName(possible_departures[i]);
            
//            cout<<getIntersectionName(possible_departures[i])<<endl;
            cout<<possibles<<endl;

        }
        
    }
    else if(possible_departures.size() == 1)
    {
        cout<<"***Found matching departure: "<<start<<endl;
        FoundDeparture = true;
    }
    }


    while(!FoundDestination)
    {

        cout << "Please enter your ending point(enter'cancel' to cancel): ";


        getline(cin, end);
        if(toLowerAndRemoveWhiteSpace(end) == "cancel")
        {
            cout<<"Returning to map......"<<endl;
            cancel = true;
            return;
        }
        possible_destinations = find_intersection_ids_from_partial_intersection_name(end);
    

    if(possible_destinations.size() == 0)
    {
         cout<<"***Could not find destination, please ensure the input is correct. The input you entered: "<<end<<endl;

    }
    
    if(possible_destinations.size() > 1)
    {
        cout<<"***Found multiple matching destinations: "<<endl;
        for(int j = 0; j < possible_destinations.size(); j++)
        {
            string possibles = getIntersectionName(possible_destinations[j]);
            cout<<possibles<<endl;

        }
    }
    
    if(possible_destinations.size() == 1)
    {
        cout<<"***Found matching destination: "<<end<<endl;
        FoundDestination = true;
    }
    }
    departure = possible_departures[0];
    destination = possible_destinations[0];
    if(!cancel)
    {
    
   
        //destination = toLowerAndRemoveWhiteSpace(destination);   
    drivePath = find_path_between_intersections(departure, destination,turn_penalty);
    double travel_time=int(compute_path_travel_time( drivePath,turn_penalty)/60);
    cout<< blue<< "It will take estimately " << travel_time << "minutes to reach your destination" << endl; 
    cout<<"Please follow the directions or the highlighted route: "<<endl;
    walkPath = {};
              firstIntersection = departure;
        secondIntersection = destination;
    if( drivePath.empty()){
       cout<<"Sorry,there is no route found between departure and destination" <<endl;
    }

    for(int i = 0; i <  drivePath.size(); i++)
    {
       
        double distance=int(find_street_segment_length( drivePath[i]));
        isDuplicate = false;
        currentStreet = getStreetName(getInfoStreetSegment( drivePath[i]).streetID);
        if(i== 0)
        {
            cout<<"Start by driving down "<<currentStreet<<endl;
            duplicate = currentStreet;
        }else
       
                if(currentStreet == duplicate)
                {
                    
                    if(distance>=250) {
                    cout<<"Then,Continue straight on "<<currentStreet;
                    cout<<" for "<< distance << "m" << endl;
                    }
                    
                    isDuplicate = true;
                  
                }
                      
            if(isDuplicate == false && i < drivePath.size() -1)
            {
                if(getIntersectionPosition(getInfoStreetSegment( drivePath[i]).from).lat()<getIntersectionPosition(getInfoStreetSegment( drivePath[i+1]).to).lat()){
                    cout<< color2<<"Then, turn left on "<<currentStreet;
                    cout<<" for "<< distance << "m" << endl;
                    duplicate = currentStreet;
                }
                else {
                    cout<< color3 <<"Then, turn right on "<<currentStreet;
                    cout<<" for "<< distance << "m" << endl;
                    duplicate = currentStreet;   
                }
            }
       
      
        //if there are more than one curve point,need to loop through then connecting all of the points together 
    }
    
    cout<<"You are now at your destination!"<<endl;
    
      vector <LatLon> points;
    if(drivePath.size() != 0)
    {
        for(int i = 0; i< drivePath.size() ;i++)
    {
        points.push_back(getIntersectionPosition(Global_from[drivePath[i]]));
        points.push_back(getIntersectionPosition(Global_to[drivePath[i]]));
        
    }
    }
    if(points.size() != 0)
    {
    ezgl::renderer* g = application->get_renderer();
    ezgl::rectangle rec = findMaxRectanglesOfPoints(points);
    g->set_visible_world(rec);
    application->refresh_drawing();
    }
}
}
    

//Draw the driving path
void drawDrivePath(ezgl::renderer *g)

{
    if( drivePath.size() != 0)
    {

        g->set_line_width(3);
        for(int i = 0; i <  drivePath.size(); i++)
        {
            
            int current =  drivePath[i];
            g->set_color(ezgl::BLUE);
            if(Global_number_of_curve_points[current] == 0)
            {
                g->draw_line({x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()
            });
            }
            else if(Global_number_of_curve_points[current] == 1)
            {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0,current).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
            });
            g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
            {
                x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()
            });
            }else {
            //corner case when there is a street that starts and ends in the exactly same point 
            if (getIntersectionPosition(Global_from[current]).lat() == getIntersectionPosition(Global_to[current]).lat() && getIntersectionPosition(Global_from[current]).lon() == getIntersectionPosition(Global_to[current]).lon()) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });

                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });
            }//If not the corner case connect from and to to the first and last curve point and connect all together 
            else {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(0, current).lon()), getStreetSegmentCurvePoint(0, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_from[current]).lon()), getIntersectionPosition(Global_from[current]).lat()
                });
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lon()), getStreetSegmentCurvePoint(Global_number_of_curve_points[current] - 1, current).lat()},
                {
                    x_from_lon(getIntersectionPosition(Global_to[current]).lon()), getIntersectionPosition(Global_to[current]).lat()

                });
            }
            //connect the curve points from 0 to number of curve points of the segment -1
            for (int j = 0; j < Global_number_of_curve_points[current] - 1; ++j) {
                g->draw_line({x_from_lon(getStreetSegmentCurvePoint(j, current).lon()), getStreetSegmentCurvePoint(j, current).lat()},
                {

                    x_from_lon(getStreetSegmentCurvePoint(j + 1, current).lon()), getStreetSegmentCurvePoint(j + 1, current).lat()
                });
            }
        }
    }
}
}