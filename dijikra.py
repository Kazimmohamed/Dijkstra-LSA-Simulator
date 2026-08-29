import csv
import heapq
import math
from pathlib import Path
from collections import defaultdict

try:
    import matplotlib.pyplot as plt
    HAS_MATPLOTLIB = True
except ModuleNotFoundError:
    plt = None
    HAS_MATPLOTLIB = False


class Network:

    def __init__(self):
        self.graph = defaultdict(dict)

    def load_topology(self, filename):

        try:
            with open(filename, "r", newline="") as file:
                reader = csv.DictReader(file)

                for row in reader:
                    source = int(row["source"])
                    destination = int(row["destination"])
                    cost = int(row["cost"])

                    self.graph[source][destination] = cost
                    self.graph[destination][source] = cost

            print("Topology loaded successfully.")

        except FileNotFoundError:
            print(f"Error: {filename} not found.")

    def print_topology(self):

        print("\n========== NETWORK TOPOLOGY ==========")

        for router in sorted(self.graph):
            print(f"\nRouter {router} neighbours:")

            for neighbor, cost in self.graph[router].items():
                print(f"  Router {neighbor} (cost {cost})")

        print("\n======================================")

    def dijkstra(self, source):

        distance = {router: float("inf") for router in self.graph}
        previous = {router: None for router in self.graph}
        distance[source] = 0

        priority_queue = [(0, source)]
        visited = set()

        while priority_queue:
            current_distance, current_router = heapq.heappop(priority_queue)

            if current_router in visited:
                continue

            visited.add(current_router)

            for neighbor, cost in self.graph[current_router].items():
                new_distance = current_distance + cost

                if new_distance < distance[neighbor]:
                    distance[neighbor] = new_distance
                    previous[neighbor] = current_router
                    heapq.heappush(priority_queue, (new_distance, neighbor))

        return distance, previous

    def get_path(self, previous, source, destination):

        path = []
        current = destination

        while current is not None:
            path.append(current)

            if current == source:
                break

            current = previous[current]

        if not path or path[-1] != source:
            return []

        path.reverse()
        return path

    def print_shortest_paths(self, source):

        distance, previous = self.dijkstra(source)

        print(f"\n========== DIJKSTRA SPF FROM ROUTER {source} ==========")
        print(f"{'Destination':<15}{'Cost':<10}Shortest Path")
        print("-" * 45)

        for destination in sorted(self.graph):
            if destination == source:
                continue

            path = self.get_path(previous, source, destination)

            if distance[destination] == float("inf"):
                print(f"{destination:<15}{'INF':<10}Unreachable")
            else:
                path_string = " -> ".join(map(str, path))
                print(f"{destination:<15}{distance[destination]:<10}{path_string}")

        print("========================================")

    def generate_routing_table(self, source):

        distance, previous = self.dijkstra(source)

        print(f"\n========== ROUTING TABLE FOR ROUTER {source} ==========")
        print("-" * 65)
        print(f"{'Destination':<15}{'Next Hop':<15}{'Cost':<10}Path")
        print("-" * 65)

        for destination in sorted(self.graph):
            if destination == source:
                continue

            path = self.get_path(previous, source, destination)

            if not path:
                print(f"{destination:<15}{'-':<15}{'INF':<10}Unreachable")
                continue

            next_hop = path[1]
            path_string = " -> ".join(map(str, path))
            print(f"{destination:<15}{next_hop:<15}{distance[destination]:<10}{path_string}")

        print("-" * 65)


def read_csv_rows(filename):

    path = Path(__file__).resolve().parent / filename

    if not path.exists():
        print(f"Warning: {filename} not found. Run the C++ simulator first.")
        return []

    with path.open("r", newline="") as file:
        return list(csv.DictReader(file))


