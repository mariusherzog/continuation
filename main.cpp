#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "continuation.h"


struct timed_execution {
    //typedef void (*func_type)(void);
    using func_type = std::function<void(void)>;
    timed_execution(func_type func, const std::chrono::milliseconds period)
        : func_(func)
        , period_(period)
    {
    }

    void run()
    {
        thread_ = std::thread(std::bind(&timed_execution::threadFunc,this));
    }

private:
    void threadFunc() {
        //while(true) {
            std::this_thread::sleep_for(period_);
            func_();
        //}
    }
    func_type func_;
    const std::chrono::milliseconds period_;
    std::thread thread_;
};

void print()
{
    std::cout << "Then this" << std::flush;
}


class timed_continuator : public continuation<void, std::string>
{
    public:
        timed_continuator(void (*fun)(void)):
            t {std::bind(&timed_continuator::handler, this), std::chrono::milliseconds(2000)},
            func {fun}
        {
        }

        void run()
        {
            t.run();
        }


    private:
        timed_execution t;

        void (*func)(void);

        void handler()
        {
            func();
            finished("test");
        }
};



int main()
{
//    timed_execution t(print,std::chrono::milliseconds(2000));
//    t.run();

    timed_continuator t(print);
    t.andThen([](std::string value) {std::cout << value << std::flush; });
    t.run();


    std::cout << "This first" << std::flush;

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}

