#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "Node.h"
#include "VcdWriter.h"
using namespace std;

struct Event{
    uint64_t timestamp;
    Node* target;
    bool new_value;

    bool operator>(const Event& other) const{
        return timestamp > other.timestamp;
    }
};

struct ClockGenerator{
    Node* clk_node;
    uint64_t half_period;
};

struct GlitchReport{
    string node_name;
    uint64_t timestamp;
    uint64_t duration;
    bool glitch_type;
};

class EventSimulator{
    private:
        uint64_t curr_t{0};
        // priority queue<type, container, comparator>
        priority_queue<Event, vector<Event>, greater<Event>> event_queue;
        VcdWriter* vcd_writer{nullptr};
        vector<ClockGenerator> clocks;
        vector<GlitchReport> detected_glitches;
        uint64_t glitch_threshold{2};  // Any pulse <=2 will be considered a glitch
        unordered_map<Node*, bool> pending_values;

        bool get_pending_value(Node* n) const {
            auto it = pending_values.find(n);
            if (it != pending_values.end()) {
                return it->second;
            }
            return n->value;
        }

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

                case GateType::DFF: {
                    if(node->inputs.size() < 2) return node->value;

                    bool d_val = node->inputs[0]->value;
                    bool clk_val = node->inputs[1]->value;

                    bool rising_edge = (!node->prev_clk_value && clk_val);
                    node->prev_clk_value = clk_val;

                    if(rising_edge){
                        return d_val;
                    }
                    return node->value;
                }

                default:
                    return node->value;
            }
        }

    public:
        EventSimulator() = default;

        uint64_t get_time() const{return curr_t;}

        const vector<GlitchReport>& get_glitches() const {return detected_glitches; }

        void register_clock(Node* clk_node, uint64_t half_period){
            clocks.push_back({clk_node, half_period});
        }

        void set_vcd_writer(VcdWriter* writer) {vcd_writer = writer;}

        void set_glitch_threshold(uint64_t threshold) {glitch_threshold = threshold;}

        void schedule_event(uint64_t t, Node* n, bool val){
            event_queue.push({t, n, val});
            pending_values[n] = val;
        }
        void run(uint64_t max_time = UINT64_MAX){
            cout<<"\n====== Starting Event-Driven Simulation======\n";

            for(const auto& clk : clocks){
                bool state = false;
                for(uint64_t t=0; t<max_time; t+=clk.half_period){
                    schedule_event(t, clk.clk_node, state);
                    state = !state;
                }
            }

            while(!event_queue.empty()){
                Event ev = event_queue.top();
                if(ev.timestamp > max_time) break;

                event_queue.pop();
                curr_t = ev.timestamp;
                if(ev.target->value != ev.new_value){
                    uint64_t duration = curr_t - ev.target->last_transition_time;

                    if(ev.target->last_transition_time > 0 && duration <= glitch_threshold){
                        detected_glitches.push_back({ev.target->name, curr_t, duration, ev.new_value});
                        cout<<"[!] Hazard Glitch detected on '"<<ev.target->name
                            <<"' at T: "<<curr_t<<" ns (Pulse width: "<<duration<<" ns)\n";
                    }

                    ev.target->value = ev.new_value;
                    ev.target->last_transition_time = curr_t;
                    
                    if(vcd_writer){
                        vcd_writer->dump_change(curr_t, ev.target);
                    }

                    for(Node* downstream : ev.target->outputs){
                        if(downstream->type == GateType::INPUT || downstream->type == GateType::CLOCK)  continue;

                        bool next_val = compute_gate_output(downstream);

                        if(get_pending_value(downstream) != next_val){
                            uint64_t future_time = curr_t + downstream->propagation_delay;
                            schedule_event(future_time, downstream, next_val);

                        }

                    }
                }


            }
            cout<<"\n=== Hazard Analysis report===\n";

            if(detected_glitches.empty()){
                cout<<"Status: Clean, no race conditions detected.\n";
            }
            else{
                cout<<"Total glitches: "<<detected_glitches.size()<<endl<<endl;
                for(const auto& g : detected_glitches){
                    cout<<"Node '"<<g.node_name<<"' experienced a "
                        <<(g.glitch_type ? "Static-1 (1->0->1)" : "Static-0 (0->1->0)")
                        <<" glitch of "<<g.duration
                        <<" ns at T: "<<g.timestamp<<" ns\n";
                }

            }
            cout<<"=============================\n";
        }

};


