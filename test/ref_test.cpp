#include "defs.hpp"

#ifndef REF_CONCURRENT
#define REF_CONCURRENT
#endif

#include "data/Ref.hpp"
#include "data/SpinLock.hpp"
#include "data/Atomic.hpp"
#include <SFML/System/Thread.hpp>
#include <SFML/System/ThreadLocalPtr.hpp>
#include <SFML/System/Mutex.hpp>

#include <iostream>
#include <vector>
#include <sstream>

typedef index_t ThreadID;

static const index_t NUM_THREADS = 4;
static const index_t DATA_SIZE = 4000000;

static SpinLock printLock;

sf::ThreadLocalPtr<ThreadID> thread_id;

struct Data {
    int id;
    unsigned alive;
    int value;

    Atomic<uint32> num_alive;

    Data(int _id, int _value) :
        id(_id), alive(id), value(_value)
    {
        num_alive.inc();
    }

    ~Data() {
        ASSERT(alive != 0xDEADBEEF);
        alive = 0xDEADBEEF;
        num_alive.dec();

        // printLock.acquire();
        // std::cerr << "[Thread " << *thread_id << "] deleting data " << id << std::endl;
        // printLock.release();        
    }

    int compute() {
        return value;
    }
};

static Ref<Data> NULL_DATA;

//static SpinLock data_lock;
static sf::Mutex data_lock;
static Ref<Data> *data = 0;

static SpinLock init_locks[NUM_THREADS];
static SpinLock start_locks[NUM_THREADS];
volatile long computed_sums[NUM_THREADS];

static void gen_data(Ref<Data> *data, index_t num) {
    for (index_t i = 0; i < num; ++i)
        data[i] = new Data(i, i * 3 + 11);
}

static void fill_data(Ref<Data> *localData) {
    for (index_t i = 0; i < DATA_SIZE; ++i)
        localData[i] = data[i];

    data_lock.Lock();

    for (index_t i = 0; i < DATA_SIZE - 1; ++i) {
        index_t j = i + rand() % (DATA_SIZE - i);
        Ref<Data> tmp = localData[i];
        localData[i] = localData[j];
        localData[j] = tmp;
    }
    
    data_lock.Unlock();
}


void acq_lock(SpinLock *lock) {
    lock->acquire();
}

void rel_lock(SpinLock *lock) {
    lock->release();
}

void del_ref(Ref<Data> *ref) {
    delete ref;
}

static void worker(ThreadID id) {

    thread_id = &id;

    Ref<Data> LOCAL_NULL_DATA;
    Ref<Data> *localData = new Ref<Data>[DATA_SIZE];
    fill_data(localData);

    printLock.acquire();
    std::cerr << "[Thread " << id << "] initialized" << std::endl;
    printLock.release();

    init_locks[id].release();
    start_locks[id].acquire();

    int64 sum = 0;

    for (index_t i = 0; i < DATA_SIZE; ++i) {
        sum += localData[i]->compute();
        localData[i] = LOCAL_NULL_DATA;
    }

    delete[] localData;

    printLock.acquire();
    std::cerr << "[Thread " << id << "] computed sum: " << sum << std::endl;
    printLock.release();

    computed_sums[id] = sum;
}

int main(void) {

    sf::Thread *threads[NUM_THREADS];
    data = new Ref<Data>[DATA_SIZE];
    gen_data(data, DATA_SIZE);

    for (index_t i = 0; i < NUM_THREADS; ++i) {
        init_locks[i].acquire();
        start_locks[i].acquire();
        threads[i] = new sf::Thread(worker, i);
        threads[i]->Launch();
    }

    printLock.acquire();
    std::cerr << "[main] Threads started" << std::endl;
    printLock.release();

    for (index_t i = 0; i < NUM_THREADS; ++i)
        init_locks[i].acquire();

    printLock.acquire();
    std::cerr << "[main] Threads initialized" << std::endl;
    printLock.release();

    delete[] data;
    data = 0;

    for (index_t i = 0; i < NUM_THREADS; ++i)
        start_locks[i].release();

    printLock.acquire();
    std::cerr << "[main] Threads begin work" << std::endl;
    printLock.release();

    for (index_t i = 0; i < NUM_THREADS; ++i) {
        printLock.acquire();
        std::cerr << "Waiting for thread: " << i << std::endl;
        printLock.release();
        
        threads[i]->Wait();
        
        printLock.acquire();
        std::cerr << "Thread finished: " << i << std::endl;
        printLock.release();
    }

    for (index_t i = 0; i < NUM_THREADS; ++i)
        delete threads[i];

    bool failed = false;
    for (index_t i = 1; i < NUM_THREADS; ++i) {
        if (computed_sums[i] != computed_sums[0]) {
            std::cerr << "ERROR: computed sums inconsistent!" << std::endl;
            failed = true;
        }
    }

    if (!failed) {
        std::cerr << "SUCCESS: all sums are consistent" << std::endl;
    }

    return 0;
}
