#ifndef ROUTER_H
#define ROUTER_H

#include "LSA.h"
#include <unordered_map>
#include <iostream>

class Router {
private:
    int id;

    // Neighbor router ID -> link cost
    std::unordered_map<int, int> neighbors;

    // Origin router ID -> latest LSA
    std::unordered_map<int, LSA> lsdb;

    // Sequence number for this router's own LSA
    int sequenceNumber;

public:
    Router();

    Router(int id);

    int getId() const;

    void addNeighbor(int neighborId, int cost);

    void removeNeighbor(int neighborId);

    bool hasNeighbor(int neighborId) const;

    const std::unordered_map<int, int>& getNeighbors() const;

    LSA generateLSA();

    bool receiveLSA(const LSA& lsa);

    bool hasLSA(int originRouter, int sequenceNumber) const;

    const std::unordered_map<int, LSA>& getLSDB() const;

    void printNeighbors() const;

    void printLSDB() const;
};

#endif