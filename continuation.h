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
        bind(C& anteced, std::function<std::unique_ptr<continuation<R, A>>(A)> n):
            anteced {anteced},
            next {nullptr}
        {
            anteced.and_then([=](A a) -> R {
                continuation<R, A>* ptr = n(a).release();
                //std::shared_ptr<continuation<R, A>> next {ptr};
                next = std::shared_ptr<continuation<R, A>> {ptr};
                std::function<R(A)> handler_wrapper = [=](A a1) { handler(a1); };
                next->and_then(handler_wrapper);
                next->run();
            });
        }

        void and_then(std::function<R(A)> handler)
        {
            this->handler = handler;
        }

        void run() override
        {
            //anteced.run();
        }

    private:
        C& anteced;
        std::shared_ptr<continuation<R, A>> next;
        std::function<R(A)> handler;
};

template <typename C, typename R, typename A>
bind<C, R, A> operator>>=(C& lhs, std::function<std::unique_ptr<continuation<R, A>>(A)> rhs)
{
    return bind<C,R,A>(lhs, rhs);
}


#endif // CONTINUATION

