import requests
import json
from graphviz import Graph, Digraph

def fetch_kraken_pairs():
    url = "https://api.kraken.com/0/public/AssetPairs"
    response = requests.get(url)
    data = response.json()
    
    if data["error"]:
        raise Exception(f"Error fetching data from Kraken API: {data['error']}")
    
    pairs = data["result"]
    
    formatted_pairs = list({k: v['wsname'] for k, v in pairs.items()}.values())
    pairrs = {}
    for p in formatted_pairs: 
        k = "BTC" if p.split('/')[0] == "XBT" else p.split('/')[0]

        if k not in pairrs.keys():
            pairrs[k] = []
        pairrs[k].append("BTC" if p.split('/')[1] == "XBT" else p.split('/')[1])
    # print(pairrs)

    return pairrs

def select_currencies_forming_triangles(currency_pairs):
    triangles = set()
    for base_currency in currency_pairs.keys():
        for quote_currency in currency_pairs[base_currency]:
            if quote_currency in currency_pairs.keys():
                for potential_third_currency in currency_pairs[quote_currency]:
                    if (potential_third_currency in currency_pairs[base_currency]) or (potential_third_currency in currency_pairs.keys() and base_currency in currency_pairs[potential_third_currency]):
                        triangle = tuple(sorted([base_currency, quote_currency, potential_third_currency]))
                        triangles.add(triangle)
    return list(triangles)

def count_connections(triangles):
    connections_count = {}
    for triangle in triangles:
        for i in range(3):
            node = triangle[i]
            if node not in connections_count:
                connections_count[node] = set()
            connections_count[node].add(triangle[(i + 1) % 3])
            connections_count[node].add(triangle[(i + 2) % 3])
    return connections_count

def filter_nodes(connections_count, min_connections):
    filtered_nodes = {node for node, neighbors in connections_count.items() if len(neighbors) >= min_connections}
    return filtered_nodes

def filter_graph(triangles, min_connections):
    connections_count = count_connections(triangles)
    filtered_nodes = filter_nodes(connections_count, min_connections)
    
    while True:
        initial_count = len(filtered_nodes)
        connections_count = {node: neighbors for node, neighbors in connections_count.items() if node in filtered_nodes}
        
        for node in list(connections_count.keys()):
            connections_count[node] = {neighbor for neighbor in connections_count[node] if neighbor in filtered_nodes}
            if len(connections_count[node]) < min_connections:
                filtered_nodes.discard(node)
        
        if len(filtered_nodes) == initial_count:
            break

    return filtered_nodes

def make_graph(triangles, filtered_nodes, pairs):
    graph_draw = Graph(comment='Triangular Arbitrage Opportunities')
    drawn_edges = set()
    directed_edges = {}

    # Set graph size and ratio for square page layout
    graph_draw.attr(size='100,100')
    graph_draw.attr(ratio='1.0')

    # Define node attributes for size and fontsize
    node_attr = {'shape': 'circle', 'width': '1.0', 'height': '1.0', 'fontsize': '35','fontname': 'Times'}
    graph_draw.attr('node', **node_attr)

    # Draw graph with filtered nodes
    for triangle in triangles:
        if all(node in filtered_nodes for node in triangle):
            for i in range(3):
                graph_draw.node(triangle[i], color='purple')
                edge = (triangle[i], triangle[(i + 1) % 3])
                reverse_edge = (triangle[(i + 1) % 3], triangle[i])
                
                if (edge[0] in pairs.keys() and edge[1] in pairs[edge[0]]):
                    if edge not in drawn_edges and reverse_edge not in drawn_edges:
                        graph_draw.edge(triangle[i], triangle[(i + 1) % 3], color='black')
                        drawn_edges.add(edge)

                        # Add to directed edges dictionary
                        if triangle[i] not in directed_edges:
                            directed_edges[triangle[i]] = set()
                        directed_edges[triangle[i]].add(triangle[(i + 1) % 3])

                elif (edge[1] in pairs.keys() and edge[0] in pairs[edge[1]]):
                    if edge not in drawn_edges and reverse_edge not in drawn_edges:
                        graph_draw.edge(edge[1], edge[0], color='black')
                        drawn_edges.add(reverse_edge)

                        # Add to directed edges dictionary
                        if edge[1] not in directed_edges:
                            directed_edges[edge[1]] = set()
                        directed_edges[edge[1]].add(edge[0])

    graph_draw.render('./test-output.gv', view=True)
    return directed_edges


if __name__ == "__main__":
    currency_pairs = fetch_kraken_pairs()
    triangles = select_currencies_forming_triangles(currency_pairs)
    filtered_nodes = filter_graph(triangles, 7)
    print(f'Number of nodes with or more connections: {len(filtered_nodes)}')
    directed_edges = make_graph(triangles, filtered_nodes, currency_pairs)
    # print(directed_edges)
    # Format directed edges as requested
    formatted_edges = "{\n"
    for k, v in directed_edges.items():
        targets = ", ".join(f'"{target}"' for target in v)
        formatted_edges += f'    {{"{k}", {{{targets}}}}},\n'
    formatted_edges = formatted_edges.rstrip(',\n') + "\n}"
    

    # print(formatted_edges)

    # Initialize the counter
    total_entries = 0

    # Iterate through the dictionary and count the entries
    for key, value_set in directed_edges.items():
        total_entries += len(value_set)

    # Print the total number of entries
    print("Total number of entries:", total_entries)

    to_write = []
    for k, vs in directed_edges.items():
        for v in vs:
            to_write.append(f"\"{k}/{v}\",")

    with open('directed_edges.txt', 'w') as file:
        file.write("{ ")
        file.write("".join(to_write))
        file.write(" };\n")

        file.write(formatted_edges)
        # print(len(to_write))

    