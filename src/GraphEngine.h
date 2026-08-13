#pragma once
#include<Node.h>
#include<iostream>
#include<queue>
#include<unordered_map>
#include<vector>
using namespace std;

class circuitGraph{
    private:
        vector<Node*> nodes;
        vector<Node*> evaluation_order;

    public:
        void addNode(Node* node){
            nodes.push_back(node);
        }

        void addWire(Node* src, Node* dst){
            src->outputs.push_back(dst);
            dst->inputs.push_back(src);
            dst->in_degree++;
        }

        bool compile(){
            evaluation_order.clear();
            queue<Node*> ready_queue;
            
            // temporaru hash map for keeping track of inputs 
            unordered_map<Node*, int> in_degrees;

            for(auto* node : nodes){
                in_degrees[node] = node->in_degree;
                if(node->in_degree == 0){
                    ready_queue.push(node);
                }
            }

            while(!ready_queue.empty()){
                Node* curr = ready_queue.front();
                ready_queue.pop();
                evaluation_order.push_back(curr);
                
                // Now the that curr has been popped, 'next' gate is receiving one less input as it received an output wire from curr. --in_degrees to check and push to ready_queue.
                for(Node* next : curr->outputs){
                    if(--in_degrees[next] == 0){
                        ready_queue.push(next);
                    }
                }
            }

            return evaluation_order.size() == nodes.size();
        }

        void evaluate(){
            for(Node* node : evaluation_order){
                if(node->type == GateType::INPUT) continue;
                
                bool result = false;
                switch(node->type){
                    case GateType::AND: {
                        result = true;
                        for(auto* i : node->inputs) result &= i->value;
                        break; 
                    }

                    case GateType::NAND: {
                        result = true;
                        for(auto* i : node->inputs) result &= i->value;
                        result = !result;
                        break; 
                    }

                    case GateType::OR: {
                        result = false;
                        for(auto* i : node->inputs) result |= i->value;
                        break; 
                    }

                    case GateType::NOR: {
                        result = false;
                        for(auto* i : node->inputs) result |= i->value;
                        result=!result;
                        break; 
                    }

                    case GateType::NOT: {
                        if(!node->inputs.empty()) result = !node->inputs[0]->value;
                        break; 
                    }

                    case GateType::XOR: {
                        result = true;
                        for(auto* i : node->inputs) result ^= i->value;
                        break; 
                    }
                    default: break;
                
                }
                node->value = result;

            }
        }

        void display() const {
            cout<<"\n\n Circuit Evaluation Results:\n\n";
            for(const auto* node : evaluation_order){
                cout<<"Node: "<<node->name<<" = "<<(node->value ? "1" : "0")<<endl<<endl;
            }
        }
};