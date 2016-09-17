#ifndef CONTINUATION
#define CONTINUATION

#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include <future>


template<typename T>
auto fold(std::future<std::future<T>>&& f)
-> std::future<T>
{
    return std::async(std::launch::deferred, [](std::future<std::future<T>>&& f)
    {
        return f.get().get();
    }, std::move(f));
}


template<typename T>
auto fold(std::future<T>&& f)
-> std::future<T>
{
    return std::move(f);
}

/*
auto fold(int&& f)
-> std::future<int>
{
    return std::async(std::launch::deferred, [](int&& f)
    {
        return std::move(f);
    }, std::move(f));
}*/

template <typename R, typename... A>
class continuation
{
    public:
        continuation<R, A...>& and_then(std::function<std::future<R>(A...)> handler)
        {
            this->handler = handler;
            return *this;
        }

        continuation<R, A...>& and_then(std::function<R(A...)> handler)
        {
            auto future_wrapper = [handler](A... args) { return std::async(std::launch::async, handler, std::forward<A>(args)...); };
            this->handler = future_wrapper;
            return *this;
        }

        std::future<R> run()
        {
            return fold(std::async(std::launch::async, [this]() { return this->run_impl(); }));
        }

        virtual std::future<R> run_impl() = 0;

    protected:
        std::future<R> invoke_handler(A&&... args)
        {
            return handler(std::forward<A>(args)...);
        }

    private:
        std::function<std::future<R>(A...)> handler;
};


template <typename R, typename... A>
class creturn : public continuation<R, A...>
{
    public:
        creturn(A&&... values):
            values {std::forward<A>(values)...}
        {
        }

        std::future<R> run_impl() override
        {
            constexpr auto size = std::tuple_size<typename std::decay<decltype(values)>::type>::value;
            auto seq = std::make_index_sequence<size>{};
            return invoke_helper(seq);
        }

    private:
        template <std::size_t... I>
        std::future<R> invoke_helper(std::index_sequence<I...>)
        {
            return this->invoke_handler(std::get<I>(std::forward<decltype(values)>(values))...);
        }

        std::function<R(A...)> handler;
        std::tuple<A...> values;
};


template <typename C, typename R, typename... A>
class bind : public continuation<R, A...>
{
    public:
        bind(std::unique_ptr<C> anteced, std::function<std::unique_ptr<continuation<R, A...>>(A...)> n):
            anteced {std::move(anteced)},
            anteced_ref {nullptr},
            next {nullptr}
        {
            this->anteced->and_then([=](A&&... a) {
                continuation<R, A...>* ptr = n(std::forward<A>(a)...).release();
                next = std::shared_ptr<continuation<R, A...>> {ptr};
                std::function<std::future<R>(A...)> handler_wrapper = [=](A... a1) { return fold(this->invoke_handler(std::forward<A>(a1)...)); };
                next->and_then(handler_wrapper);
                return next->run();
            });
        }

        bind(C& anteced_ref, std::function<std::unique_ptr<continuation<R, A...>>(A...)> n):
            anteced {nullptr},
            anteced_ref {&anteced_ref},
            next(nullptr)
        {
            anteced_ref.and_then([=](A&&... a) {
                continuation<R, A...>* ptr = n(std::forward<A>(a)...).release();
                next = std::shared_ptr<continuation<R, A...>> {ptr};
                std::function<std::future<R>(A...)> handler_wrapper = [=](A... a1) { return fold(this->invoke_handler(std::forward<A>(a1)...)); };
                next->and_then(handler_wrapper);
                return next->run();
            });
        }

        std::future<R> run_impl() override
        {
            if (anteced_ref) {
                return anteced_ref->run();
            } else {
                return anteced->run();
            }
        }

    private:
        std::unique_ptr<C> anteced;
        C* anteced_ref;
        std::shared_ptr<continuation<R, A...>> next;
};


template <typename C, typename R, typename... A>
std::unique_ptr<bind<C, R, A...>> operator|(C& lhs, std::function<std::unique_ptr<continuation<R, A...>>(A...)> rhs)
{
    return std::unique_ptr<bind<C, R, A...>> {new bind<C,R,A...>(lhs, rhs)};
}

template <typename C, typename R, typename... A>
std::unique_ptr<bind<C, R, A...>> operator|(std::unique_ptr<C> lhs, std::function<std::unique_ptr<continuation<R, A...>>(A...)> rhs)
{
    return std::unique_ptr<bind<C, R, A...>> {new bind<C,R,A...>(std::move(lhs), rhs)};
}


#endif // CONTINUATION

