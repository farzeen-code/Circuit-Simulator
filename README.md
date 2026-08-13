# ⚡ High-Performance Digital Logic Circuit Simulator

A lightweight, cache-optimized C++ digital logic evaluation engine designed to simulate logic gate networks, evaluate boolean signal propagation, and detect combinational feedback loops.

Built to showcase core concepts in **Electronic Design Automation (EDA)**: graph algorithms, memory management, and modern C++ design patterns.

---

## 🛠 Key Features (Current Implementation)

* **Custom Fixed-Size Memory Pool Allocator:** Implements placement `new` and `union`-based free-lists to pre-allocate $4\text{ KB}$ memory chunks. Bypasses standard system heap `new`/`delete` fragmentation for $\mathcal{O}(1)$ node allocation and cache-friendly prefetching.
* **Graph Dependency Evaluation Engine:** Represents logic gates as nodes and interconnect wires as directed edges within a Directed Acyclic Graph (DAG) structure.
* **Topological Sorting (Kahn's Algorithm):** Employs Breadth-First Search (BFS) in-degree tracking to establish the exact logical sequence required for signal evaluation while instantly detecting feedback loops (short circuits).
* **Multi-Gate Support:** Evaluates standard Boolean logic operations including `INPUT`, `OUTPUT`, `AND`, `NAND`, `OR`, `NOR`, `NOT`, and `XOR`.

---

## 📁 Architecture Overview

```text
CircuitSimulator/
├── CMakeLists.txt         # Build system configuration
└── src/
    ├── Node.h             # Gate data structure & GateType enum class
    ├── MemoryPool.h       # Custom arena/pool allocator template
    ├── GraphEngine.h      # Graph network, topological sort, & evaluation pass
    └── main.cpp           # Testbench / Driver program