def plot_topology(network, output_filename):

    if not network.graph:
        print("Warning: topology graph is empty; skipping topology plot.")
        return

    routers = sorted(network.graph)
    positions = {}

    for index, router in enumerate(routers):
        angle = 2.0 * math.pi * index / len(routers)
        positions[router] = (math.cos(angle), math.sin(angle))

    if not HAS_MATPLOTLIB:
        svg_filename = output_filename.with_suffix(".svg")
        width = 700
        height = 700
        center = width / 2
        scale = 250
        printed_links = set()
        lines = [
            '<svg xmlns="http://www.w3.org/2000/svg" width="700" height="700" viewBox="0 0 700 700">',
            '<rect width="700" height="700" fill="white"/>',
            '<text x="350" y="35" text-anchor="middle" font-family="Arial" font-size="22">Final 4-Router LSA Topology</text>',
        ]

        for source in routers:
            for destination, cost in network.graph[source].items():
                link = tuple(sorted((source, destination)))

                if link in printed_links:
                    continue

                printed_links.add(link)
                x1 = center + positions[source][0] * scale
                y1 = center + positions[source][1] * scale
                x2 = center + positions[destination][0] * scale
                y2 = center + positions[destination][1] * scale
                mx = (x1 + x2) / 2
                my = (y1 + y2) / 2
                lines.append(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="#6c757d" stroke-width="3"/>')
                lines.append(f'<rect x="{mx - 14:.1f}" y="{my - 13:.1f}" width="28" height="22" rx="4" fill="white" stroke="#adb5bd"/>')
                lines.append(f'<text x="{mx:.1f}" y="{my + 5:.1f}" text-anchor="middle" font-family="Arial" font-size="14">{cost}</text>')

        for router in routers:
            x = center + positions[router][0] * scale
            y = center + positions[router][1] * scale
            lines.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="34" fill="#2b8a3e"/>')
            lines.append(f'<text x="{x:.1f}" y="{y + 6:.1f}" text-anchor="middle" font-family="Arial" font-size="20" font-weight="bold" fill="white">{router}</text>')

        lines.append("</svg>")
        svg_filename.write_text("\n".join(lines), encoding="utf-8")
        print(f"Topology visualization saved to {svg_filename}")
        return

    plt.figure(figsize=(7, 7))
    printed_links = set()

    for source in routers:
        for destination, cost in network.graph[source].items():
            link = tuple(sorted((source, destination)))

            if link in printed_links:
                continue

            printed_links.add(link)
            x_values = [positions[source][0], positions[destination][0]]
            y_values = [positions[source][1], positions[destination][1]]
            plt.plot(x_values, y_values, color="#6c757d", linewidth=2)
            plt.text(
                sum(x_values) / 2.0,
                sum(y_values) / 2.0,
                str(cost),
                ha="center",
                va="center",
                bbox={"boxstyle": "round,pad=0.2", "fc": "white", "ec": "#adb5bd"},
            )

    for router in routers:
        x, y = positions[router]
        plt.scatter([x], [y], s=900, color="#2b8a3e", zorder=3)
        plt.text(
            x,
            y,
            str(router),
            ha="center",
            va="center",
            color="white",
            fontsize=13,
            fontweight="bold",
            zorder=4,
        )

    plt.title("Final 4-Router LSA Topology")
    plt.axis("off")
    plt.tight_layout()
    plt.savefig(output_filename, dpi=160)
    plt.close()
    print(f"Topology visualization saved to {output_filename}")


def average_by_network_size(rows, value_field):

    grouped = defaultdict(list)

    for row in rows:
        grouped[int(row["network_size"])].append(float(row[value_field]))

    return [
        (network_size, sum(values) / len(values))
        for network_size, values in sorted(grouped.items())
    ]


def write_line_chart_svg(points, output_filename, title, x_label, y_label, color):

    width = 800
    height = 500
    left = 85
    right = 35
    top = 60
    bottom = 75
    plot_width = width - left - right
    plot_height = height - top - bottom

    x_values = [point[0] for point in points]
    y_values = [point[1] for point in points]
    min_x = min(x_values)
    max_x = max(x_values)
    min_y = 0
    max_y = max(y_values) if max(y_values) > 0 else 1

    def scale_x(value):
        if max_x == min_x:
            return left + plot_width / 2

        return left + (value - min_x) / (max_x - min_x) * plot_width

    def scale_y(value):
        return top + plot_height - (value - min_y) / (max_y - min_y) * plot_height

    scaled_points = [(scale_x(x), scale_y(y)) for x, y in points]
    polyline_points = " ".join(f"{x:.1f},{y:.1f}" for x, y in scaled_points)

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        f'<rect width="{width}" height="{height}" fill="white"/>',
        f'<text x="{width / 2}" y="32" text-anchor="middle" font-family="Arial" font-size="21">{title}</text>',
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}" stroke="#343a40" stroke-width="1.5"/>',
        f'<line x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}" stroke="#343a40" stroke-width="1.5"/>',
    ]

    for tick in range(6):
        y_value = min_y + (max_y - min_y) * tick / 5
        y = scale_y(y_value)
        lines.append(f'<line x1="{left}" y1="{y:.1f}" x2="{left + plot_width}" y2="{y:.1f}" stroke="#dee2e6" stroke-width="1"/>')
        lines.append(f'<text x="{left - 10}" y="{y + 4:.1f}" text-anchor="end" font-family="Arial" font-size="12">{y_value:.1f}</text>')

    for x_value in x_values:
        x = scale_x(x_value)
        lines.append(f'<line x1="{x:.1f}" y1="{top + plot_height}" x2="{x:.1f}" y2="{top + plot_height + 5}" stroke="#343a40" stroke-width="1"/>')
        lines.append(f'<text x="{x:.1f}" y="{top + plot_height + 22}" text-anchor="middle" font-family="Arial" font-size="12">{x_value}</text>')

    lines.append(f'<polyline points="{polyline_points}" fill="none" stroke="{color}" stroke-width="3"/>')

    for index, (x, y) in enumerate(scaled_points):
        original_y = y_values[index]
        lines.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="5" fill="{color}"/>')
        lines.append(f'<text x="{x:.1f}" y="{y - 10:.1f}" text-anchor="middle" font-family="Arial" font-size="11">{original_y:.1f}</text>')

    lines.append(f'<text x="{width / 2}" y="{height - 18}" text-anchor="middle" font-family="Arial" font-size="14">{x_label}</text>')
    lines.append(f'<text x="18" y="{height / 2}" text-anchor="middle" font-family="Arial" font-size="14" transform="rotate(-90 18 {height / 2})">{y_label}</text>')
    lines.append("</svg>")

    output_filename.write_text("\n".join(lines), encoding="utf-8")


