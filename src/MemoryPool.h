#include<vector>
#include<cstddef>
#include<utility>
using namespace std;

template <typename T, size_t BlockSize = 4096>

class fixedPoolAllocator{
    private:
        union nodeUnion{
            T element;
            nodeUnion* next;

            nodeUnion() {}
            ~nodeUnion() {}
        };
        nodeUnion* free_list{nullptr};
        vector<void*> allocatedBlocks; 

        void allocate_new_block(){
            size_t num_elements = BlockSize / sizeof(nodeUnion);
            nodeUnion* block = static_cast<nodeUnion*>(::operator new(BlockSize));
            allocatedBlocks.push_back(block);

            for(size_t i=0; i<num_elements-1; ++i){
                block[i].next = &block[i+1];

            }

            block[num_elements - 1].next = free_list;
            free_list = block;
        }


    public:
        fixedPoolAllocator() = default;
        
        template <typename...Args>
        T* allocate(Args&&...args) {
            if(!free_list) allocate_new_block();

            nodeUnion* curr = free_list;
            free_list = free_list->next;

            T* obj_ptr = reinterpret_cast<T*>(&curr->element);
            :: new (obj_ptr) T(forward<Args>(args)...);
            return obj_ptr;
        }

        void deallocate(T* ptr){
            if(!ptr) return;
            ptr->~T();

            nodeUnion* node = reinterpret_cast<nodeUnion*>(ptr);
            node->next = free_list;
            free_list = node;
        }

        ~fixedPoolAllocator(){
            for(void* ptr: allocatedBlocks){
                ::operator delete(ptr);
            }
        }
        

};

