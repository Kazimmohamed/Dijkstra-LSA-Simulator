#include "Network.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <set>
#include <unordered_set>
#include <fstream>

FloodResult::FloodResult()
    : totalIterations(0) {
}

void Network::addRouter(int id) {

    if (routers.find(id) == routers.end()) {
        routers.emplace(id, Router(id));
    }
}

void Network::addLink(int routerA, int routerB, int cost, bool verbose) {

    addRouter(routerA);
    addRouter(routerB);

    routers.at(routerA).addNeighbor(routerB, cost);
    routers.at(routerB).addNeighbor(routerA, cost);

    if (verbose) {
        std::cout
            << "Link added: "
            << routerA
            << " <-> "
            << routerB
            << " (cost "
            << cost
            << ")\n";
    }
}

void Network::removeLink(int routerA, int routerB, bool verbose) {

    if (routers.find(routerA) == routers.end()) {
        return;
    }

    if (routers.find(routerB) == routers.end()) {
        return;
    }

    routers.at(routerA).removeNeighbor(routerB);
    routers.at(routerB).removeNeighbor(routerA);

    if (verbose) {
        std::cout
            << "\nLink removed: "
            << routerA
            << " <-> "
            << routerB
            << "\n";
    }
}

Router* Network::getRouter(int id) {

    auto it = routers.find(id);

    if (it == routers.end()) {
        return nullptr;
    }

    return &it->second;
}

void Network::generateAllLSAs(bool verbose) {

    if (verbose) {
        std::cout << "\nGenerating LSAs...\n";
    }

    for (auto& entry : routers) {

        Router& router = entry.second;

        LSA lsa = router.generateLSA();

        router.receiveLSA(lsa);

        if (verbose) {
            std::cout
                << "Router "
                << router.getId()
                << " generated LSA #"
                << lsa.sequenceNumber
                << "\n";
        }
    }
}

FloodResult Network::floodLSAs(bool verbose) {

    if (verbose) {
        std::cout << "\nStarting LSA flooding...\n";
    }

    FloodResult result;

    for (auto& sourceEntry : routers) {

        int sourceId = sourceEntry.first;

        Router& sourceRouter = sourceEntry.second;

        auto sourceLSDB = sourceRouter.getLSDB();

        auto ownLSAIt = sourceLSDB.find(sourceId);

        if (ownLSAIt == sourceLSDB.end()) {
            continue;
        }

        LSA lsa = ownLSAIt->second;

        std::queue<int> currentRound;
        std::unordered_set<int> visited;

        currentRound.push(sourceId);
        visited.insert(sourceId);

        int iterationsForThisLSA = 0;

        while (!currentRound.empty()) {

            iterationsForThisLSA++;

            if (verbose) {
                std::cout
                    << "\n  LSA "
                    << lsa.originRouter
                    << "#"
                    << lsa.sequenceNumber
                    << " - Flooding round "
                    << iterationsForThisLSA
                    << ":\n";
            }

            std::queue<int> nextRound;

            while (!currentRound.empty()) {

                int currentId = currentRound.front();
                currentRound.pop();

                Router* currentRouter =
                    getRouter(currentId);

                if (currentRouter == nullptr) {
                    continue;
                }

                for (const auto& neighborEntry :
                     currentRouter->getNeighbors()) {

                    int neighborId = neighborEntry.first;

                    if (visited.find(neighborId) != visited.end()) {
                        continue;
                    }

                    visited.insert(neighborId);

                    Router* neighborRouter =
                        getRouter(neighborId);

                    if (neighborRouter == nullptr) {
                        continue;
                    }

                    bool newInformation =
                        neighborRouter->receiveLSA(lsa);

                    if (newInformation) {

                        if (verbose) {
                            std::cout
                                << "      "
                                << currentId
                                << " -> "
                                << neighborId
                                << "\n";
                        }

                        nextRound.push(neighborId);
                    }
                }
            }

            currentRound = std::move(nextRound);
        }

        result.totalIterations += iterationsForThisLSA;
        result.iterationsByOrigin[lsa.originRouter] = iterationsForThisLSA;

        if (verbose) {
            std::cout
                << "  LSA "
                << lsa.originRouter
                << "#"
                << lsa.sequenceNumber
                << " converged in "
                << iterationsForThisLSA
                << " flooding rounds.\n";
        }
    }

    if (verbose) {
        std::cout
            << "\nLSA flooding completed.\n";

        std::cout
            << "Total flooding iterations: "
            << result.totalIterations
            << "\n";
    }

    return result;
}

