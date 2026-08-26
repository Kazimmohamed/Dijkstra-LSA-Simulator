#ifndef LSA_H
#define LSA_H

#include <vector>
#include <utility>

class LSA {
public:
    int originRouter;
    int sequenceNumber;
    std::vector<std::pair<int, int>> links;

    LSA();

    LSA(int originRouter,
        int sequenceNumber,
        const std::vector<std::pair<int, int>>& links);
};

#endif