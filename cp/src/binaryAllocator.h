#ifndef CP_BINARYALLOCATOR_H
#define CP_BINARYALLOCATOR_H

#include <cmath>
#include "allocator.h"

class BinaryAllocator : public Allocator {
public:

    explicit BinaryAllocator(size_type size) {
        size = align(size);
        if ((startPointer = malloc(size)) == nullptr) {
            std::cerr << "Failed to allocate memory\n";
            return;
        }
        totalSize = size;
        endPointer = static_cast<void *>(static_cast<char *>(startPointer) + totalSize);
        auto *header = (Header *) startPointer;
        header->isAvailable = true;
        header->size = (totalSize - headerSize);
        header->previousSize = 0;
        usedSize = headerSize;
    };

    /**
     * Выравнивает размер запрашиваемой памяти до ближайшей степени 2.
     */
    static size_type align(size_type size) {
        int i = 0;
        while (pow(2, i) < size){
            i++;
        }
        return (size_type) pow(2, i);
    }

    /**
     * Выделяет блок памяти заданного размера.
     * @param size - размер блока памяти.
     * @return указатель на выделенный объект памяти или значение null, если память не была выделена
     */
    pointer allocate(size_type size) override {
        if (size <= 0) {
            std::cerr << "Size must be bigger than 0\n";
            return nullptr;
        }
        size = align(size);
        if (size > totalSize - usedSize) { return nullptr; }
        auto *header = find(size);
        if (header == nullptr) { return nullptr; }
        splitBlock(header, size);
        return header + 1;
    };

    /**
     * Освобождает переданный указатель.
     * @param ptr - указатель для освобождения памяти.
     */
    void deallocate(pointer ptr) override {
        if (!validateAddress(ptr)) {
            return;
        }
        auto *header = static_cast<Header *>(ptr) - 1;
        header->isAvailable = true;
        usedSize -= header->size;
        //defragmentation(header);
    };

private:

    /**
     * Функции проверки доступности близлежащих блоков.
     */
    bool isPreviousFree(Header *header) {
        auto *previous = header->previous();
        return header != startPointer && previous->isAvailable;
    }

    bool isNextFree(Header *header) {
        auto *next = header->next();
        return header != endPointer && next->isAvailable;
    }

    /**
     * Функция слияния свободных блоков.
     */
    void defragmentation(Header *header) {
        if (isPreviousFree(header)) {
            auto *previous = header->previous();
            if (header->next() < endPointer) {
                header->next()->previousSize += previous->size + headerSize;
            }
            previous->size += header->size + headerSize;
            usedSize -= headerSize;
            header = previous;
        }
        if (isNextFree(header)) {
            header->size += headerSize + header->next()->size;
            usedSize -= headerSize;
            auto *next = header->next();
            if (next != endPointer) { next->previousSize = header->size; }
        }
    }

};

#endif //CP_BINARYALLOCATOR_H
