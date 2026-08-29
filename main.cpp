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

    std::cout
        << "\nInitial LSDB synchronized: "
        << (network.areLSDBsSynchronized() ? "yes" : "no")
        << "\n";

    network.printAllRoutingTables();

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

    std::cout
        << "\nUpdated LSDB synchronized: "
        << (network.areLSDBsSynchronized() ? "yes" : "no")
        << "\n";

    network.printAllRoutingTables();

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

    std::cout
        << "\nFinal LSDB synchronized: "
        << (network.areLSDBsSynchronized() ? "yes" : "no")
        << "\n";

    network.printAllRoutingTables();

    // ==========================================
    // END
    // ==========================================
    network.exportTopologyCSV("topology.csv");
    network.exportRoutingTablesCSV("routing_tables.csv");
    Network::runMeasurementExperiments(
        "spf_results.csv",
        "convergence_results.csv",
        4,
        64,
        4,
        5
    );

    std::cout << "\nSimulation completed.\n";

    return 0;
}
