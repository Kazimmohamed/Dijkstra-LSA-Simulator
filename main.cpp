#include "Network.h"
#include <iostream>

int main() {

    Network network;

    // ==========================================
    // CREATE NETWORK
    // ==========================================

    network.addRouter(0);
    network.addRouter(1);
    network.addRouter(2);
    network.addRouter(3);

    // Topology:
    //
    //        2
    //   0 -------- 1
    //   |          |
    //  5|          |1
    //   |          |
    //   2 -------- 3
    //        2

    network.addLink(0, 1, 2);
    network.addLink(0, 2, 5);
    network.addLink(1, 3, 1);
    network.addLink(2, 3, 2);

    // ==========================================
    // SHOW INITIAL TOPOLOGY
    // ==========================================

    network.printTopology();

    // ==========================================
    // GENERATE INITIAL LSAs
    // ==========================================

    network.generateAllLSAs();

    // ==========================================
    // FLOOD INITIAL LSAs
    // ==========================================

    network.floodLSAs();

    // ==========================================
    // SHOW INITIAL LSDBs
    // ==========================================

    network.printAllLSDBs();

    // ==========================================
    // REMOVE LINK
    // ==========================================

    std::cout << "\n\n========== TOPOLOGY CHANGE ==========\n";

    std::cout << "\nRemoving link 1 <-> 3...\n";

    network.removeLink(1, 3);

    network.printTopology();

    // ==========================================
    // GENERATE UPDATED LSAs
    // ==========================================

    network.generateAllLSAs();

    // ==========================================
    // FLOOD UPDATED LSAs
    // ==========================================

    network.floodLSAs();

    // ==========================================
    // SHOW UPDATED LSDBs
    // ==========================================

    network.printAllLSDBs();

    // ==========================================
    // ADD LINK BACK
    // ==========================================

    std::cout << "\n\n========== ADDING LINK BACK ==========\n";

    network.addLink(1, 3, 1);

    network.printTopology();

    // ==========================================
    // GENERATE NEW LSAs
    // ==========================================

    network.generateAllLSAs();

    // ==========================================
    // FLOOD NEW LSAs
    // ==========================================

    network.floodLSAs();

    // ==========================================
    // SHOW FINAL LSDBs
    // ==========================================

    network.printAllLSDBs();

    // ==========================================
    // END
    // ==========================================
    network.exportTopologyCSV("topology.csv");
    std::cout << "\nSimulation completed.\n";

    return 0;
}