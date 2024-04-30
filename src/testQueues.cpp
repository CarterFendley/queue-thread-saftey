#include "safeGenerator.hpp"
#include "k_utils/strings.hpp"
#include "gtest/gtest-param-test.h"
#include <_types/_uint32_t.h>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <gtest/gtest.h>
#include <mutex>
#include <queue>
#include <thread>
#include <future>
#include <vector>

/********************************
 *
 * Testing queue popping + writing
 *
 ********************************/
static const size_t PUSH_ITERS = 1000;

// Sync
std::condition_variable cv;
std::mutex m;
bool worker_started = false;
bool halt_worker = false;

// Fixture to facilitate parameterization
// See: https://google.github.io/googletest/advanced.html#value-parameterized-tests
typedef struct {
    std::chrono::milliseconds postPushSleep;
    std::chrono::milliseconds postFrontSleep;
    std::chrono::milliseconds postPopSleep;
} timings;

class PushAndPopFixture : public testing::TestWithParam<timings> {}; 


static std::vector<std::string> queuePopper(
        std::queue<uint32_t>& queue,
        std::chrono::milliseconds postFrontSleep,
        std::chrono::milliseconds postPopSleep
){
    // Signal that this thread is live
    m.lock();
    worker_started = true;
    m.unlock();
    cv.notify_one();

    std::vector<std::string> errors;
    uint32_t expected = 0;

    while (!halt_worker || queue.size() != 0) {
        // For when we don't have a halt signal and just need to spin
        while (queue.size() < 1)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            
            // If halt worker signal while we are waiting, break
            if(halt_worker)
                break;
        }
        
        uint32_t actual = queue.front();
        std::this_thread::sleep_for(postFrontSleep);
        queue.pop();
        std::this_thread::sleep_for(postPopSleep);
        if (actual != expected) {
            errors.push_back(
                string_format("Error: Unexpected queue value (expected=%u, actual=%u)!", expected, actual)
            );

            // Assume actual is "new normal" and set expectation for next iteration based on that
            expected = actual + 1;
        } else {
            // Actual == expected (awesome!)
            expected = expected + 1;
        }
    }

    return errors;
}


TEST_P(PushAndPopFixture, TestPushAndPop) {
    std::queue<uint32_t> queue;
    
    auto popper = std::async(
        std::launch::async, // Looks like default is to encourage deferral: https://en.cppreference.com/w/cpp/thread/async
        &queuePopper,
        std::ref(queue),
        GetParam().postFrontSleep,
        GetParam().postPopSleep
    );

    // Not 100% sure on implementations of async so want to make sure the thread is live at the same time as the pushing ththread 
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return worker_started; }); 

    for(std::size_t n=0; n<PUSH_ITERS; n++) {
        queue.push(n);
        std::this_thread::sleep_for(GetParam().postPushSleep);
    }

    // Signal that the reading thread should close
    halt_worker = true;
    popper.wait();

    // See if we have errors
    EXPECT_EQ(popper.get().size(), 0);
}

timings balanced {std::chrono::milliseconds(5), std::chrono::milliseconds(5), std::chrono::milliseconds(5)};
timings fastWrite {std::chrono::milliseconds(0), std::chrono::milliseconds(5), std::chrono::milliseconds(5)};
timings slowWrite {std::chrono::milliseconds(5), std::chrono::milliseconds(0), std::chrono::milliseconds(0)};
timings frontPopDelay {std::chrono::milliseconds(0), std::chrono::milliseconds(5), std::chrono::milliseconds(0)};
INSTANTIATE_TEST_SUITE_P(
        PushPopParameterized,
        PushAndPopFixture,
        testing::Values(balanced, fastWrite, slowWrite, frontPopDelay),
        // Make the test names readable
        [](const testing::TestParamInfo<PushAndPopFixture::ParamType>& info) {
            return string_format(
                "%u___%ums_postPushSleep___%ums_postFrontSleep___%ums_postPopSleep",
                info.index,
                info.param.postPushSleep.count(),
                info.param.postFrontSleep.count(),
                info.param.postPopSleep.count()
            );
        }
);

/********************************
 *
 * Testing queue writing
 *
 ********************************/
static const size_t WORKER_COUNT = 4;
static const size_t WORKER_ITERS = 500;

std::size_t baseSpeed = 100;
std::size_t baseVariation = 100;

static void queuePusher(std::queue<uint32_t>& queue, SafeGenerator& gen, uint32_t iters) {
    uint32_t num;
    for(std::uint32_t i=0; i<iters; i++) {
        num = gen.generate();
        queue.push(num);

        // std::size_t amount = baseSpeed + rand() % baseVariation;
        // std::this_thread::sleep_for(std::chrono::microseconds(amount));
    }
}

// TEST(TestQueues2, TestMultiPush) {
//     std::queue<uint32_t> queue;
//     SafeGenerator gen;
//
//     std::thread workers[WORKER_COUNT];
//     for(std::size_t n=0; n<WORKER_COUNT; n++) {
//         workers[n] = std::thread(
//             &queuePusher,
//             std::ref(queue),
//             std::ref(gen),
//             WORKER_ITERS
//         );
//     }
//
//     // Wait on all workers to finish
//     for(std::size_t n=0; n<WORKER_COUNT; n++) workers[n].join();
//
//     // Smoke test
//     EXPECT_EQ(queue.size(), WORKER_ITERS * WORKER_COUNT);
//
//     // Unpack into a vector
//     std::vector<uint32_t> vector;
//     for(std::size_t n=0; n<queue.size(); n++) {
//         vector.push_back(queue.front());
//         queue.pop();
//     }
//
//     
//     std::sort(vector.begin(), vector.end());
//     auto end = vector.end();
//     EXPECT_TRUE(std::unique(vector.begin(), vector.end()) == end);
// }


