#ifndef CONTINUATION
#define CONTINUATION

#include <functional>

template <typename R, typename A>
class continuation
{
    public:
        void and_then(std::function<R(A)> handler)
        {
            this->handler = handler;
        }

        virtual void run() = 0;

    protected:
        virtual R finished(A arg)
        {
            return handler(arg);
        }

    private:
        std::function<R(A)> handler;
};

template <typename R, typename A>
class creturn : public continuation<R, A>
{
    public:
        creturn(A value):
            value {value}
        {
        }

        void and_then(std::function<R(A)> handler)
        {
            this->handler = handler;
        }

        void run() override
        {
            finished(value);
        }

    private:
        std::function<R(A)> handler;
        A value;
};


template <typename C, typename R, typename A>
class bind : public continuation<R, A>
{
    public:
        bind(C& anteced, continuation<R, A>& next):
            anteced {anteced}
        {
            anteced.and_then([&](A a) -> R {
                std::function<R(A)> handler_wrapper = [=](A) { handler(a); };
                next.and_then(handler_wrapper);
                next.run();
            });
        }

        void and_then(std::function<R(A)> handler)
        {
            this->handler = handler;
        }

        void run()
        {
            anteced.run();
        }

    private:
        C& anteced;
        std::function<R(A)> handler;
};

template <typename R, typename A>
bind<continuation<R,A>, R, A> operator>>=(continuation<R, A>& lhs, continuation<R, A>& rhs)
{
    return bind<continuation<R,A>,R,A>(lhs, rhs);
}


#endif // CONTINUATION

