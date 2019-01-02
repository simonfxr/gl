#include "defs.hpp"

#ifndef REF_CONCURRENT
#define REF_CONCURRENT
#endif

#include "data/Atomic.hpp"
#include "data/Ref.hpp"
#include "data/SpinLock.hpp"
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/ThreadLocalPtr.hpp>

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>

typedef defs::index index_t;

typedef index_t ThreadID;

static const index_t NUM_THREADS = 32;
static const index_t DATA_SIZE = 400000;

static sf::Mutex printLock;

sf::ThreadLocalPtr<ThreadID> thread_id;

static void
acq(sf::Mutex &mtx)
{
    mtx.lock();
}

static void
rel(sf::Mutex &mtx)
{
    mtx.unlock();
}

struct Data
{
    int id;
    Atomic<int32> alive;
    int value;

    Atomic<int32> num_alive;

    Data(int _id, int _value) : id(_id), alive(_id), value(_value)
    {
        num_alive.inc();
    }

    ~Data()
    {
        int32 dead = 0xDEADBEEF;
        bool ok = alive.xchg(id, &dead);
        ASSERT(ok);
        num_alive.dec();

        // acq(printLock);
        // std::cerr << "[Thread " << *thread_id << "] deleting data " << id <<
        // std::endl; rel(printLock);
    }

    int compute()
    {
        ASSERT(alive.get() == id);
        return value;
    }
};

static Ref<Data> NULL_DATA;

// static SpinLock data_lock;
static sf::Mutex data_lock;
static Ref<Data> *data = 0;

static struct
{
    sf::Mutex init_lock;
    sf::Mutex start_lock;
    unsigned int rand_state;
    bool is_started;
    long computed_sum;
} thread_data[NUM_THREADS];

#define THREAD (thread_data[*thread_id])

static void
gen_data(Ref<Data> *data, index_t num)
{
    for (index_t i = 0; i < num; ++i)
        data[i] = new Data(i, i * 3 + 11);
}

static void
fill_data(Ref<Data> *localData)
{
    for (index_t i = 0; i < DATA_SIZE; ++i)
        localData[i] = data[i];

    for (index_t i = 0; i < DATA_SIZE - 1; ++i) {
        index_t j = i + rand_r(&THREAD.rand_state) % (DATA_SIZE - i);
        Ref<Data> tmp = localData[i];
        localData[i] = localData[j];
        localData[j] = tmp;
    }
}

static void
worker(ThreadID id)
{
    thread_id = &id;

    acq(THREAD.start_lock);
    THREAD.is_started = true;

    Ref<Data> LOCAL_NULL_DATA;
    Ref<Data> *localData = new Ref<Data>[DATA_SIZE];
    fill_data(localData);

    // acq(printLock);
    // std::cerr << "[Thread " << id << "] initialized" << std::endl;
    // rel(printLock);

    rel(THREAD.start_lock);
    acq(THREAD.init_lock);

    int64 sum = 0;

    for (index_t i = 0; i < DATA_SIZE; ++i) {
        sum += localData[i]->compute();
        localData[i] = LOCAL_NULL_DATA;
    }

    delete[] localData;

    // acq(printLock);
    // std::cerr << "[Thread " << id << "] computed sum: " << sum << std::endl;
    // rel(printLock);

    THREAD.computed_sum = sum;
}

int
main(void)
{

    sf::Thread *threads[NUM_THREADS];
    data = new Ref<Data>[DATA_SIZE];
    gen_data(data, DATA_SIZE);

    unsigned garbage[64];
    unsigned seed = 0;
    for (index_t i = 0; i < ARRAY_LENGTH(garbage); ++i)
        seed = ((seed << 11) | (seed >> 21)) ^ garbage[i];
    srand(seed);

    for (index_t i = 0; i < NUM_THREADS; ++i) {
        acq(thread_data[i].init_lock);
        thread_data[i].rand_state = rand();
        threads[i] = new sf::Thread(worker, i);
        threads[i]->launch();
    }

    acq(printLock);
    std::cerr << "[main] Threads started" << std::endl;
    rel(printLock);

    for (index_t i = 0; i < NUM_THREADS; ++i) {
        for (;;) {
            acq(thread_data[i].start_lock);
            if (thread_data[i].is_started)
                break;
            rel(thread_data[i].start_lock);
        }
    }

    acq(printLock);
    std::cerr << "[main] Threads initialized" << std::endl;
    rel(printLock);

    delete[] data;
    data = 0;

    for (index_t i = 0; i < NUM_THREADS; ++i)
        rel(thread_data[i].init_lock);

    acq(printLock);
    std::cerr << "[main] Threads begin work" << std::endl;
    rel(printLock);

    for (index_t i = 0; i < NUM_THREADS; ++i) {
        acq(printLock);
        std::cerr << "Waiting for thread: " << i << std::endl;
        rel(printLock);

        threads[i]->wait();

        acq(printLock);
        std::cerr << "Thread finished: " << i << std::endl;
        rel(printLock);
    }

    for (index_t i = 0; i < NUM_THREADS; ++i)
        delete threads[i];

    bool failed = false;
    for (index_t i = 1; i < NUM_THREADS; ++i) {
        if (thread_data[i].computed_sum != thread_data[i].computed_sum) {
            std::cerr << "ERROR: computed sums inconsistent!" << std::endl;
            failed = true;
        }
    }

    if (!failed) {
        std::cerr << "SUCCESS: all sums are consistent" << std::endl;
    }

    return failed ? 1 : 0;
}