def plot_spf_results(rows, output_filename):

    if not rows:
        return

    averages = average_by_network_size(rows, "avg_time_us")

    if not HAS_MATPLOTLIB:
        svg_filename = output_filename.with_suffix(".svg")
        write_line_chart_svg(
            averages,
            svg_filename,
            "Dijkstra SPF Execution Time vs Network Size",
            "Network size (routers)",
            "Average SPF time per source (microseconds)",
            "#1c7ed6",
        )
        print(f"SPF timing visualization saved to {svg_filename}")
        return

    plt.figure(figsize=(8, 5))
    plt.plot(
        [item[0] for item in averages],
        [item[1] for item in averages],
        marker="o",
        color="#1c7ed6",
        linewidth=2,
    )
    plt.title("Dijkstra SPF Execution Time vs Network Size")
    plt.xlabel("Network size (routers)")
    plt.ylabel("Average SPF time per source (microseconds)")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.tight_layout()
    plt.savefig(output_filename, dpi=160)
    plt.close()
    print(f"SPF timing visualization saved to {output_filename}")


def plot_convergence_results(rows, output_filename):

    if not rows:
        return

    averages = average_by_network_size(rows, "total_flooding_iterations")

    if not HAS_MATPLOTLIB:
        svg_filename = output_filename.with_suffix(".svg")
        write_line_chart_svg(
            averages,
            svg_filename,
            "LSA Flooding Convergence Iterations vs Network Size",
            "Network size (routers)",
            "Average total flooding iterations",
            "#e67700",
        )
        print(f"Convergence visualization saved to {svg_filename}")
        return

    plt.figure(figsize=(8, 5))
    plt.plot(
        [item[0] for item in averages],
        [item[1] for item in averages],
        marker="o",
        color="#e67700",
        linewidth=2,
    )
    plt.title("LSA Flooding Convergence Iterations vs Network Size")
    plt.xlabel("Network size (routers)")
    plt.ylabel("Average total flooding iterations")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.tight_layout()
    plt.savefig(output_filename, dpi=160)
    plt.close()
    print(f"Convergence visualization saved to {output_filename}")


def print_cpp_measurement_summary():

    spf_rows = read_csv_rows("spf_results.csv")
    convergence_rows = read_csv_rows("convergence_results.csv")

    if spf_rows:
        print("\n========== C++ SPF TIMING SUMMARY ==========")
        print(f"{'Routers':<12}{'Avg SPF us':<15}")

        for network_size, average in average_by_network_size(spf_rows, "avg_time_us"):
            print(f"{network_size:<12}{average:<15.2f}")

    if convergence_rows:
        print("\n========== C++ CONVERGENCE SUMMARY ==========")
        print(f"{'Routers':<12}{'Avg iterations':<18}{'Synchronized'}")

        grouped_sync = defaultdict(list)

        for row in convergence_rows:
            grouped_sync[int(row["network_size"])].append(
                row["lsdb_synchronized"].lower() == "true"
            )

        for network_size, average in average_by_network_size(
            convergence_rows,
            "total_flooding_iterations",
        ):
            synchronized = all(grouped_sync[network_size])
            print(f"{network_size:<12}{average:<18.2f}{synchronized}")

    base_dir = Path(__file__).resolve().parent
    plot_spf_results(spf_rows, base_dir / "spf_execution_time.png")
    plot_convergence_results(convergence_rows, base_dir / "convergence_iterations.png")


if __name__ == "__main__":

    network = Network()
    base_dir = Path(__file__).resolve().parent
    csv_path = base_dir / "topology.csv"

    print("\nReading topology from:")
    print(csv_path)

    network.load_topology(csv_path)
    network.print_topology()

    for router in sorted(network.graph):
        network.print_shortest_paths(router)
        network.generate_routing_table(router)

    plot_topology(network, base_dir / "topology_graph.png")
    print_cpp_measurement_summary()

    print("\nDijkstra SPF calculation completed.")
