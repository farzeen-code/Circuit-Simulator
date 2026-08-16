#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "Node.h"
using namespace std;

struct Event{
    uint64_t timestamp;
    Node* target;
    bool new_value;

    bool operator>(const Event& other) const{
        return timestamp > other.timestamp;
    }
};

class EventSimulator{
    private:
        uint64_t curr_t{0};
        // priority queue<type, container, comparator>
        priority_queue<Event, vector<Event>, greater<Event>> event_queue;

        bool compute_gate_output(Node* node) const{
            switch(node->type){
                case GateType::AND: {
                    bool res = true;
                    for(const auto* i : node->inputs) res &= i->value;
                    return res;
                }

                case GateType::NAND: {
                    bool res = true;
                    for(const auto* i : node->inputs) res &= i->value;
                    return !res;
                }

                case GateType::OR: {
                    bool res = false;
                    for(const auto* i : node->inputs) res |= i->value;
                    return res;
                }

                case GateType::NOR: {
                    bool res = false;
                    for(const auto* i : node->inputs) res |= i->value;
                    return !res;
                }

                case GateType::XOR: {
                    bool res = false;
                    for(const auto* i : node->inputs) res ^= i->value;
                    return res;
                }

                case GateType::NOT: {
                    return node->inputs.empty() ? node->value : !node->inputs[0]->value;
                }

                case GateType::OUTPUT: {
                    return node->inputs.empty() ? node->value : node->inputs[0]->value; 
                }

                default:
                    return node->value;
            }
        }

    public:
        EventSimulator() = default;

        uint64_t get_time() const{
            return curr_t;
        }

        void schedule_event(uint64_t t, Node* n, bool val){
            event_queue.push({t, n, val});
    
        }

        void run(uint64_t max_time = UINT64_MAX){
            cout<<"\n====== Starting Event-Driven Simulation======\n";

            while(!event_queue.empty()){
                Event ev = event_queue.top();
                if(ev.timestamp > max_time) break;

                event_queue.pop();
                curr_t = ev.timestamp;
                if(ev.target->value != ev.new_value || curr_t == 0){
                    ev.target->value = ev.new_value;
                    cout<<"T= "<<curr_t<<" ns"<<endl<<"Signal Transition: "<<
                    ev.target->name<<" -> "<<ev.target->value<<endl;

                    for(Node* downstream : ev.target->outputs){
                        if(downstream->type == GateType::INPUT) continue;

                        bool next_val = compute_gate_output(downstream);

                        if(downstream->value != next_val || curr_t == 0){
                            uint64_t future_time = curr_t + downstream->propagation_delay;
                            event_queue.push({future_time, downstream, next_val});

                        }

                    }
                }


            }
            cout<<"\nSimulation finished at T: "<<curr_t<<" ns "<<endl;
        }

};