std::vector<int> Network::getSortedRouterIds() const {

    std::vector<int> ids;

    for (const auto& entry : routers) {
        ids.push_back(entry.first);
    }

    std::sort(ids.begin(), ids.end());

    return ids;
}

bool Network::areLSDBsSynchronized() const {

    if (routers.empty()) {
        return true;
    }

    const auto ids = getSortedRouterIds();
    const auto& reference = routers.at(ids.front()).getLSDB();

    for (int routerId : ids) {

        const auto& lsdb = routers.at(routerId).getLSDB();

        if (lsdb.size() != reference.size()) {
            return false;
        }

        for (const auto& entry : reference) {

            auto it = lsdb.find(entry.first);

            if (it == lsdb.end()) {
                return false;
            }

            const LSA& expected = entry.second;
            const LSA& actual = it->second;

            if (actual.sequenceNumber != expected.sequenceNumber ||
                actual.links != expected.links) {
                return false;
            }
        }
    }

    return true;
}

std::unordered_map<int, std::unordered_map<int, int>>
Network::buildGraphFromLSDB(int routerId) const {

    std::unordered_map<int, std::unordered_map<int, int>> graph;

    auto routerIt = routers.find(routerId);

    if (routerIt == routers.end()) {
        return graph;
    }

    for (int id : getSortedRouterIds()) {
        graph[id];
    }

    const auto& lsdb = routerIt->second.getLSDB();

    for (const auto& entry : lsdb) {

        int source = entry.first;
        graph[source];

        for (const auto& link : entry.second.links) {

            int destination = link.first;
            int cost = link.second;

            graph[source][destination] = cost;
            graph[destination][source] = cost;
        }
    }

    return graph;
}

std::vector<RouteEntry> Network::computeRoutingTableFromGraph(
    int source,
    const std::unordered_map<int, std::unordered_map<int, int>>& graph
) const {

    const int infinity = std::numeric_limits<int>::max() / 4;
    std::unordered_map<int, int> distance;
    std::unordered_map<int, int> previous;

    for (const auto& entry : graph) {
        distance[entry.first] = infinity;
        previous[entry.first] = -1;
    }

    if (distance.find(source) == distance.end()) {
        return {};
    }

    using QueueEntry = std::pair<int, int>;
    std::priority_queue<
        QueueEntry,
        std::vector<QueueEntry>,
        std::greater<QueueEntry>
    > queue;

    distance[source] = 0;
    queue.push({0, source});

    while (!queue.empty()) {

        QueueEntry current = queue.top();
        queue.pop();

        int currentDistance = current.first;
        int currentRouter = current.second;

        if (currentDistance != distance[currentRouter]) {
            continue;
        }

        auto neighborIt = graph.find(currentRouter);

        if (neighborIt == graph.end()) {
            continue;
        }

        for (const auto& neighborEntry : neighborIt->second) {

            int neighbor = neighborEntry.first;
            int cost = neighborEntry.second;
            int newDistance = currentDistance + cost;

            if (newDistance < distance[neighbor]) {
                distance[neighbor] = newDistance;
                previous[neighbor] = currentRouter;
                queue.push({newDistance, neighbor});
            }
        }
    }

    std::vector<int> destinations;

    for (const auto& entry : graph) {
        destinations.push_back(entry.first);
    }

    std::sort(destinations.begin(), destinations.end());

    std::vector<RouteEntry> table;

    for (int destination : destinations) {

        if (destination == source) {
            continue;
        }

        RouteEntry route;
        route.destination = destination;
        route.nextHop = -1;
        route.cost = distance[destination];

        if (route.cost != infinity) {

            int current = destination;

            while (current != -1) {
                route.path.push_back(current);

                if (current == source) {
                    break;
                }

                current = previous[current];
            }

            if (!route.path.empty() && route.path.back() == source) {
                std::reverse(route.path.begin(), route.path.end());

                if (route.path.size() > 1) {
                    route.nextHop = route.path[1];
                }
            } else {
                route.path.clear();
                route.cost = infinity;
            }
        }

        table.push_back(route);
    }

    return table;
}

