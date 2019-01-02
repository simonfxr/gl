#include "data/Atomic.hpp"
#include "defs.hpp"
#include "sys/measure.hpp"

#include <SFML/System.hpp>
#include <SFML/System/Thread.hpp>

#include <iostream>
#include <sched.h>
#include <time.h>
#include <unistd.h>

namespace {

inline void
compiler_rw_barrier()
{
    __asm__ __volatile__("" ::: "memory");
}

template<typename T>
T
load_acquire(const T *addr)
{
    T x = *const_cast<const volatile T *>(addr);
    compiler_rw_barrier();
    return x;
}

template<typename T>
void
store_release(T *addr, const T &val)
{
    compiler_rw_barrier();
    *static_cast<volatile T *>(addr) = val;
}

struct CacheLine
{
private:
    char pad[64];
};

struct BusyWait
{
    int waits;
    BusyWait() : waits(0) {}
    void wait() __attribute__((noinline));
};

inline void
cpu_pause()
{
    __asm__ __volatile__("rep; nop" : :);
}

static void
sleep(unsigned usecs)
{
    struct timespec tp;
    tp.tv_sec = usecs / 1000000u;
    tp.tv_nsec = (usecs % 1000000u) * 1000u;
    nanosleep(&tp, 0);
}

void
BusyWait::wait()
{
    if (waits < 50)
        cpu_pause();
    else if (waits < 100)
        for (int i = 0; i < 50; ++i)
            cpu_pause();
    else if (waits < 150)
        sched_yield();
    else if (waits < 200)
        sleep(50);
    else if (waits < 300)
        sleep(100);
    else
        sleep(1000);
    ++waits;
}

typedef CacheLine CacheLineA __attribute__((aligned(64)));

static const size_t MAX_CACHE = 128;

template<typename T>
struct Chan
{
    typedef T type;

    struct Node
    {
        type value;
        Node *next;
    };

    Node *head; // read end
    size_t head_pos;
    CacheLine _pad1;

    Node *tail;       // write end
    Node *head_cache; // points to nodes already consumed
    size_t head_cache_pos;
    Node *head_copy; // lazily updated snapshot of head
    size_t head_pos_copy;
    size_t num_allocs;

    Chan()
      : head(new Node)
      , head_pos(0)
      , tail(head)
      , head_cache(head)
      , head_cache_pos(0)
      , head_copy(head)
      , head_pos_copy(0)
      , num_allocs(0)
    {}

    ~Chan()
    {
        Node *first = load_acquire(&head_cache);
        while (first != 0) {
            Node *node = first;
            first = first->next;
            delete node;
        }
    }

    Node *allocNode()
    {

        if (head_cache != head_copy) {
            Node *n = head_cache;
            head_cache = head_cache->next;
            ++head_cache_pos;
            return n;
        }

        head_copy = load_acquire(&head);
        head_pos_copy = load_acquire(&head_pos);
        if (head_cache != head_copy) {
            Node *n = head_cache;
            head_cache = head_cache->next;
            ++head_cache_pos;

            while (head_cache != head_copy && head_cache_pos < head_pos_copy &&
                   (head_pos_copy - head_cache_pos) > MAX_CACHE) {
                Node *node = head_cache;
                head_cache = head_cache->next;
                ++head_cache_pos;
                delete node;
            }

            return n;
        } else {
            ++num_allocs;
            return new Node;
        }
    }

    void put(const T &val)
    {
        Node *n = allocNode();
        n->value = val;
        n->next = 0;
        store_release(&tail->next, n);
        tail = n;
    }

    bool get(T *value)
    {
        Node *head_next = load_acquire(&head->next);
        if (head_next == 0)
            return false;
        head = head_next;
        ++head_pos;
        *value = head->value;
        return true;
    }
};

const int N = 10000000;

Chan<int> chan;
Atomic<int64> sum_producer;
Atomic<int64> sum_consumer;
sf::Mutex start_producer;
sf::Mutex start_consumer;

} // namespace

static void
producer(void)
{
    start_producer.lock();

    int64 sum = 0;
    Chan<int> &c = chan;
    for (int i = 0; i < N; ++i)
        sum += i, c.put(i);
    c.put(N);

    sum_producer.set(sum);
}

static void
consumer(void)
{
    start_consumer.lock();

    int64 sum = 0;
    int i = 0;
    Chan<int> &c = chan;
    while (i < N) {
        sum += i;
        BusyWait waiter;
        while (!c.get(&i))
            waiter.wait();
    }

    sum_consumer.set(sum);
}

int
main(void)
{

    sf::Thread producer_thread(producer);
    sf::Thread consumer_thread(consumer);

    start_producer.lock();
    start_consumer.lock();
    producer_thread.launch();
    consumer_thread.launch();
    start_producer.unlock();
    start_consumer.unlock();

    time_op(producer_thread.wait(), consumer_thread.wait());

    __sync_synchronize();

    std::cerr << "PRODUCER: " << sum_producer.get() << std::endl;
    std::cerr << "CONSUMER: " << sum_consumer.get() << std::endl;
    std::cerr << "  allocs: " << chan.num_allocs << std::endl;

    return 0;
}
