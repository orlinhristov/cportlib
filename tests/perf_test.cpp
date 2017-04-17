#include <cport/completion_port.hpp>
#include <cport/task_scheduler.hpp>
#include <atomic>
#include <ctime>
#include <iostream>
#include <unordered_map>

int main()
{
    const auto total_itmes = 1000000;

    cport::completion_port cp;
    cport::task_scheduler ts(cp);
    cport::util::thread_group tg([&]{ cp.run(); });
    
    std::unordered_map<std::thread::id, std::size_t> um;

    for (int i = 0; i < 10; i++) {
        std::mutex mtx;
        std::atomic<int> ref_count{ total_itmes };

        auto b = std::clock();

        for (auto i = 0; i < total_itmes; ++i)
        {
            ts.async(
                [&](cport::generic_error&){
                std::lock_guard<std::mutex> lock(mtx);
                ++um[std::this_thread::get_id()];
            }
                ,
                [&](const cport::generic_error&){
                --ref_count;
                std::lock_guard<std::mutex> lock(mtx);
                ++um[std::this_thread::get_id()];
            });
        }

        while (ref_count > 0)
        {
            std::this_thread::yield();
        }

        std::cout << "time elapsed = " << (double)(clock() - b) / CLOCKS_PER_SEC << std::endl;
    }

    cp.stop();
        
    tg.join();
/*    
    std::cout << "time elapsed = " << (double)(clock() - b)/CLOCKS_PER_SEC << std::endl;

    for (auto& vt : um)
    {
        std::cout << "[" << vt.first << "] = " << vt.second << std::endl;
    }
*/
    return 0;
}