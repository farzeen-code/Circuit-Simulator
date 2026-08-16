#include<iostream>
#include "Node.h"
#include "MemoryPool.h"
#include "GraphEngine.h"
#include "Parser.h"
#include "EventSimulator.h"
using namespace std;


int main(){

    cout<<"==================================\n";
    cout<<"Event-Driven Logic Circuit Simulator (DES)\n";
    cout<<"==================================\n\n";

    fixedPoolAllocator<Node> pool;
    circuitGraph circuit;
    NetlistParser parser(pool, circuit);

    string filename = "circuit.txt";
    cout<<"Loading circuit from file: "<<filename<<endl;
    if(!parser.load_file(filename)){
        cout<<"Failed to load circuit file.\n\n";
        return 1;
    }

    const auto& nodes = parser.get_nodes();

    cout<<"\nCircuit Connectivity check:"<<endl;
    for(const auto& pair : nodes){
        cout<<"Node ["<<pair.first<<"] has: "<<endl
            <<pair.second->inputs.size()<<" inputs"<<endl
            <<pair.second->outputs.size()<<" outputs"<<endl;
    }
    EventSimulator sim;

    sim.schedule_event(0, nodes.at("A"), true);
    sim.schedule_event(15, nodes.at("B"), true);
    sim.schedule_event(30, nodes.at("Cin"), true);

    

    // Run the simulation across the time domain
    sim.run(60);
  
    for (const auto& pair : parser.get_nodes()){
        pool.deallocate(pair.second);
    }

    cout<<"\nMemory safely returned to the pool. Exiting program\n";
    return 0;

}


