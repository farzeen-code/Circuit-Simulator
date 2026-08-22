```markdown
# Event-Driven Logic Circuit Simulator (DES Engine)

A high-performance, cycle-accurate digital logic simulation engine implemented in modern C++ (C++17). The engine parses textual netlist descriptions, constructs an in-memory Directed Acyclic Graph (DAG) using a custom fixed-block memory pool allocator, simulates sub-nanosecond signal propagation delays using a discrete-event priority queue, detects electrical timing hazards/glitches, and exports IEEE 1364 Value Change Dump (`.vcd`) waveforms for digital logic analyzers.

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
│
▼
Layer 4: Static Hazard Analysis & Glitch Detection (Race Condition Engine)
│
▼
Layer 5: Multi-Bit Bus Expander & 4-Bit Arithmetic Logic Unit (ALU)

```

---

## Module Breakdown

| Layer | Component | Header | Responsibility |
| :---: | :--- | :--- | :--- |
| **1** | **Memory Pool** | `MemoryPool.h` | High-throughput fixed-size block allocator managing node object lifecycles without heap churn. |
| **1** | **Node** | `Node.h` | Core data structure representing gates, clock pins, propagation delays, and pin connections. |
| **1** | **Graph Engine** | `GraphEngine.h` | Manages node connectivity, wire insertion, and in-degree tracking. |
| **1** | **Netlist Parser** | `Parser.h` | Lexes and parses text netlists for combinational gates, clocked pins, sequential elements, and multi-bit buses. |
| **2** | **Event Simulator** | `EventSimulator.h` | Priority queue event dispatcher executing timed transitions and gate evaluations across nanoseconds. |
| **2.5**| **VCD Writer** | `VcdWriter.h` | Formatter generating standard IEEE 1364 waveform trace files compatible with logic analyzers. |
| **3** | **Clock & Sequential** | `EventSimulator.h` | Multi-clock generator and edge-triggered storage element evaluation (0 → 1 transition detection). |
| **4** | **Hazard Engine** | `EventSimulator.h` | Real-time race condition monitor detecting static-0/static-1 glitches and transient intermediate states. |
| **5** | **Bus Synthesizer** | `Parser.h` | Regex-driven multi-bit bus token expander and vector pin scheduler supporting composite logic blocks. |

---

## Layer-by-Layer Implementation

### Layer 1: Memory Pool & Graph Compiler
- **`fixedPoolAllocator<T>`:** Custom memory manager that allocates contiguous memory chunks, eliminating dynamic allocation overhead (`malloc`/`new`) and pointer fragmentation during circuit graph construction.
- **`circuitGraph`:** Builds an in-memory Directed Acyclic Graph (DAG) with fan-in and fan-out adjacency lists to model electrical wire routing.

### Layer 2: Discrete Event Simulator (DES)
- **Time Wheel / Priority Queue:** Simulates true physical time using an O(log N) min-priority queue ordered by timestamp.
- **Propagation Delays:** Assigns specific physical switching latencies to logic gates:
  - Inverters (`NOT`): 1 ns delay
  - Basic gates (`AND`, `OR`, `NAND`, `NOR`): 2 ns delay
  - Complex gates (`XOR`): 3 ns delay

### Layer 2.5: IEEE 1364 VCD Waveform Exporter
- Compact ASCII symbol aliasing (mapping long signal names like `COUT` to 1-character tokens like `!`, `"`, `#`).
- Real-time timestamp logging (`#<time>`) and digital state dumps (`0`/`1`) compatible with standard EDA waveform viewers.

### Layer 3: Sequential Logic & Clock Engine
- **Periodic Clock Source (`register_clock`):** Automates continuous square-wave oscillations across the time domain without manual event scheduling.
- **Edge-Triggered D Flip-Flop (`DFF`):** 
  - Tracks previous clock states (`prev_clk_value`) to detect **rising edges** (0 → 1).
  - Ignores data changes (`D`) during steady clock phases, capturing and latching input data exclusively on the rising edge with a 1 ns internal propagation delay.

### Layer 4: Static Hazard & Glitch Detection
- **Glitch Threshold Profiling:** Identifies transient race conditions where a signal toggles and settles within a specified window (duration ≤ `glitch_threshold`).
- **Hazard Classification:**
  - **Static-1 Hazard (1 → 0 → 1):** Output expected to stay high momentarily drops due to asymmetric path delays (e.g., inverter lag).
  - **Static-0 Hazard (0 → 1 → 0):** Output expected to stay low momentarily pulses high during intermediate switching.
- **Lookahead Synchronization (`pending_values`):** Maintains scheduled future gate states to prevent stale overwrite bugs in rapid multi-path transitions.

