#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Node.h"
using namespace std;

class VcdWriter{
    private:
        ofstream vcd_file;
        unordered_map<string, char> signal_identifiers;
        uint64_t last_dumped_time{UINT64_MAX};

    public:
        VcdWriter() = default;
        ~VcdWriter(){
            if(vcd_file.is_open()){
                vcd_file.close();
            }
        }

        bool open(string& filepath, const unordered_map<string, Node*> nodes){
            
            if(vcd_file.is_open()){
                vcd_file.close();
            }
            vcd_file.clear();

            vcd_file.open(filepath, ios::out | ios::trunc);
            if(!vcd_file.is_open()){
                cerr<<"Error: Could not open VCD file "<<filepath<<endl<<endl;
                return false;

            }

            vcd_file << "$date\n  August 2026\n$end\n";
            vcd_file << "$version\n  Custom C++ Discrete Event Simulator\n$end\n";
            vcd_file << "$timescale 1ns $end\n";
            vcd_file << "$scope module top $end\n";

            char curr_id = '!';
            for(const auto& pair : nodes){
                signal_identifiers[pair.first] = curr_id;
                vcd_file<<"$var wire 1 "<<curr_id<<" "<<pair.first<<" $end\n";
                curr_id++;
            }
            

            vcd_file << "$upscope $end\n";
            vcd_file << "$enddefinitions $end\n";
            vcd_file << "$dumpvars\n";

            for(const auto& pair : nodes ){
                vcd_file << (pair.second->value ? '1' : '0') << signal_identifiers[pair.first]<< " \n";
            }
            vcd_file << "$end\n";
            return true;
        }

        void dump_change(uint64_t timestamp, Node* node){
            if(!vcd_file.is_open() || !node) return;

            if(timestamp != last_dumped_time){
                vcd_file << "#" << timestamp <<"\n";
                last_dumped_time = timestamp;
            }

            char id = signal_identifiers[node->name];
            vcd_file << (node->value ? '1' : '0') << id <<endl<<endl;

        }
        

};