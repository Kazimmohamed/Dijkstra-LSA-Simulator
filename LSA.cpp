#include "LSA.h"

LSA::LSA()
    : originRouter(-1),
      sequenceNumber(0) {
}

LSA::LSA(int originRouter,
         int sequenceNumber,
         const std::vector<std::pair<int, int>>& links)
    : originRouter(originRouter),
      sequenceNumber(sequenceNumber),
      links(links) {
}