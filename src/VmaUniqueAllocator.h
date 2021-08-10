//
// Created by 16182 on 7/29/2021.
//

#ifndef HEART_SIM_VMAUNIQUEALLOCATOR_H
#define HEART_SIM_VMAUNIQUEALLOCATOR_H

#include<vk_mem_alloc.h>
#include<memory>

class UniqueAllocator {
public:
    UniqueAllocator():
    _allocator(nullptr)
    {}
    UniqueAllocator(VmaAllocator allocator):
    _allocator(allocator)
    {}
    UniqueAllocator(UniqueAllocator &) = delete;
    UniqueAllocator(UniqueAllocator && src){
        _allocator = src._allocator;
        src._allocator = nullptr;
    }
    UniqueAllocator & operator=(UniqueAllocator && src){
        _allocator = src._allocator;
        src._allocator = nullptr;
        return *this;
    }

    ~UniqueAllocator(){
        if(_allocator){
            vmaDestroyAllocator(_allocator);
            _allocator = nullptr;
        }
    }

    VmaAllocator & operator*(){
        return _allocator;
    }
    VmaAllocator const& operator*() const{
        return _allocator;
    }
private:
    VmaAllocator _allocator;
};

#endif //HEART_SIM_VMAUNIQUEALLOCATOR_H
