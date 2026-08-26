#ifndef NETWORK_H
#define NETWORK_H

#include "Router.h"
#include <unordered_map>
#include <vector>
#include <string>

class Network {
private:
    std::unordered_map<int, Router> routers;

public:
    void addRouter(int id);

    void addLink(int routerA, int routerB, int cost);

    void removeLink(int routerA, int routerB);

    Router* getRouter(int id);

    void generateAllLSAs();

    void floodLSAs();

    void printTopology() const;

    void printAllLSDBs() const;

    void exportTopologyCSV(const std::string& filename) const;
};

#endif