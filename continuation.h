#ifndef CONTINUATION
#define CONTINUATION

#include <functional>
#include <memory>

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
        bind(std::unique_ptr<C> anteced, std::function<std::unique_ptr<continuation<R, A>>(A)> n):
            anteced {std::move(anteced)},
            anteced_ref {nullptr},
            next {nullptr}
        {
            this->anteced->and_then([=](A a) -> R {
                continuation<R, A>* ptr = n(a).release();
                //std::shared_ptr<continuation<R, A>> next {ptr};
                next = std::shared_ptr<continuation<R, A>> {ptr};
                std::function<R(A)> handler_wrapper = [=](A a1) { handler(a1); };
                next->and_then(handler_wrapper);
                next->run();
            });
        }

        bind(C& anteced_ref, std::function<std::unique_ptr<continuation<R, A>>(A)> n):
            anteced {nullptr},
            anteced_ref {&anteced_ref},
            next(nullptr)
        {
            anteced_ref.and_then([=](A a) -> R {
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
            if (anteced_ref) {
                anteced_ref->run();
            } else {
                anteced->run();
            }
        }

    private:
        std::unique_ptr<C> anteced;
        C* anteced_ref;
        std::shared_ptr<continuation<R, A>> next;
        std::function<R(A)> handler;
};


template <typename C, typename R, typename A>
std::unique_ptr<bind<C, R, A>> operator|(C& lhs, std::function<std::unique_ptr<continuation<R, A>>(A)> rhs)
{
    return std::unique_ptr<bind<C, R, A>> {new bind<C,R,A>(lhs, rhs)};
}

template <typename C, typename R, typename A>
std::unique_ptr<bind<C, R, A>> operator|(std::unique_ptr<C> lhs, std::function<std::unique_ptr<continuation<R, A>>(A)> rhs)
{
    return std::unique_ptr<bind<C, R, A>> {new bind<C,R,A>(std::move(lhs), rhs)};
}


#endif // CONTINUATION

