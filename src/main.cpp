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

    string filename = "dff_circuit.txt";
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

    sim.register_clock(nodes.at("CLK"), 5);

    sim.schedule_event(3, nodes.at("D"), true);
    sim.schedule_event(12, nodes.at("D"), false);

    sim.run(40);
    
    cout<<"Waveform dump written to '"<<vcd_path<<"' \n";
  
    for (const auto& pair : parser.get_nodes()){
        pool.deallocate(pair.second);
    }

    cout<<"\nMemory safely returned to the pool. Exiting program\n";
    return 0;

}


