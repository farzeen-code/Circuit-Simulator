#include<iostream>
#include "Node.h"
#include "MemoryPool.h"
#include "GraphEngine.h"
#include "Parser.h"
#include "EventSimulator.h"
#include "VcdWriter.h"

using namespace std;

void schedule_bus(EventSimulator& sim, const unordered_map<string, Node*> nodes, const string& bus_name, int bits, uint64_t val, uint64_t time){
    for(int i = 0; i < bits; ++i){
        string pin = bus_name + "[" + to_string(i) + "]";
        bool bit_val = (val >> i) & 1;
        sim.schedule_event(time, nodes.at(pin), bit_val);
    }
};

uint64_t read_bus(const unordered_map<string, Node*> nodes, const string& bus_name, int bits){
    uint64_t val = 0;
    for(int i = 0; i<bits; i++){
        string pin = bus_name + "[" + to_string(i) + "]";
        if(nodes.at(pin)->value){
            val |= (1ULL << i);
        }
    }
    return val;
};


int main(){

    cout<<"==================================\n";
    cout<<"Sequential Circuit and Clock Engine\n";
    cout<<"==================================\n\n";

    

    fixedPoolAllocator<Node> pool;
    circuitGraph circuit;
    NetlistParser parser(pool, circuit);

    string filename = "alu4.txt";
    cout<<"Loading circuit from file: "<<filename<<endl;
    if(!parser.load_file(filename)){
        cout<<"Failed to load circuit file.\n\n";
        return 1;
    }

    const auto& nodes = parser.get_nodes();

    VcdWriter vcd;
    string vcd_path = "waveform.vcd";

    if(!vcd.open(vcd_path, nodes)) return 1;


    cout<<"\nCircuit Connectivity check:"<<endl;
    for(const auto& pair : nodes){
        cout<<"Node ["<<pair.first<<"] has: "<<endl
            <<pair.second->inputs.size()<<" inputs"<<endl
            <<pair.second->outputs.size()<<" outputs"<<endl;
    }

    EventSimulator sim;
    sim.set_vcd_writer(&vcd);
    sim.schedule_event(0, nodes.at("CIN"), false);

   // Test 1: Bitwise AND (OP = 0b00). A = 0b1010 (10), B = 0b1100 (12) -> Expected OUT = 0b1000 (8)
    schedule_bus(sim, nodes, "A", 4, 0b1010, 0);
    schedule_bus(sim, nodes, "B", 4, 0b1100, 0);
    schedule_bus(sim, nodes, "OP", 2, 0b00, 0);

    // Test 2: Bitwise OR (OP = 0b01) at T = 30ns -> Expected OUT = 0b1110 (14)
    schedule_bus(sim, nodes, "OP", 2, 0b01, 30);

    // Test 3: Bitwise XOR (OP = 0b10) at T = 60ns -> Expected OUT = 0b0110 (6)
    schedule_bus(sim, nodes, "OP", 2, 0b10, 60);

    // Test 4: Addition (OP = 0b11) at T = 90ns. A = 0b0111 (7), B = 0b0101 (5) -> Expected OUT = 0b1100 (12), COUT = 0
    schedule_bus(sim, nodes, "A", 4, 0b0111, 90);
    schedule_bus(sim, nodes, "B", 4, 0b0101, 90);
    schedule_bus(sim, nodes, "OP", 2, 0b11, 90);

    // Test 5: Addition with Overflow at T = 120ns. A = 0b1100 (12), B = 0b0110 (6) -> Expected OUT = 0b0010 (2), COUT = 1
    schedule_bus(sim, nodes, "A", 4, 0b1100, 120);
    schedule_bus(sim, nodes, "B", 4, 0b0110, 120);
    schedule_bus(sim, nodes, "OP", 2, 0b11, 120);

    sim.run(200);

    cout<<"\n===== 4-bit ALU Final Verification =====\n";
    cout<<"ALU_OUT[3:0] Value: " << read_bus(nodes, "ALU_OUT", 4)<<" 0b"
        << bitset<4>(read_bus(nodes, "ALU_OUT", 4))<<" )\n";
    cout<<"COUT value: "<<nodes.at("COUT")->value<<"\n";
    cout<<"========================================\n";
  
    for (const auto& pair : parser.get_nodes()){
        pool.deallocate(pair.second);
    }

    cout<<"\nMemory safely returned to the pool. Exiting program\n";
    return 0;

}


