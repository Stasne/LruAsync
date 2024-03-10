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