std::vector<RouteEntry> Network::computeRoutingTable(int source) const {
    return computeRoutingTableFromGraph(source, buildGraphFromLSDB(source));
}

void Network::printRoutingTable(int source) const {

    const int infinity = std::numeric_limits<int>::max() / 4;
    std::vector<RouteEntry> table = computeRoutingTable(source);

    std::cout
        << "\n========== ROUTING TABLE FOR ROUTER "
        << source
        << " ==========\n";

    std::cout
        << "Destination    Next Hop       Cost      Path\n";

    std::cout << "------------------------------------------------------------\n";

    for (const RouteEntry& route : table) {

        std::cout << route.destination << "              ";

        if (route.nextHop == -1) {
            std::cout << "-              ";
        } else {
            std::cout << route.nextHop << "              ";
        }

        if (route.cost == infinity) {
            std::cout << "INF       Unreachable\n";
        } else {
            std::cout << route.cost << "         ";

            for (size_t i = 0; i < route.path.size(); ++i) {
                if (i > 0) {
                    std::cout << " -> ";
                }

                std::cout << route.path[i];
            }

            std::cout << "\n";
        }
    }
}

void Network::printAllRoutingTables() const {

    for (int routerId : getSortedRouterIds()) {
        printRoutingTable(routerId);
    }
}

void Network::printTopology() const {

    std::cout << "\n========== NETWORK TOPOLOGY ==========\n";

    for (const auto& entry : routers) {

        const Router& router = entry.second;

        router.printNeighbors();
    }

    std::cout << "======================================\n";
}

void Network::printAllLSDBs() const {

    std::cout << "\n========== ALL ROUTER LSDBs ==========\n";

    for (const auto& entry : routers) {

        entry.second.printLSDB();
    }

    std::cout << "\n======================================\n";
}

void Network::exportTopologyCSV(const std::string& filename) const {

    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cout << "Error: Could not create " << filename << "\n";
        return;
    }

    file << "source,destination,cost\n";

    for (const auto& entry : routers) {

        int source = entry.first;

        const Router& router = entry.second;

        for (const auto& neighbor : router.getNeighbors()) {

            int destination = neighbor.first;
            int cost = neighbor.second;

            // Write each undirected link only once
            if (source < destination) {

                file
                    << source
                    << ","
                    << destination
                    << ","
                    << cost
                    << "\n";
            }
        }
    }

    file.close();

    std::cout
        << "\nTopology exported to "
        << filename
        << "\n";
}

void Network::exportRoutingTablesCSV(const std::string& filename) const {

    const int infinity = std::numeric_limits<int>::max() / 4;
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cout << "Error: Could not create " << filename << "\n";
        return;
    }

    file << "source,destination,next_hop,cost,path\n";

    for (int source : getSortedRouterIds()) {

        std::vector<RouteEntry> table = computeRoutingTable(source);

        for (const RouteEntry& route : table) {

            file
                << source
                << ","
                << route.destination
                << ",";

            if (route.nextHop != -1) {
                file << route.nextHop;
            }

            file << ",";

            if (route.cost == infinity) {
                file << "INF";
            } else {
                file << route.cost;
            }

            file << ",";

            for (size_t i = 0; i < route.path.size(); ++i) {
                if (i > 0) {
                    file << "->";
                }

                file << route.path[i];
            }

            file << "\n";
        }
    }

    file.close();

    std::cout
        << "Routing tables exported to "
        << filename
        << "\n";
}

