#include <iostream>
#include <vector>
#include "freeBlocksAllocator.h"
#include "binaryAllocator.h"
#include <benchmark/benchmark.h>
#include <chrono>

/**
 * Вариант 19
 * Сравнить 2 алгоритма аллокации: списки свободных блоков и блоки по 2 в степени n.
 * Каждый аллокатор должен обладать следующим интерфейсом:
 * Allocator* createMemoryAllocator(void* realMemory, size_t memorySize) - создание аллокатора
 * void* alloc(Allocator* allocator, size_t block_size) - выделение памяти
 * void* free(Allocator* allocator, void* block) - возвращение выделенной памяти
 */


static void BM_FreeBlocksAllocator(benchmark::State &state) {
    for (auto _: state) {
        auto allocator = FreeBlocksAllocator(1024000);
        std::vector<void *> pointers;
        char mod;
        while (std::cin >> mod) {
            if (mod == '+') {
                int size;
                std::cin >> size;
                auto ptr = allocator.allocate(size);
                if (ptr != nullptr) {
                    pointers.push_back(ptr);
                }
            }
            if (mod == '-') {
                int index;
                std::cin >> index;
                allocator.deallocate(pointers[index]);
            }
        }
    }
}

BENCHMARK(BM_FreeBlocksAllocator);

static void BM_BinaryAllocator(benchmark::State &state) {
    for (auto _: state) {
        auto allocator = BinaryAllocator(1024000);
        std::vector<void *> pointers;
        char mod;
        while (std::cin >> mod) {
            if (mod == '+') {
                int size;
                std::cin >> size;
                auto ptr = allocator.allocate(size);
                if (ptr) {
                    pointers.push_back(ptr);
                }
            }
            if (mod == '-') {
                int index;
                std::cin >> index;
                allocator.deallocate(pointers[index]);
            }
        }
    }
}

BENCHMARK(BM_BinaryAllocator);

//BENCHMARK_MAIN();

int main(){

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    auto allocator = BinaryAllocator(1024000);
    std::vector<void *> pointers;
    char mod;
    while (std::cin >> mod) {
        if (mod == '+') {
            int size;
            std::cin >> size;
            auto ptr = allocator.allocate(size);
            if (ptr) {
                pointers.push_back(ptr);
            }
        }
        if (mod == '-') {
            int index;
            std::cin >> index;
            allocator.deallocate(pointers[index]);
        }
    }
    std::chrono::time_point<std::chrono::system_clock> finish = std::chrono::system_clock::now();

    std::chrono::duration<double> difference = finish - start;
    const double ms = difference.count() * 1000;
    std::cout << ms << " ms\n";

}