#ifndef CONTINUATION
#define CONTINUATION

#include <functional>

template <typename R, typename A>
class continuation
{
    public:
        R andThen(std::function<R(A)> handler)
        {
            this->handler = handler;
        }

        virtual void run()
        {
        }

    protected:
        virtual R finished(A arg)
        {
            return handler(arg);
        }

    private:
        std::function<R(A)> handler;
};


template <typename C, typename R, typename A>
class bind : public continuation<R, A>
{
    public:
        bind(C& anteced, continuation<R, A>& next, std::function<R(A)> handler):
            anteced {anteced}
        {
            anteced.andThen([&](A a) -> R {
                argument = a;
                std::function<R(A)> handler_wrapper = [=](A) mutable { handler(argument); };
                next.andThen(handler_wrapper);
                next.run();
            });
        }

        void run()
        {
            anteced.run();
        }

    private:
        C& anteced;
        A argument;
};


#endif // CONTINUATION

