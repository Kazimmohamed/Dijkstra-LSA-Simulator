#include "Router.h"

Router::Router()
    : id(-1),
      sequenceNumber(0) {
}

Router::Router(int id)
    : id(id),
      sequenceNumber(0) {
}

int Router::getId() const {
    return id;
}

void Router::addNeighbor(int neighborId, int cost) {
    neighbors[neighborId] = cost;
}

void Router::removeNeighbor(int neighborId) {
    neighbors.erase(neighborId);
}

bool Router::hasNeighbor(int neighborId) const {
    return neighbors.find(neighborId) != neighbors.end();
}

const std::unordered_map<int, int>& Router::getNeighbors() const {
    return neighbors;
}

LSA Router::generateLSA() {
    sequenceNumber++;

    std::vector<std::pair<int, int>> links;

    for (const auto& entry : neighbors) {
        links.push_back({entry.first, entry.second});
    }

    return LSA(id, sequenceNumber, links);
}

bool Router::receiveLSA(const LSA& lsa) {

    auto it = lsdb.find(lsa.originRouter);

    // LSA does not exist yet
    if (it == lsdb.end()) {
        lsdb[lsa.originRouter] = lsa;
        return true;
    }

    // Newer LSA received
    if (lsa.sequenceNumber > it->second.sequenceNumber) {
        lsdb[lsa.originRouter] = lsa;
        return true;
    }

    // Older or duplicate LSA
    return false;
}

bool Router::hasLSA(int originRouter, int sequenceNumber) const {

    auto it = lsdb.find(originRouter);

    if (it == lsdb.end()) {
        return false;
    }

    return it->second.sequenceNumber >= sequenceNumber;
}

const std::unordered_map<int, LSA>& Router::getLSDB() const {
    return lsdb;
}

void Router::printNeighbors() const {

    std::cout << "Router " << id << " neighbours:\n";

    if (neighbors.empty()) {
        std::cout << "  None\n";
        return;
    }

    for (const auto& entry : neighbors) {
        std::cout
            << "  Router "
            << entry.first
            << " (cost "
            << entry.second
            << ")\n";
    }
}

void Router::printLSDB() const {

    std::cout << "\nRouter " << id << " LSDB:\n";

    if (lsdb.empty()) {
        std::cout << "  Empty\n";
        return;
    }

    for (const auto& entry : lsdb) {

        const LSA& lsa = entry.second;

        std::cout
            << "  LSA from Router "
            << lsa.originRouter
            << " | Sequence "
            << lsa.sequenceNumber
            << "\n";

        for (const auto& link : lsa.links) {

            std::cout
                << "      -> Router "
                << link.first
                << " | Cost "
                << link.second
                << "\n";
        }
    }
}