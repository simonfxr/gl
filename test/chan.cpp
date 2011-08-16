#include "defs.hpp"
#include "data/Atomic.hpp"

#include <SFML/System/Thread.hpp>

#include <iostream>

using namespace defs;

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
        Node *next = first->next.lazyGet();

        if (next == 0) {
            next = first->next.get();
            if (next == 0)
                return false;
        }
        
        *value = first->value;
        head = next;
        delete first;
        return true;
    }
};

static const int N = 10000000;

Chan<int> chan;

static void producer(void) {
    int64 sum = 0;
    Chan<int> &c = chan;
    for (int i = 0; i < N; ++i)
        sum += i, c.put(i);
    c.put(N);

    std::cerr << "PRODUCER: " << sum << std::endl;
}

static void consumer(void) {
    int64 sum = 0;
    int i = 0;
    Chan<int> &c = chan;
    while (i < N) {
        sum += i;
        while (!c.get(&i))
            ;
    }

    std::cerr << "CONSUMER: " << sum << std::endl;
}

int main(void) {

    sf::Thread producer_thread(producer);
    sf::Thread consumer_thread(consumer);

    producer_thread.Launch();
    consumer_thread.Launch();

    producer_thread.Wait();
    consumer_thread.Wait();

    return 0;
}
