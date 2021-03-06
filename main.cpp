#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>

#include "continuation.h"


struct timed_execution
{
    using func_type = std::function<void(void)>;
    timed_execution(func_type func, const std::chrono::milliseconds period)
        : func_(func)
        , period_(period)
    {
    }

    void run()
    {
        thread_ = std::thread(std::bind(&timed_execution::thread_func,this));
    }

private:
    void thread_func()
    {
        std::this_thread::sleep_for(period_);
        func_();
    }
    func_type func_;
    const std::chrono::milliseconds period_;
    std::thread thread_;
};

void print()
{
    std::cout << "Then this" << std::flush;
}

void print2()
{
    std::cout << "Then this2" << std::flush;
}


class timed_continuator : public continuation<int, std::string, int>
{
    public:
        timed_continuator(void (*fun)(void), std::string n = "q"):
            n {n},
            func {fun}
        {

        }

        std::future<int> run_impl(std::string s, int) override
        {
            //t.run();
            func();
            std::this_thread::sleep_for(std::chrono::milliseconds(1750));
            auto val =  invoke_handler(s + "i", 13);
            return std::async(std::launch::deferred, [](std::future<int>&& f){ return f.get()+101;}, std::move(val));
            //return std::async(std::launch::deferred, [](){return 20;});
        }

    private:
        std::string n;

        void (*func)(void);
};



int main()
{
    timed_continuator t(print);

    std::cout << "\n";


    auto f = timed_continuator::pointer (new timed_continuator(print2, ""));
    auto f2 = timed_continuator::pointer (new timed_continuator(print, ""));

    auto y =  t
              | timed_continuator::pointer { new timed_continuator(print2, "") }
              | (std::move(f) | std::move(f2))
                << timed_continuator::condition ([](std::string s, int) { return s.length() < 3; })
              | timed_continuator::pointer { new timed_continuator(print, "")}
              ;
    y->and_then([](std::string s, int) {
        std::cout << s << std::flush; return 97; });
    auto fut = y->run("", 0);


    std::cout << "This first" << std::flush;

    std::this_thread::sleep_for(std::chrono::seconds(30));

    std::cout << "\n" << fut.get();

    return 0;
}

