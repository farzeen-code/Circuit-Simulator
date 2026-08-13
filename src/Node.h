#pragma once
#include<string>
#include<vector>
using namespace std;

enum class GateType{
    INPUT,
    OUTPUT,
    AND,
    OR,
    NOT,
    NAND,
    NOR,
    XOR 
};

struct Node{
    string name;
    GateType type;
    bool value{false};
    int propagation_delay{1};

    vector<Node*> inputs;
    vector<Node*> outputs;

    // tracks how many signals the node is waiting on
    int in_degree{0};

    Node(string n, GateType t) : name(move(n)), type(t) {}
};