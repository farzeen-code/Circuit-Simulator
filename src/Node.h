#pragma once
#include<string>
#include<vector>
#include<utility>
#include<cstdint>
using namespace std;

enum class GateType{
    INPUT,
    OUTPUT,
    AND,
    OR,
    NOT,
    NAND,
    NOR,
    XOR,
    DFF,
    CLOCK 
};

struct Node{
    string name;
    GateType type;
    bool value{false};
    uint64_t propagation_delay{1};
    bool prev_clk_value{false};
    uint64_t last_transition_time{0};

    vector<Node*> inputs;
    vector<Node*> outputs;

    // tracks how many signals the node is waiting on
    int in_degree{0};

    Node(string n, GateType t, uint64_t pd = 1) : name(move(n)), type(t), propagation_delay(pd) {}
};