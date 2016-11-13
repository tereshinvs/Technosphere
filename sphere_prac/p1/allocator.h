#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdexcept>
#include <string>

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const {
        return type;
    }
};

struct MemoryInfo {
    MemoryInfo *next;
    MemoryInfo **reference;
    size_t size;
};

struct GeneralMemoryInfo {
    size_t allocated;
    char *data_end;
    char *begin_ref;
};

class Pointer;

class Allocator {
private:
    char *base;
    size_t size;

    GeneralMemoryInfo *get_general_memory_info() const;
    MemoryInfo *get_next(MemoryInfo *from) const;
    MemoryInfo **get_available_ref_space() const;

public:
    Allocator(void *_base, size_t _size);

    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    std::string dump();

    friend class Pointer;
};

class Pointer {
private:
    MemoryInfo **p;

    void set(MemoryInfo **newp) {
        p = newp;
    }

    Pointer(MemoryInfo **_p):
        p(_p) {}

public:
    Pointer():
        p(nullptr) {}

    void *get() const {
        if (!p || !*p)
            return nullptr;
        return *p + 1;
    }

    friend class Allocator;
};

#endif