### Layer 5: Multi-Bit Bus Expander & 4-Bit ALU Synthesizer
- **Bus Syntax (`BUS INPUT/OUTPUT`):** Lexical regex engine automatically unpacks multi-bit vector declarations (e.g., `BUS INPUT A[3:0] B[3:0] OP[1:0]`) into distinct indexed scalar nodes.
- **4-Bit Arithmetic Logic Unit:** Combines 4-bit bitwise logic operations and ripple-carry addition with a 4:1 multiplexer slice architecture:
  - `OP = 00`: Bitwise AND (`A & B`)
  - `OP = 01`: Bitwise OR (`A | B`)
  - `OP = 10`: Bitwise XOR (`A ^ B`)
  - `OP = 11`: 4-Bit Addition (`A + B + CIN`) with Ripple-Carry Overflow (`COUT`)

---

## Circuits Implemented & Verified

### 1. Combinational Logic: 1-Bit Full Adder
Implements a 1-bit full adder using two cascaded XOR gates, two AND gates, and an OR gate to compute `SUM = A ⊕ B ⊕ Cin` and `COUT = (A · B) + (S1 · Cin)`.

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

1. **Clock:** Configured with a 10 ns period (5 ns half-period), producing rising edges at T = 5 ns, 15 ns, 25 ns, ...
2. **T = 3 ns:** Data input `D → 1`. Output `Q` remains 0 (holds previous state).
3. **T = 5 ns:** Rising clock edge arrives. `D = 1` is sampled.
4. **T = 6 ns (5 ns + 1 ns delay):** Output `Q` transitions to 1.
5. **T = 12 ns:** Data input `D → 0`. Output `Q` holds 1 in memory until the next clock edge at T = 15 ns.

#### D Flip-Flop Waveform Simulation:

<img width="1851" height="823" alt="Screenshot 2026-08-18 230549" src="https://github.com/user-attachments/assets/c498a5ba-302f-46da-bb73-68f9920bf0c7" />

---

### 3. Timing Hazards: Asymmetric Path Glitch Circuit

Evaluates the boolean function `F = (A · B) + (!A · C)` where `B = 1` and `C = 1`. Switching `A` from 1 → 0 introduces a 1 ns inverter skew path that induces a temporary Static-1 hazard dip.

#### Netlist (`hazard_circuit.txt`):

```text
INPUT A B C

NOT NOT_A A
AND G1 A B
AND G2 NOT_A C
OR F G1 G2

OUTPUT F

```

#### Automated Hazard Report Output:

```text
=== Hazard Analysis Report ===
Total glitches: 1

Node 'F' experienced a Static-1 (1->0->1) glitch of 1 ns at T: 15 ns
=============================

```

---

### 4. 4-Bit Arithmetic Logic Unit (ALU) Synthesizer

Full 4-bit data path processor executing vector bitwise and ripple-carry arithmetic operations selected via a 2-bit opcode decoder.

#### Netlist (`alu4.txt`):

```text
BUS INPUT A[3:0] B[3:0] OP[1:0]
INPUT CIN

# Opcode Decoder (AND=00, OR=01, XOR=10, ADD=11)
NOT NOT_OP1 OP[1]
NOT NOT_OP0 OP[0]
AND DEC0 NOT_OP1 NOT_OP0
AND DEC1 NOT_OP1 OP[0]
AND DEC2 OP[1] NOT_OP0
AND DEC3 OP[1] OP[0]

# 4 Bit Slices with Full Adder & 4:1 Multiplexer
# (See alu4.txt for complete structural gate mapping)

BUS OUTPUT ALU_OUT[3:0]
OUTPUT COUT

```

#### ALU Verification Testbench & Execution Trace:

* **Test 1 (T = 0 ns):** `OP = 00` (AND) → `A = 10`, `B = 12` ⇒ `ALU_OUT = 8 (0b1000)`
* **Test 2 (T = 30 ns):** `OP = 01` (OR) → `A = 10`, `B = 12` ⇒ `ALU_OUT = 14 (0b1110)`
* **Test 3 (T = 60 ns):** `OP = 10` (XOR) → `A = 10`, `B = 12` ⇒ `ALU_OUT = 6 (0b0110)`
* **Test 4 (T = 90 ns):** `OP = 11` (ADD) → `A = 7`, `B = 5` ⇒ `ALU_OUT = 12 (0b1100)`, `COUT = 0`
* **Test 5 (T = 120 ns):** `OP = 11` (ADD with Overflow) → `A = 12`, `B = 6` ⇒ `ALU_OUT = 2 (0b0010)`, `COUT = 1`

#### 4-Bit ALU Waveform Simulation:

<img width="1834" height="842" alt="Screenshot 2026-08-21 183918" src="https://github.com/user-attachments/assets/d4581de7-1f25-435f-8fe9-a76f23e2c958" />


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
* [x] **Layer 4:** Static Hazard & Dynamic Glitch Detection (Race condition analysis)
* [x] **Layer 5:** Multi-Bit Bus Syntax & 4-Bit ALU Synthesizer

---

```

```
