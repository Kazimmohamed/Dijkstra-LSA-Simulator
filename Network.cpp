#include "Network.h"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <fstream>

void Network::addRouter(int id) {

    if (routers.find(id) == routers.end()) {
        routers.emplace(id, Router(id));
    }
}

void Network::addLink(int routerA, int routerB, int cost) {

    addRouter(routerA);
    addRouter(routerB);

    routers.at(routerA).addNeighbor(routerB, cost);
    routers.at(routerB).addNeighbor(routerA, cost);

    std::cout
        << "Link added: "
        << routerA
        << " <-> "
        << routerB
        << " (cost "
        << cost
        << ")\n";
}

void Network::removeLink(int routerA, int routerB) {

    if (routers.find(routerA) == routers.end()) {
        return;
    }

    if (routers.find(routerB) == routers.end()) {
        return;
    }

    routers.at(routerA).removeNeighbor(routerB);
    routers.at(routerB).removeNeighbor(routerA);

    std::cout
        << "\nLink removed: "
        << routerA
        << " <-> "
        << routerB
        << "\n";
}

Router* Network::getRouter(int id) {

    auto it = routers.find(id);

    if (it == routers.end()) {
        return nullptr;
    }

    return &it->second;
}

void Network::generateAllLSAs() {

    std::cout << "\nGenerating LSAs...\n";

    for (auto& entry : routers) {

        Router& router = entry.second;

        LSA lsa = router.generateLSA();

        router.receiveLSA(lsa);

        std::cout
            << "Router "
            << router.getId()
            << " generated LSA #"
            << lsa.sequenceNumber
            << "\n";
    }
}

void Network::floodLSAs() {

    std::cout << "\nStarting LSA flooding...\n";

    int totalIterations = 0;

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

            std::cout
                << "\n  LSA "
                << lsa.originRouter
                << "#"
                << lsa.sequenceNumber
                << " - Flooding round "
                << iterationsForThisLSA
                << ":\n";

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

                        std::cout
                            << "      "
                            << currentId
                            << " -> "
                            << neighborId
                            << "\n";

                        nextRound.push(neighborId);
                    }
                }
            }

            currentRound = std::move(nextRound);
        }

        totalIterations += iterationsForThisLSA;

        std::cout
            << "  LSA "
            << lsa.originRouter
            << "#"
            << lsa.sequenceNumber
            << " converged in "
            << iterationsForThisLSA
            << " flooding rounds.\n";
    }

    std::cout
        << "\nLSA flooding completed.\n";

    std::cout
        << "Total flooding iterations: "
        << totalIterations
        << "\n";
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