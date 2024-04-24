#include "safeGenerator.hpp"
#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <queue>
#include <thread>
#include <vector>

static const size_t WORKER_COUNT = 4;
static const size_t WORKER_ITERS = 500;

static void queuePusher(std::queue<uint32_t>& queue, SafeGenerator& gen, uint32_t iters) {
    uint32_t num;
    for(std::uint32_t i=0; i<iters; i++) {
        num = gen.generate();
        queue.push(num);
    }
}

TEST(TestQueues, TestQueues) {
    std::queue<uint32_t> queue;
    SafeGenerator gen;

    std::thread workers[WORKER_COUNT];
    for(std::size_t n=0; n<WORKER_COUNT; n++) {
        workers[n] = std::thread(
            &queuePusher,
            std::ref(queue),
            std::ref(gen),
            WORKER_ITERS
        );
    }

    // Wait on all workers to finish
    for(std::size_t n=0; n<WORKER_COUNT; n++) workers[n].join();

    // Smoke test
    EXPECT_EQ(queue.size(), WORKER_ITERS * WORKER_COUNT);

    // Unpack into a vector
    std::vector<uint32_t> vector;
    for(std::size_t n=0; n<queue.size(); n++) {
        vector.push_back(queue.front());
        queue.pop();
    }

    
    std::sort(vector.begin(), vector.end());
    auto end = vector.end();
    EXPECT_TRUE(std::unique(vector.begin(), vector.end()) == end);
}
