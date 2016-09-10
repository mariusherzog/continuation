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

    protected:
        R finished(A arg)
        {
            return handler(arg);
        }

    private:
        std::function<R(A)> handler;
};


#endif // CONTINUATION

