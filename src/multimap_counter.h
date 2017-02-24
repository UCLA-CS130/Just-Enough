#pragma once
#include <map>
#include <mutex>

typedef int counter_t;

// a thread-safe map to a map to counters
// for example, it can be used to count requests to uri's, grouped by status code:
//
//   MultiMapCounter<string, Response::ResponseCode> uriCounter;
//   uriCounter.increment("/foo", Response::code_200_OK);
//   uriCounter.increment("/foo", Response::code_404_not_found);
//   for (auto c : uriCounter.iterateKey("/foo"))
//       cout << c.first << ": " << c.second << endl;
//   // prints "200 : 1\n 404 : 1"
template<typename A, typename B>
class MultiMapCounter {
    public:

        void increment(A outer_key, B inner_key, counter_t delta=1) {
            std::unique_lock<std::mutex> lck(mtx_);

            // TODO: improve performance with emplace
            auto outermatch = map_.find(outer_key);
            if (outermatch == map_.end()) {
                map_[outer_key][inner_key] = delta;
            } else {
                map_[outer_key][inner_key] += delta;
            }
        }

        counter_t get(A outer_key, B inner_key) {
            std::unique_lock<std::mutex> lck(mtx_);
            return map_[outer_key][inner_key];
        }

        // return by copy so it will be threadsafe but possibly immediately outdated
        std::map<B, counter_t> iterateKey(A outer_key) {
            std::unique_lock<std::mutex> lck(mtx_);
            return map_[outer_key];
        }

    private:
        std::mutex mtx_;
        std::map<A, std::map<B, counter_t>> map_;
};
