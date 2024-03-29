#include <cache/lru-async.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <latch>
#include <thread>

class DummyData
{
public:
    DummyData() = default;
    DummyData(int i)
        : val(i)
        , d_(10000){};

    int val;

private:
    std::vector<long long> d_;
};

template <typename Fn>
void RunWorkers(int n, const Fn& fn)
{
    std::vector<std::thread> workers;
    workers.reserve(n - 1);
    n = std::max(1, n);

    while (--n)
    {
        workers.emplace_back(fn, n);
    }
    fn(n);
    for (auto& t : workers)
        t.join();
}
void DoubleBeginTest()
{
    DummyData            d1(1);
    DummyData            d2(2);
    DummyData            d3(3);
    std::list<DummyData> srclist;
    srclist.push_front(d3);
    srclist.push_front(d2);

    std::list<DummyData> list;
    list.push_front(d1);
    auto it1 = list.begin();

    list.splice(it1, list, srclist.begin());
    list.splice(it1, list, srclist.begin());

    std::cout << "splice result\n";
    for (auto& dum : list)
        std::cout << dum.val << " ";
}

int main(int argc, char** argv)
{
    std::cout << "Hello, World!";
    size_t threadCount{ 100 };
    size_t cacheSize{ 10000 };

    cache::LruAsync<size_t, DummyData> cache(cacheSize);

    std::latch barr(threadCount);
    RunWorkers(threadCount, [&cache, cacheSize, threadCount, &barr](size_t threadNum) {
        size_t startIndex = threadNum * cacheSize / threadCount;
        size_t endIndex   = startIndex + cacheSize / threadCount;
        std::cout << "Thread " << threadNum << " from " << startIndex << " to " << endIndex << '\n';

        barr.arrive_and_wait();

        for (int i = startIndex; i < endIndex; ++i)
            cache.put(i, DummyData(i));

        for (auto i = startIndex; i < endIndex; ++i)
        {
            auto val = cache.get(i);
            assert(val->val == i);
        }
    });

    return 0;
}
