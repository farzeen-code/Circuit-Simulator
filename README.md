# Event-Driven Logic Circuit Simulator (DES Engine)

A high-performance, cycle-accurate digital logic simulation engine implemented in modern C++ (C++17). The engine parses textual netlist descriptions, constructs an in-memory Directed Acyclic Graph (DAG) using a custom fixed-block memory pool allocator, simulates sub-nanosecond signal propagation delays using a discrete-event priority queue, and exports IEEE 1364 Value Change Dump (`.vcd`) waveforms for digital logic analyzers.

---

## Architectural Layers


```

Layer 1: Static Graph Compiler & Custom Memory Pool Allocator
│
▼
Layer 2: Discrete Event Simulation (DES) Engine (Nanosecond Gate Delays)
│
▼
Layer 2.5: IEEE 1364 VCD Waveform Generation (GTKWave / WaveTrace)
│
▼
Layer 3: Sequential Logic Engine, Periodic Clock Generator & D Flip-Flops

```

---

## Module Breakdown

| Layer | Component | Header | Responsibility |
| :---: | :--- | :--- | :--- |
| **1** | **Memory Pool** | `MemoryPool.h` | High-throughput fixed-size block allocator managing node object lifecycles without heap churn. |
| **1** | **Node** | `Node.h` | Core data structure representing gates, clock pins, propagation delays, and pin connections. |
| **1** | **Graph Engine** | `GraphEngine.h` | Manages node connectivity, wire insertion, and in-degree tracking. |
| **1** | **Netlist Parser** | `Parser.h` | Lexes and parses text netlists for combinational gates, clocked pins, and sequential elements. |
| **2** | **Event Simulator** | `EventSimulator.h` | Priority queue event dispatcher executing timed transitions and gate evaluations across nanoseconds. |
| **2.5**| **VCD Writer** | `VcdWriter.h` | Formatter generating standard IEEE 1364 waveform trace files compatible with logic analyzers. |
| **3** | **Clock & Sequential** | `EventSimulator.h` | Multi-clock generator and edge-triggered storage element evaluation ($0 \rightarrow 1$ transition detection). |

---

## Layer-by-Layer Implementation

### Layer 1: Memory Pool & Graph Compiler
- **`fixedPoolAllocator<T>`:** Custom memory manager that allocates contiguous memory chunks, eliminating dynamic allocation overhead (`malloc`/`new`) and pointer fragmentation during circuit graph construction.
- **`circuitGraph`:** Builds an in-memory Directed Acyclic Graph (DAG) with fan-in and fan-out adjacency lists to model electrical wire routing.

### Layer 2: Discrete Event Simulator (DES)
- **Time Wheel / Priority Queue:** Simulates true physical time using an $O(\log N)$ min-priority queue ordered by timestamp.
- **Propagation Delays:** Assigns specific physical switching latencies to logic gates:
  - Basic gates (`AND`, `OR`, `NOT`, `NAND`, `NOR`): $2\text{ ns}$ delay
  - Complex gates (`XOR`): $3\text{ ns}$ delay

### Layer 2.5: IEEE 1364 VCD Waveform Exporter
- Compact ASCII symbol aliasing (mapping long signal names like `COUT` to 1-character tokens like `!`, `"`, `#`).
- Real-time timestamp logging (`#<time>`) and digital state dumps (`0`/`1`) compatible with standard EDA waveform viewers.

### Layer 3: Sequential Logic & Clock Engine
- **Periodic Clock Source (`register_clock`):** Automates continuous square-wave oscillations across the time domain without manual event scheduling.
- **Edge-Triggered D Flip-Flop (`DFF`):** 
  - Tracks previous clock states (`prev_clk_value`) to detect **rising edges** ($0 \rightarrow 1$).
  - Ignores data changes ($D$) during steady clock phases, capturing and latching input data exclusively on the rising edge with a $1\text{ ns}$ internal propagation delay.

---

## Circuits Implemented & Verified

### 1. Combinational Logic: 1-Bit Full Adder
Implements a 1-bit full adder using two cascaded XOR gates, two AND gates, and an OR gate to compute $\text{SUM} = A \oplus B \oplus C_{in}$ and $\text{COUT} = (A \cdot B) + (S_1 \cdot C_{in})$.


#### Netlist (`circuit.txt`):
```text
INPUT A B Cin

XOR S1 A B
XOR SUM S1 Cin

AND C1 A B
AND C2 S1 Cin
OR COUT C1 C2

OUTPUT SUM COUT

```

#### Full Adder Waveform Simulation:

<img width="1755" height="424" alt="Screenshot 2026-08-18 221700" src="https://github.com/user-attachments/assets/51b94a7e-89e8-4a75-895d-2abcdb0bfd59" />


---

### 2. Sequential Logic: 1-Bit D Flip-Flop Memory Register

Demonstrates hardware memory retention and clock-synchronized state updates.


#### Netlist (`dff_circuit.txt`):

```text
INPUT D
CLOCK CLK

DFF Q D CLK

OUTPUT Q

```

#### Stimulus Scenario:

1. **Clock:** Configured with a $10\text{ ns}$ period ($5\text{ ns}$ half-period), producing rising edges at $T = 5\text{ ns}, 15\text{ ns}, 25\text{ ns}, \dots$
2. **$T = 3\text{ ns}$:** Data input $D \rightarrow 1$. Output $Q$ remains $0$ (holds previous state).
3. **$T = 5\text{ ns}$:** Rising clock edge arrives. $D=1$ is sampled.
4. **$T = 6\text{ ns}$ ($5\text{ ns} + 1\text{ ns}$ delay):** Output $Q$ transitions to $1$.
5. **$T = 12\text{ ns}$:** Data input $D \rightarrow 0$. Output $Q$ holds $1$ in memory until the next clock edge at $T = 15\text{ ns}$.

#### D Flip-Flop Waveform Simulation:

<img width="1851" height="823" alt="Screenshot 2026-08-18 230549" src="https://github.com/user-attachments/assets/c498a5ba-302f-46da-bb73-68f9920bf0c7" />


---

## Build & Execution Instructions

### Prerequisites

* **C++ Compiler:** GCC (MinGW-w64 on Windows) or Clang with C++17 support
* **Build System:** CMake (v3.15+)

### Build

```powershell
# 1. Configure the project
cmake -B build -G "MinGW Makefiles"

# 2. Compile the executable
cmake --build build

```

### Run

```powershell
# Execute the simulation engine and produce waveform.vcd
.\build\CircuitSimulator.exe

```

### Viewing Waveforms

Open `waveform.vcd` directly in VS Code using the **WaveTrace** extension or launch with **GTKWave**:

```powershell
gtkwave waveform.vcd

```

---

## Roadmap

* [x] **Layer 1:** Static Graph Compiler & Fixed-Block Memory Pool Allocator
* [x] **Layer 2:** Discrete Event Simulator (DES) with Nanosecond Propagation Delays
* [x] **Layer 2.5:** IEEE 1364 VCD Waveform Exporter
* [x] **Layer 3:** Periodic Clock Generator & D Flip-Flop Sequential Memory
* [ ] **Layer 4:** Static Hazard & Dynamic Glitch Detection (Race condition analysis)
* [ ] **Layer 5:** Multi-Bit Bus Syntax & 4-Bit ALU Synthesizer

---

