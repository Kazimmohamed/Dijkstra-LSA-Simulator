import csv
import heapq
import os
from collections import defaultdict


class Network:

    def __init__(self):
        self.graph = defaultdict(dict)

    # ------------------------------------------
    # Load topology from topology.csv
    # ------------------------------------------
    def load_topology(self, filename):

        try:
            with open(filename, "r") as file:

                reader = csv.DictReader(file)

                for row in reader:

                    source = int(row["source"])
                    destination = int(row["destination"])
                    cost = int(row["cost"])

                    # Undirected network
                    self.graph[source][destination] = cost
                    self.graph[destination][source] = cost

            print("Topology loaded successfully.")

        except FileNotFoundError:
            print(f"Error: {filename} not found.")

    # ------------------------------------------
    # Display topology
    # ------------------------------------------
    def print_topology(self):

        print("\n========== NETWORK TOPOLOGY ==========")

        printed_links = set()

        for router in sorted(self.graph):

            print(f"\nRouter {router} neighbours:")

            for neighbor, cost in self.graph[router].items():

                link = tuple(sorted((router, neighbor)))

                if link not in printed_links:

                    print(
                        f"  Router {neighbor} "
                        f"(cost {cost})"
                    )

                    printed_links.add(link)

        print("\n======================================")

    # ------------------------------------------
    # DIJKSTRA ALGORITHM
    # ------------------------------------------
    def dijkstra(self, source):

        # Distance from source to every router
        distance = {
            router: float("inf")
            for router in self.graph
        }

        # Previous router used to reconstruct path
        previous = {
            router: None
            for router in self.graph
        }

        # Distance from source to itself
        distance[source] = 0

        # Priority queue
        # (distance, router)
        priority_queue = [(0, source)]

        visited = set()

        while priority_queue:

            current_distance, current_router = heapq.heappop(
                priority_queue
            )

            # Ignore already processed routers
            if current_router in visited:
                continue

            visited.add(current_router)

            # Check all neighbours
            for neighbor, cost in self.graph[current_router].items():

                new_distance = current_distance + cost

                # Found a shorter path
                if new_distance < distance[neighbor]:

                    distance[neighbor] = new_distance

                    previous[neighbor] = current_router

                    heapq.heappush(
                        priority_queue,
                        (new_distance, neighbor)
                    )

        return distance, previous

    # ------------------------------------------
    # Construct shortest path
    # ------------------------------------------
    def get_path(self, previous, source, destination):

        path = []

        current = destination

        while current is not None:

            path.append(current)

            if current == source:
                break

            current = previous[current]

        # Destination is unreachable
        if not path or path[-1] != source:
            return []

        path.reverse()

        return path

    # ------------------------------------------
    # Print shortest paths
    # ------------------------------------------
    def print_shortest_paths(self, source):

        distance, previous = self.dijkstra(source)

        print(
            f"\n========== DIJKSTRA SPF FROM ROUTER "
            f"{source} =========="
        )

        print(
            f"{'Destination':<15}"
            f"{'Cost':<10}"
            f"Shortest Path"
        )

        print("-" * 45)

        for destination in sorted(self.graph):

            if destination == source:
                continue

            path = self.get_path(
                previous,
                source,
                destination
            )

            if distance[destination] == float("inf"):

                print(
                    f"{destination:<15}"
                    f"{'INF':<10}"
                    f"Unreachable"
                )

            else:

                path_string = " -> ".join(
                    map(str, path)
                )

                print(
                    f"{destination:<15}"
                    f"{distance[destination]:<10}"
                    f"{path_string}"
                )

        print("========================================")

    # ------------------------------------------
    # Generate routing table
    # ------------------------------------------
    def generate_routing_table(self, source):

        distance, previous = self.dijkstra(source)

        print(
            f"\n========== ROUTING TABLE FOR ROUTER "
            f"{source} =========="
        )

        print("-" * 65)

        print(
            f"{'Destination':<15}"
            f"{'Next Hop':<15}"
            f"{'Cost':<10}"
            f"Path"
        )

        print("-" * 65)

        for destination in sorted(self.graph):

            if destination == source:
                continue

            path = self.get_path(
                previous,
                source,
                destination
            )

            if not path:

                print(
                    f"{destination:<15}"
                    f"{'-':<15}"
                    f"{'INF':<10}"
                    f"Unreachable"
                )

                continue

            # First router after source
            next_hop = path[1]

            path_string = " -> ".join(
                map(str, path)
            )

            print(
                f"{destination:<15}"
                f"{next_hop:<15}"
                f"{distance[destination]:<10}"
                f"{path_string}"
            )

        print("-" * 65)


# ==================================================
# MAIN PROGRAM
# ==================================================

if __name__ == "__main__":

    # Create network
    network = Network()

    # ------------------------------------------
    # Find topology.csv in the same folder
    # as this Python file
    # ------------------------------------------

    csv_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "topology.csv"
    )

    print("\nReading topology from:")
    print(csv_path)

    # ------------------------------------------
    # Load topology
    # ------------------------------------------

    network.load_topology(csv_path)

    # ------------------------------------------
    # Display topology
    # ------------------------------------------

    network.print_topology()

    # ------------------------------------------
    # Run Dijkstra from every router
    # ------------------------------------------

    for router in sorted(network.graph):

        network.print_shortest_paths(router)

        network.generate_routing_table(router)

    print("\nDijkstra SPF calculation completed.")