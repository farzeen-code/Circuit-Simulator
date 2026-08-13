#include<iostream>
#include<Node.h>
#include<MemoryPool.h>
#include<GraphEngine.h>
using namespace std;

int main(){

    cout<<"\n Initializing Circuit Simulator\n";

    fixedPoolAllocator<Node> pool;

    Node* inA = pool.allocate("In_A", GateType::INPUT);
    Node* inB = pool.allocate("In_B", GateType::INPUT);
    Node* and1 = pool.allocate("AND_1", GateType::AND);
    Node* not1 = pool.allocate("NOT_1", GateType::NOT);

    inA->value = true;
    inB->value = true;

    circuitGraph circuit;
    circuit.addNode(inA);
    circuit.addNode(inB);
    circuit.addNode(and1);
    circuit.addNode(not1);

    circuit.addWire(inA, and1);
    circuit.addWire(inB, and1);
    circuit.addWire(and1, not1);

    if(!circuit.compile()){
        cout<<"Error: Cycle detected in the circuit.";
        return 1;
    }

    cout<<"\nCircuit compiled successfully!\n";

    circuit.evaluate();
    circuit.display();

    pool.deallocate(inA);
    pool.deallocate(inB);
    pool.deallocate(and1);
    pool.deallocate(not1);

    return 0;
}