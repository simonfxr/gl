#include "defs.hpp"
#include "data/Atomic.hpp"
#include "sys/measure.hpp"

#include <SFML/System/Thread.hpp>
#include <SFML/System.hpp>

#include <iostream>

using namespace defs;

namespace {

template <typename T>
void set_volatile(T& ref, const T& val) {
    volatile T * loc = &ref;
    *loc = val;
}

template <typename T>
T get_volatile(const T& ref) {
    volatile const T * loc = &ref;
    return *loc;
}

volatile int foo;

template <typename T>
struct Chan {
    typedef T type;

    struct Node {
        type value;
        Atomic<Node *> next;

        Node() :
            value(0xFFFFFF),
            next(0) {}
    };

    Node *head;
    Node *tail;

    Chan() :
        head(0),
        tail(0)
    {
        Node *first = new Node;
        head = first;
        set_volatile(tail, first);        
    }

    void put(const T& val) {
        Node *last = tail;
        Node *node = new Node;
        last->value = val;
        last->next.set(node);
        tail = node;
    }

    bool get(T *value) {
        Node *first = head;
        Node *next = first->next.get();

        if (next == 0) {
            // next = first->next.get();
            // if (next == 0)
                return false;
        }
        
        *value = first->value;
        head = next;
        delete first;
        return true;
    }
};

const int N = 10000000;

Chan<int> chan;
Atomic<int64> sum_producer;
Atomic<int64> sum_consumer;
sf::Mutex start_producer;
sf::Mutex start_consumer;


}

static void producer(void) {
    start_producer.Lock();

    int64 sum = 0;
    Chan<int> &c = chan;
    for (int i = 0; i < N; ++i)
        sum += i, c.put(i);
    c.put(N);

    sum_producer.set(sum);
}

static void consumer(void) {
    start_consumer.Lock();

    int64 sum = 0;
    int i = 0;
    Chan<int> &c = chan;
    while (i < N) {
        sum += i;
        while (!c.get(&i))
            ;
    }

    sum_consumer.set(sum);
}

int main(void) {

    sf::Thread producer_thread(producer);
    sf::Thread consumer_thread(consumer);

    start_producer.Lock();
    start_consumer.Lock();
    producer_thread.Launch();
    consumer_thread.Launch();
    start_producer.Unlock();
    start_consumer.Unlock();

    time_op(producer_thread.Wait(),
            consumer_thread.Wait());

    std::cerr << "PRODUCER: " << sum_producer.get() << std::endl;
    std::cerr << "CONSUMER: " << sum_consumer.get() << std::endl;

    return 0;
}
