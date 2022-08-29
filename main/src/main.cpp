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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <string>
#include "m3.h"
#include "m2.h"
#include "m1.h"
#include "OSMDatabaseAPI.h"
#include "helper.h"
using namespace std;

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage
string getMapOrExit();
//The default map to load if none is specified//
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
string toronto = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
string beijing = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
string cairo = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
string capetown = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
string goldenhorseshoe = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
string hamilton = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
string hongkong = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
string iceland = "/cad2/ece297s/public/maps/iceland.streets.bin";
string interlaken = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
string london = "/cad2/ece297s/public/maps/london_england.streets.bin";
string moscow = "/cad2/ece297s/public/maps/moscow_russia.streets.bin";
string newdelhi= "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
string newyork = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
string rio_de_janeiro = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
string saint_helena = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
string singapore = "/cad2/ece297s/public/maps/singapore.streets.bin";
string sydney = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
string tehran = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
string tokyo = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";

int main(int argc, char** argv) {
    bool exit = false;
    
   while( exit == false)
    {
    
    std::string map_path;
    if(argc == 1) {
        
        //Use a default map
        //map_path=default_map_path;
        map_path = getMapOrExit();
        if(map_path == "exit")
        {
            cout<<"Terminating program..."<<endl;
            return SUCCESS_EXIT_CODE;
        }
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }

    //Load the map and related data structures
    bool load_success = load_map(map_path);
    if(!load_success) {
        std::cerr << "Failed to load map '" << map_path << "'\n";
        return ERROR_EXIT_CODE;
    }

    std::cout << "Successfully loaded map '" << map_path << "'\n";
    
    //You can now do something with the map data


    //Clean-up the map data and related data structures
    std::cout << "Closing map\n";
    draw_map();
    close_map(); 
    
   }
    return SUCCESS_EXIT_CODE;
}

string getMapOrExit() {
    cout << "Maps We Offer: Hamilton, Toronto, Golden-Horseshoe, Beijing, Hongkong, "
            << "Cairo, Cape-Town, Iceland, Interlaken, New-York, Saint-Helena, "
            << "London, Moscow, New-Delhi, Rio-de-Janeiro, Singapore, "
            << "Sydney, Tehran, Tokyo." << endl;
    cout << "To exit the program type 'exit'" << endl;
    cout << "What map would you like to see? (Press the 'Proceed' button to load a new map)" << endl;

    string map;

    while (true) {
        getline(cin, map);
        map = toLowerAndRemoveWhiteSpace(map);
        if (map == "toronto")
        {
            return toronto;
        }
        else if ( map == "hamilton")
        {
            return hamilton;
        }
        else if (map == "golden-horseshoe")
        {
            return goldenhorseshoe;
        }
        else if (map == "beijing")
        {
            return beijing;
        }
        else if(map == "hongkong")
        {
            return hongkong;
        }
        else if(map == "cairo")
        {
            return cairo;
        }
        else if(map == "cape-town" || map == "capetown" )
        { 
            return capetown;
        }
        else if( map == "iceland")
        {
            return iceland;
        }
        else if (map == "interlaken")
        {
           return interlaken;
        }
        else if(map == "new-york" || map == "newyork")
        {
            return newyork;
        }
        else if(map == "saint-helena" || map == "sainthelena")
        {
            return saint_helena;
        }
        else if( map == "london")
        {
            return london;
        }
        else if(map == "moscow")
        {
            return moscow;
        }
        else if( map == "new-delhi" || map == "newdelhi" )
        {
            return newdelhi;
        }
        else if(map == "rio-de-janeiro" || map == "riodejaneiro" ||
            map == "rio-dejaneiro" || map == "riode-janeiro")
        {
            return rio_de_janeiro;
        }
        else if(map == "singapore")
        {
            return singapore;
        }
        else if(map == "sydney")
        {
            return sydney;
        }
        else if (map == "tehran")
        {
            return tehran;
        }
        else if(map == "tokyo")
        {
            return tokyo;
        }
        else if(map == "goldenhorseshoe")
        {
            return goldenhorseshoe;
        }
        else if(map == "exit")
        {
            return map;
        } else {
            cout << "Invalid map name, please try again (only map names offered and 'exit' are valid)." << endl;
        }
    }
    
    return map;
}