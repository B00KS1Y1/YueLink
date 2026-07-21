#ifndef SINGLETON_H
#define SINGLETON_H

namespace Utils
{

template <typename T> class Singleton
{
public:
    static T &instance()
    {
        static T s_instance;
        return s_instance;
    }

    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};

} // namespace Utils

#endif // SINGLETON_H
