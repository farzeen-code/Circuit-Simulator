#include<iostream>
#include "Node.h"
#include "MemoryPool.h"
#include "GraphEngine.h"
#include "Parser.h"
#include "EventSimulator.h"
#include "VcdWriter.h"

using namespace std;


int main(){

    cout<<"==================================\n";
    cout<<"Sequential Circuit and Clock Engine\n";
    cout<<"==================================\n\n";

    fixedPoolAllocator<Node> pool;
    circuitGraph circuit;
    NetlistParser parser(pool, circuit);

    string filename = "hazard_circuit.txt";
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
    sim.set_glitch_threshold(2); // Flag any pulse <= 2ns

    // Initial setup: A=1, B=1, C=1 (F evaluates to 1)
    sim.schedule_event(0, nodes.at("A"), true);
    sim.schedule_event(0, nodes.at("B"), true);
    sim.schedule_event(0, nodes.at("C"), true);

    // At T=10ns, flip A from 1 -> 0 while B=1, C=1
    // The inverter delay causes G1 to drop before G2 rises, creating a 1ns dip on F
    sim.schedule_event(10, nodes.at("A"), false);

    sim.run(25);
  
    for (const auto& pair : parser.get_nodes()){
        pool.deallocate(pair.second);
    }

    cout<<"\nMemory safely returned to the pool. Exiting program\n";
    return 0;

}


