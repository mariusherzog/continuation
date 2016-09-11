#include <iostream>
#include <string>
#include <chrono>
#include <thread>

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


class timed_continuator : public continuation<void, std::string>
{
    public:
        timed_continuator(void (*fun)(void), std::string n = "q"):
            t {std::bind(&timed_continuator::handler, this), std::chrono::milliseconds(2000)},
            n {n},
            func {fun}
        {

        }

        void run() override
        {
            t.run();
        }


    private:
        timed_execution t;
        std::string n;

        void (*func)(void);

        void handler()
        {
            func();
            finished(n);
        }
};



int main()
{
    timed_continuator t(print);
    //t.andThen([](std::string value) {std::cout << value << std::flush; });
    //t.run();
    //timed_continuator t2(print2);
    //timed_continuator t3(print);
    /*bind<timed_continuator, void, std::string> b(t, t2);
    b.andThen([](std::string s) {std::cout << s << std::flush;});
    b.run();*/

    std::cout << "\n";

    std::function<std::unique_ptr<continuation<void, std::string>>(std::string)> f = [](std::string q) { return std::unique_ptr<continuation<void, std::string>>(new timed_continuator(print2, "q"+q));};
    std::function<std::unique_ptr<continuation<void, std::string>>(std::string)> f2 = [](std::string q) { return std::unique_ptr<continuation<void, std::string>>(new timed_continuator(print, "q"+q));};
    auto q = (t >>= f);
    auto x = (q >>= f2);
    x.and_then([](std::string s) {std::cout << s << std::flush;});
    //x.run();
    t.run();


    std::cout << "This first" << std::flush;

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}

