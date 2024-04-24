#ifndef safeGenerator_hpp
#define safeGenerator_hpp

#include <cstdint>
#include <mutex>

class SafeGenerator {
public:
    SafeGenerator() {};
    ~SafeGenerator() {};

    uint32_t generate() {
        lock.lock();
        uint32_t num = count++;
        lock.unlock();

        return num;
    }

private:
    uint64_t count = 0;
    std::mutex lock;
};

#endif
