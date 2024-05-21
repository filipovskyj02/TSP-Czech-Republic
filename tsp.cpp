#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <cfloat>
#include <algorithm>

#define ALPHA 0.9
#define BETA 5
#define EVAPORATION_CONSTANT 0.9
#define DELTA 100
#define ITERATIONS 10
#define NUM_OF_ANTS 16

class City {
public:
    unsigned int id;
    std::string name;
    double latitude;
    double longitude;

    City() : id(0), name(""), latitude(0.0), longitude(0.0) {}
    City(int id, std::string name, double latitude, double longitude) : id(id), name(name), latitude(latitude), longitude(longitude) {}

    friend std::ostream & operator<<(std::ostream & os, const City & city) {
        os << std::setprecision(10) << city.id << " " << city.name << " " << city.latitude << " " << city.longitude << std::endl;
        return os;
    }    
};



class Map {
public:
    int cityCnt;
    std::vector<City> map;
    std::vector<std::vector<double>> distanceMatrix;
    std::vector<std::vector<double>> pheromoneMatrix;
    double initPheromoneVal;
    Map(std::string filename) {
        srand(time(NULL));
        std::ifstream file(filename);
        std::string line;
        getline(file, line);
        cityCnt = stoi(line);
        std::cout << "city cnt " << cityCnt << std::endl;
        map.resize(cityCnt);
        std::string cityName;
        for (int currentIndex = 0; currentIndex < cityCnt; currentIndex++) {
            std::getline(file, line);
            std::stringstream ss(line);
            double latitude, longitude;
            ss >> std::quoted(cityName) >> latitude >> longitude;
            map[currentIndex] = City(currentIndex, cityName, latitude, longitude);
            std::cout << map[currentIndex];
        }
        distanceMatrix.resize(cityCnt, std::vector<double>(cityCnt, -1.0));
        for (int i = 0; i < cityCnt; i++){
            for (int j = 0; j < cityCnt; j++){
                auto & dist = distanceMatrix[i][j];
                if (i == j) dist = 0.0;
                else if (dist == -1.0) {
                    dist = haversine(map[i].latitude, map[i].longitude, map[j].latitude, map[j].longitude);
                    distanceMatrix[j][i] = dist;
                }
            }
        }
        initPheromoneVal = 1.0 / (double)cityCnt;
        pheromoneMatrix.resize(cityCnt, std::vector<double>(cityCnt, initPheromoneVal));
        antColonyOptimazation();
    }
    class AntResult {
    public:
        double totalPathDistance;
        std::vector<int> path;

        bool operator<(const AntResult & other){
            return totalPathDistance < other.totalPathDistance;
        }
    };
    void antColonyOptimazation(){
        auto totalStartTime = std::chrono::system_clock::now();
        std::vector<int> currBestPath;
        double currBestDistance = DBL_MAX;
        for (int iteration = 0; iteration < ITERATIONS; iteration++){
            auto iterationStartTime = std::chrono::system_clock::now();
            std::vector<AntResult> results(NUM_OF_ANTS);
            for (auto & ant: results){
                singleAntTraversal(ant);
            }
            std::sort(results.begin(), results.end());
            if (results[0].totalPathDistance < currBestDistance){
                currBestDistance = results[0].totalPathDistance;
                currBestPath = results[0].path;
                std::cout << " currNewBest = " << currBestDistance << " km." << std::endl;
            }
            for (int i = 0; i < cityCnt; i++){
                for (int j = 0; j < cityCnt; j++){
                    pheromoneMatrix[i][j] *= EVAPORATION_CONSTANT;
                }
            }
            // keep only top half
            for (int i = 0; i < NUM_OF_ANTS/2; i++){
                double change = DELTA / results[i].totalPathDistance;
                auto & pathRef = results[i].path;
                for (int j = 0; j < cityCnt - 1; j++){
                    pheromoneMatrix[pathRef[j]][pathRef[j+1]] += change;
                }
                pheromoneMatrix[pathRef[cityCnt - 1]][pathRef[0]] += change;
            }
            auto iterationEndTime = std::chrono::system_clock::now();
            auto iterationElapsed = std::chrono::duration_cast<std::chrono::seconds>(iterationEndTime - iterationStartTime);
            std::cout << "Iteration took: " << iterationElapsed.count() << " s" << std::endl;
        }
        auto totalEndTime = std::chrono::system_clock::now();
        auto totalElapsed = std::chrono::duration_cast<std::chrono::seconds>(totalEndTime - totalStartTime);
        std::cout << "Execution took: " << totalElapsed.count() << " s" << std::endl;
    }
    int selectNextCity(std::vector<bool> & visited, int current){
        std::vector<int> neighborIndexes;
        std::vector<double> neighborValues;
        for (int node = 0; node < cityCnt; ++node) {
            if (visited[node] || node == current) continue;
            double pheromoneLevel = pheromoneMatrix[current][node]; 
            double value = pow(pheromoneLevel, ALPHA) / pow(distanceMatrix[current][node], BETA);
            neighborIndexes.push_back(node);
            neighborValues.push_back(value);
        }
        std::random_device rd;
        std::discrete_distribution<> distribution(neighborValues.begin(), neighborValues.end());
        int next = neighborIndexes[distribution(rd)];
        return next;
    }

    void singleAntTraversal(AntResult & ant){
        //std::cout << "next ant spawned!" << std::endl;
        ant.path.resize(cityCnt, -1);
        auto & citiesOrder = ant.path; 
        int currCityInOrder = 0;
        int currentIndex = rand() % cityCnt;
        std::vector<bool> visited(cityCnt, false);
        visited[currentIndex] = true;
        citiesOrder[0] = currentIndex;
        while (++currCityInOrder < cityCnt){
            currentIndex = selectNextCity(visited, currentIndex);
            visited[currentIndex] = true;
            citiesOrder[currCityInOrder] = currentIndex;
        }
        ant.totalPathDistance = getTotalPathDistance(citiesOrder);
    }

    double inline haversine(double lat1, double lon1, double lat2, double lon2){
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
        lat1 = (lat1) * M_PI / 180.0;
        lat2 = (lat2) * M_PI / 180.0;
        double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
        double rad = 6371;
        double c = 2 * asin(sqrt(a));
        return rad * c;
    }
    double inline getTotalPathDistance(std::vector<int> & citiesOrder){
        double total = 0;
        for (int i = 0; i < (int)citiesOrder.size() - 1; i++){
            total += distanceMatrix[citiesOrder[i]][citiesOrder[i + 1]];
        }
        total += distanceMatrix[citiesOrder[citiesOrder.size() - 1]][citiesOrder[0]];
        return total;
    }

};

int main() {
    Map currmap("maps/allCities6367.txt");
}