#include<iostream>
#include "Node.h"
#include "MemoryPool.h"
#include "GraphEngine.h"
#include "Parser.h"
using namespace std;

void run_test(NetlistParser& parser, circuitGraph& circuit, bool a, bool b, bool cin){

    parser.set_inputs("A", a);
    parser.set_inputs("B", b);
    parser.set_inputs("Cin", cin);

    circuit.evaluate();

    const auto& nodes = parser.get_nodes();
    bool sum = nodes.at("SUM")->value;
    bool cout_val = nodes.at("COUT")->value;

    cout << "  " << a << " | " << b << " |  " << cin 
         << "  ||  " << sum << "  |   " << cout_val << "\n";
};

int main(){

    cout<<"==================================\n";
    cout<<"Digital Logic Circuit Simulator\n";
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

    if(!circuit.compile()){
        cout<<"Error: Cycle detected in circuit."<<endl;
        return 1;
    }

    cout<<"\nCircuit compiled successfully!"<<endl;

    cout << "--- 1-Bit Full Adder Truth Table ---\n";
    cout << "  A | B | Cin || SUM | COUT\n";
    cout << "----+---+-----++-----+-----\n";

    run_test(parser, circuit, 0, 0, 0);
    run_test(parser, circuit, 0, 0, 1);
    run_test(parser, circuit, 0, 1, 0);
    run_test(parser, circuit, 0, 1, 1);
    run_test(parser, circuit, 1, 0, 0);
    run_test(parser, circuit, 1, 0, 1);
    run_test(parser, circuit, 1, 1, 0);
    run_test(parser, circuit, 1, 1, 1);
  
    for (const auto& pair : parser.get_nodes()){
        pool.deallocate(pair.second);
    }

    cout<<"\nMemory safely returned to the pool. Exiting program\n";
    return 0;

}