int Network::getUndirectedLinkCount() const {

    int directedLinks = 0;

    for (const auto& entry : routers) {
        directedLinks += static_cast<int>(entry.second.getNeighbors().size());
    }

    return directedLinks / 2;
}

void Network::runMeasurementExperiments(
    const std::string& spfFilename,
    const std::string& convergenceFilename,
    int minRouters,
    int maxRouters,
    int step,
    int runsPerSize
) {

    std::ofstream spfFile(spfFilename);
    std::ofstream convergenceFile(convergenceFilename);

    if (!spfFile.is_open() || !convergenceFile.is_open()) {
        std::cout << "Error: Could not create measurement CSV files.\n";
        return;
    }

    spfFile
        << "network_size,edge_count,run,spf_runs,total_time_us,"
        << "avg_time_us,checksum\n";

    convergenceFile
        << "network_size,edge_count,run,total_flooding_iterations,"
        << "max_lsa_iterations,lsdb_synchronized\n";

    std::mt19937 random(42);

    for (int size = minRouters; size <= maxRouters; size += step) {

        for (int run = 1; run <= runsPerSize; ++run) {

            Network network;

            for (int router = 0; router < size; ++router) {
                network.addRouter(router);
            }

            for (int router = 0; router < size; ++router) {
                int next = (router + 1) % size;
                int cost = 1 + static_cast<int>(random() % 20);
                network.addLink(router, next, cost, false);
            }

            int targetEdges = size + std::max(1, size / 2);
            std::set<std::pair<int, int>> existingEdges;

            for (int router = 0; router < size; ++router) {
                existingEdges.insert({
                    std::min(router, (router + 1) % size),
                    std::max(router, (router + 1) % size)
                });
            }

            while (static_cast<int>(existingEdges.size()) < targetEdges) {

                int a = static_cast<int>(random() % size);
                int b = static_cast<int>(random() % size);

                if (a == b) {
                    continue;
                }

                std::pair<int, int> edge = {std::min(a, b), std::max(a, b)};

                if (existingEdges.find(edge) != existingEdges.end()) {
                    continue;
                }

                int cost = 1 + static_cast<int>(random() % 20);
                network.addLink(a, b, cost, false);
                existingEdges.insert(edge);
            }

            network.generateAllLSAs(false);
            FloodResult floodResult = network.floodLSAs(false);
            bool synchronized = network.areLSDBsSynchronized();

            int maxIterations = 0;

            for (const auto& entry : floodResult.iterationsByOrigin) {
                maxIterations = std::max(maxIterations, entry.second);
            }

            convergenceFile
                << size
                << ","
                << network.getUndirectedLinkCount()
                << ","
                << run
                << ","
                << floodResult.totalIterations
                << ","
                << maxIterations
                << ","
                << (synchronized ? "true" : "false")
                << "\n";

            long long checksum = 0;
            int spfRuns = 0;

            auto start = std::chrono::high_resolution_clock::now();

            for (int source = 0; source < size; ++source) {

                std::vector<RouteEntry> table =
                    network.computeRoutingTable(source);

                spfRuns++;

                for (const RouteEntry& route : table) {
                    if (!route.path.empty()) {
                        checksum += route.cost;
                        checksum += route.nextHop;
                    }
                }
            }

            auto end = std::chrono::high_resolution_clock::now();

            long long totalTimeUs =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    end - start
                ).count();

            double avgTimeUs =
                spfRuns == 0
                    ? 0.0
                    : static_cast<double>(totalTimeUs) / spfRuns;

            spfFile
                << size
                << ","
                << network.getUndirectedLinkCount()
                << ","
                << run
                << ","
                << spfRuns
                << ","
                << totalTimeUs
                << ","
                << avgTimeUs
                << ","
                << checksum
                << "\n";
        }
    }

    spfFile.close();
    convergenceFile.close();

    std::cout
        << "\nSPF measurements exported to "
        << spfFilename
        << "\n";

    std::cout
        << "Convergence measurements exported to "
        << convergenceFilename
        << "\n";
}
