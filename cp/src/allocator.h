#ifndef CP_ALLOCATOR_H
#define CP_ALLOCATOR_H

#include <iostream>

class Allocator {
public:
    typedef void value_type;
    typedef value_type *pointer;
    typedef size_t size_type;

    Allocator() = default;

    ~Allocator() {
        ::free(startPointer);
    }

    /**
     * Выделяет блок памяти заданного размера.
     * @param size - размер блока памяти.
     * @return указатель на выделенный объект памяти или значение null, если память не была выделена
     */
    virtual pointer allocate(size_type size) = 0;

    /**
     * Освобождает переданный указатель.
     * @param ptr - указатель для освобождения памяти.
     */
    virtual void deallocate(pointer ptr) = 0;

    /**
     Освобождает всю память, выделенную для аллокатора
     */
    void free() {
        auto *header = static_cast<Header *>(startPointer);
        header->isAvailable = true;
        header->size = (totalSize - headerSize);
        usedSize = headerSize;
    };

    /**
     * Выводит информацию о состоянии памяти.
     */
    void memoryDump() {
        std::cout << "Total size: " << totalSize << std::endl;
        std::cout << "Used: " << usedSize << std::endl;
        std::cout << "Header size: " << headerSize << std::endl;
        auto *header = static_cast<Header *>(startPointer);
        while (header != endPointer) {
            auto isAvailable = header->isAvailable ? "+" : "-";
            std::cout << isAvailable << " " << header << " " << header->size << std::endl;
            header = header->next();
        }
        std::cout << std::endl;
    }

protected:

    struct Header {
    public:
        size_type size;
        size_type previousSize;
        bool isAvailable;

        /**
         * @return место в памяти заголовка следующего блока
         */
        inline Header *next() {
            return (Header *) ((char *) (this + 1) + size);
        }

        /**
         * @return место в памяти заголовка предыдущего блока
         */
        inline Header *previous() {
            return (Header *) ((char *) this - previousSize) - 1;
        }

    };

    const size_type headerSize = sizeof(Header);
    const size_type blockAlignment = 4;
    pointer startPointer = nullptr;
    pointer endPointer = nullptr;
    size_type totalSize = 0;
    size_type usedSize = 0;

    /**
     * Находит ближайший свободный блок памяти.
     * @param size - размер запрашиваемого блока.
     * @return указатель на заголовок найденного блока.
     */
    Header *find(size_type size) {
        auto *header = static_cast<Header *>(startPointer);
        while (!header->isAvailable || header->size < size) {
            header = header->next();
            if (header >= endPointer) { return nullptr; }
        }
        return header;
    }

    /**
     * Разделяет блок на 2 части.
     * @param header - заголовок блока для разделения
     * @param chunk - размер блока, который нужно оставить
     */
    void splitBlock(Header *header, size_type chunk) {
        size_type blockSize = header->size;
        header->size = chunk;
        header->isAvailable = false;
        if (blockSize - chunk >= headerSize) {
            auto *next = header->next();
            next->previousSize = chunk;
            next->size = blockSize - chunk - headerSize;
            next->isAvailable = true;
            usedSize += chunk + headerSize;
            auto *followed = next->next();
            if (followed < endPointer) {
                followed->previousSize = next->size;
            }
        } else {
            header->size = blockSize;
            usedSize += blockSize;
        }
    }

    /**
     * Проверяет валидность переданного адреса.
     */
    bool validateAddress(void *ptr) {
        auto *header = static_cast<Header *>(startPointer);
        while (header < endPointer) {
            if (header + 1 == ptr){ return true; }
            header = header->next();
        }
        return false;
    }

};

#endif //CP_ALLOCATOR_H
