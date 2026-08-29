#ifndef NETWORK_H
#define NETWORK_H

#include "Router.h"
#include <unordered_map>
#include <vector>
#include <string>

struct FloodResult {
    int totalIterations;
    std::unordered_map<int, int> iterationsByOrigin;

    FloodResult();
};

struct RouteEntry {
    int destination;
    int nextHop;
    int cost;
    std::vector<int> path;
};

class Network {
private:
    std::unordered_map<int, Router> routers;

    std::vector<int> getSortedRouterIds() const;

    std::unordered_map<int, std::unordered_map<int, int>>
    buildGraphFromLSDB(int routerId) const;

    std::vector<RouteEntry> computeRoutingTableFromGraph(
        int source,
        const std::unordered_map<int, std::unordered_map<int, int>>& graph
    ) const;

    int getUndirectedLinkCount() const;

public:
    void addRouter(int id);

    void addLink(int routerA, int routerB, int cost, bool verbose = true);

    void removeLink(int routerA, int routerB, bool verbose = true);

    Router* getRouter(int id);

    void generateAllLSAs(bool verbose = true);

    FloodResult floodLSAs(bool verbose = true);

    bool areLSDBsSynchronized() const;

    std::vector<RouteEntry> computeRoutingTable(int source) const;

    void printRoutingTable(int source) const;

    void printAllRoutingTables() const;

    void printTopology() const;

    void printAllLSDBs() const;

    void exportTopologyCSV(const std::string& filename) const;

    void exportRoutingTablesCSV(const std::string& filename) const;

    static void runMeasurementExperiments(
        const std::string& spfFilename,
        const std::string& convergenceFilename,
        int minRouters,
        int maxRouters,
        int step,
        int runsPerSize
    );
};

#endif
