# GrafMaster
## Graph Splitter – Balanced & Connected Graph Partitioning

This C program reads a graph from a file (adjacency list format) and splits it into `N` connected subgraphs, aiming to keep the number of vertices in each part as equal as possible.

### Key Features

- Supports large graphs (tested with 10,000+ vertices)
- Splits the graph into **N connected components**
- Controls the **vertex count imbalance** between subgraphs (user-defined percentage tolerance)
- Exports results to **text** and **compact binary** formats
- Guarantees: no vertex loss, no disconnected components

### Input

- Graph stored in adjacency list format (custom text file)
- Optionally supports multi-graph input (via extra lines in the file)

### Output

- Individual files for each subgraph (e.g., `graph_0.txt`, `graph_1.txt`, ...)
- Optional binary file (`.bin`) with minimal size for fast storage/loading
- Original graph export: `graph_original.csv`

### Build & Run

```bash
gcc -o graphsplit main.c -std=c99
./graphsplit input.txt -a outputPrefix -b graph.bin

