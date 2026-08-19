#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Node.h"
#include "MemoryPool.h"
#include "GraphEngine.h"
using namespace std;

/*

Circuit Definition:
INPUT A, B, C

Syntax: GATE_TYPE   OUTPUT_NAME   INPUT1   INPUT2   ...
        AND         G1            A         B

*/

class NetlistParser
{
private:
    fixedPoolAllocator<Node> &pool;
    circuitGraph &circuit;

    unordered_map<string, Node *> node_map;

    GateType string_to_gate(string &str)
    {
        string upper = str;
        transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

        if (upper == "INPUT")
            return GateType::INPUT;
        else if (upper == "OUTPUT")
            return GateType::OUTPUT;
        else if (upper == "AND")
            return GateType::AND;
        else if (upper == "OR")
            return GateType::OR;
        else if (upper == "NOT")
            return GateType::NOT;
        else if (upper == "NAND")
            return GateType::NAND;
        else if (upper == "NOR")
            return GateType::NOR;
        else if (upper == "XOR")
            return GateType::XOR;
        else if (upper == "DFF")
            return GateType::DFF;
        else if (upper == "CLOCK")
            return GateType::CLOCK;
        else
            throw runtime_error("Unknown gate type: " + str);
    }

    uint64_t get_gate_delay(GateType type)
    {
        switch (type)
        {
        case GateType::XOR:
            return 3;
        case GateType::AND:
            return 2;
        case GateType::OR:
            return 2;
        case GateType::NAND:
            return 2;
        case GateType::NOR:
            return 2;
        case GateType::NOT:
            return 1;
        case GateType::DFF:
            return 1;
        default:
            return 0;
        }
    }

    Node *get_or_create_node(string &name, GateType type, uint64_t delay = 1)
    {
        auto it = node_map.find(name);
        if (it != node_map.end())
        {
            it->second->type = type;
            it->second->propagation_delay = delay;
            return it->second;
        }

        Node *node = pool.allocate(name, type, delay);
        node->propagation_delay = delay;
        circuit.addNode(node);
        node_map[name] = node;
        return node;
    }

public:
    NetlistParser(fixedPoolAllocator<Node> &p, circuitGraph &g) : pool(p), circuit(g) {}

    bool load_file(const string &filepath)
    {
        ifstream file(filepath);
        if (!file.is_open())
        {
            cerr << "Error: Could not open file " << filepath << endl;
            return false;
        }
        string line;
        int line_no = 0;
        while (getline(file, line))
        {
            line_no++;

            stringstream ss(line);
            string first_token;

            if (!(ss >> first_token) || first_token[0] == '#')
                continue;

            string command = first_token;
            transform(command.begin(), command.end(), command.begin(), ::toupper);

            if (command == "INPUT" || command == "CLOCK")
            {
                GateType in_type = (command == "CLOCK") ? GateType::CLOCK : GateType::INPUT;
                string input_name;
                while (ss >> input_name)
                {
                    get_or_create_node(input_name, in_type, 0);
                }
            }
            else if (command == "OUTPUT")
            {
                string out_name;
                while (ss >> out_name)
                {
                    if (node_map.find(out_name) == node_map.end())
                    {
                        cerr << "Error: Output pin " << out_name << " declared before definition." << endl;
                    }
                }
            }
            else
            {
                GateType type;
                try
                {
                    type = string_to_gate(command);
                }
                catch (const exception &e)
                {
                    cerr << "Syntax error on line " << line_no << ": " << e.what() << endl;
                    return false;
                }
                string out_pin;
                if (!(ss >> out_pin))
                {
                    cerr << "Syntax error on line " << line_no << ": Missing output pin." << endl;
                    return false;
                }

                uint64_t gate_delay = get_gate_delay(type);
                Node *node = get_or_create_node(out_pin, type, gate_delay);

                string in_pin;
                while (ss >> in_pin)
                {
                    auto it = node_map.find(in_pin);
                    if (it == node_map.end())
                    {
                        cerr << "Syntax error on line " << line_no << ": Input signal " << in_pin << " not defined before use." << endl;
                        return false;
                    }
                    circuit.addWire(it->second, node);
                }
            }
        }
        file.close();
        return true;
    }

    void set_inputs(const string &name, bool val)
    {
        auto it = node_map.find(name);
        if (it != node_map.end() && it->second->type == GateType::INPUT)
        {
            it->second->value = val;
        }
        else
        {
            cerr << "Error: Unknown input pin " << name << endl
                 << endl;
        }
    }

    const unordered_map<string, Node *> get_nodes() const
    {
        return node_map;
    }
};
