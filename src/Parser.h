#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <regex>
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

    Node *get_or_create_node(const string &name, GateType type, uint64_t delay = 1)
    {
        auto it = node_map.find(name);
        if (it != node_map.end())
        {
            if(type == GateType::OUTPUT) return it->second;
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

    vector<string> expand_bus_token(const string& token){
        regex bus_regex(R"(^([a-zA-Z0-9_]+)\[(\d+):(\d+)\]$)");
        smatch match;

        if(regex_match(token, match, bus_regex)){
            string prefix = match[1].str();
            int msb = stoi(match[2].str());
            int lsb = stoi(match[3].str());

            vector<string> expanded;
            if(msb >= lsb){
                for(int i = msb; i>=lsb; i--){
                    expanded.push_back(prefix + "[" + to_string(i) + "]");
                }
                
            }
            else{
                for(int i=msb; i<=lsb; i++){
                    expanded.push_back(prefix + "[" + to_string(i) + "]");
                }
            }
            return expanded;
        }
        return {token};
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

            if(command == "BUS"){
                string bus_dir;
                if(!(ss >> bus_dir)) continue;
                transform(bus_dir.begin(), bus_dir.end(), bus_dir.begin(), ::toupper);
                GateType bus_type = (bus_dir == "INPUT") ? GateType::INPUT : GateType::OUTPUT;

                string bus_token;
                while(ss >> bus_token){
                    for(const auto& pin_name : expand_bus_token(bus_token)){
                        if(bus_type == GateType::OUTPUT){
                            if(node_map.find(pin_name) == node_map.end()){
                                get_or_create_node(pin_name, GateType::OUTPUT, 0);
                            }
                        }
                        else{
                            get_or_create_node(pin_name, GateType::INPUT, 0);
                        }
                    }
                }
            }

            else if (command == "INPUT" || command == "CLOCK")
            {
                GateType in_type = (command == "CLOCK") ? GateType::CLOCK : GateType::INPUT;
                string input_token;
                while (ss >> input_token)
                {
                    for(const auto& pin_name : expand_bus_token(input_token)){
                        get_or_create_node(pin_name, in_type, 0);
                    }
                }
            }
            else if (command == "OUTPUT")
            {
                string out_token;
                while (ss >> out_token)
                {
                    for(const auto& pin_name : expand_bus_token(out_token)){
                        if (node_map.find(pin_name) == node_map.end())
                        {
                            get_or_create_node(pin_name, GateType::OUTPUT, 0);
                        }
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

    void set_bus_value(const string& bus_name, int msb, int lsb, uint64_t val)
    {
        int step = (msb >= lsb) ? 1 : -1;
        int idx = 0;
        for(int i = lsb; lsb+=step; ++idx){
            string pin_name = bus_name + "[" + to_string(i) + "]";
            auto it = node_map.find(pin_name);
            if(it != node_map.end()){
                it->second->value = (val >> idx) & 1;
            }

            if(i == msb) break;
        }
    }

    const unordered_map<string, Node *> get_nodes() const
    {
        return node_map;
    }
};
