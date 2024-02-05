// test_WTimer.cpp: as it says
//   Testing:
//   - functionality
//   - execute periodically some events: print a Message or, send Random periodic packets
//     from Source to Destination using TCP/UDP Sockets or,
//   - use Message Queue synchronized
//

#include "Logger_decl.hpp"
#include "Logger_helpers.hpp"

#include "src/wheel_timer.hpp"

void* func(void* p, size_t s) {
   Log_to(0, "> from WTImerCB_t: @ ", LOG_TIME_LAPSE(Log_start())) ;
   return nullptr ;
}

int main()
{
   Log_to(0, "> Wheel TIMER testing ...", LOG_TIME_LAPSE(Log_start()), '\n') ;

   {  // Timer's Life block
      // 10 slots, period: 1 sec, delay correction 150 micros, debug capacity 150,
      uint32_t   period = 50 ;
      cWTimer_   timer{10, period, 170, 500, "Wheel_Timer_I"} ;
                                                               // Log_to(0, "> Timer created as: ", timer, '\n') ;

      AppCB_   cb1{func} ;

      timer.register_event(cWTimerEvent_{2, true, func, true}) ;
      timer.register_event(cWTimerEvent_{4, true, func, true}) ;
      // timer.register_event(cWTimerEvent_{6}) ;
      // timer.register_event(cWTimerEvent_{11}) ;
                                                               Log_to(0, "\n> Timer state: ", timer, '\n') ;
      timer.start() ;
      std::this_thread::sleep_for(30s) ;
      timer.stop() ;
                                                               Log_to(0, "\n> Final state: ", timer, '\n') ;
      std::this_thread::sleep_for(std::chrono::microseconds(period)) ;  // let it stop completely
   }

   Log_to(0, "\n> That's it...", LOG_TIME_LAPSE(Log_start()), '\n') ;
   return 0;
}


#ifdef CCCCCCCChrono

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

void slow_motion()
{
    static int a[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    // Generate Γ(13) == 12! permutations:
    for (size_t i = 0 ; i <1000000000 ; ++i) ;// while (std::ranges::next_permutation(a).found) {}
}


int main()
{
    using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

    const std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();

    const std::time_t t_c = std::chrono::system_clock::to_time_t(now - 24h);
    std::cout << "24 hours ago, the time was "
              << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;

    const std::chrono::time_point<std::chrono::steady_clock> start =
        std::chrono::steady_clock::now();


    slow_motion();

    const auto end = std::chrono::steady_clock::now();
    std::cout
        << "Slow calculations took "
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " ≈ "
        << (end - start) / 1ms << "ms ≈ " // almost equivalent form of the above, but
        << (end - start) / 1s << "s.\n";  // using milliseconds and seconds accordingly
}
#endif

#ifdef __bind

#include <functional>
#include <iostream>
#include <memory>
#include <random>

void f(int n1, int n2, int n3, const int& n4, int n5)
{
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

int g(int n1)
{
    return n1;
}

struct Foo
{
    void print_sum(int n1, int n2)
    {
        std::cout << n1 + n2 << '\n';
    }

    int data = 10;
};

int main()
{
    using namespace std::placeholders;  // for _1, _2, _3...

    std::cout << "1) argument reordering and pass-by-reference: ";
    int n = 7;
    // (_1 and _2 are from std::placeholders, and represent future
    // arguments that will be passed to f1)
    auto f1 = std::bind(f, _2, 42, _1, std::cref(n), n);
    n = 10;
    f1(1, 2, 1001); // 1 is bound by _1, 2 is bound by _2, 1001 is unused
                    // makes a call to f(2, 42, 1, n, 7)

    std::cout << "2) achieving the same effect using a lambda: ";
    n = 7;
    auto lambda = [&ncref = n, n](auto a, auto b, auto /*unused*/)
    {
        f(b, 42, a, ncref, n);
    };
    n = 10;
    lambda(1, 2, 1001); // same as a call to f1(1, 2, 1001)

    std::cout << "3) nested bind subexpressions share the placeholders: ";
    auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5);
    f2(10, 11, 12); // makes a call to f(12, g(12), 12, 4, 5);

    std::cout << "4) bind a RNG with a distribution: ";
    std::default_random_engine e;
    std::uniform_int_distribution<> d(0, 10);
    auto rnd = std::bind(d, e); // a copy of e is stored in rnd
    for (int n = 0; n < 10; ++n)
        std::cout << rnd() << ' ';
    std::cout << '\n';

    std::cout << "5) bind to a pointer to member function: ";
    Foo foo;
    auto f3 = std::bind(&Foo::print_sum, &foo, 95, _1);
    f3(5);

    std::cout << "6) bind to a mem_fn that is a pointer to member function: ";
    auto ptr_to_print_sum = std::mem_fn(&Foo::print_sum);
    auto f4 = std::bind(ptr_to_print_sum, &foo, 95, _1);
    f4(5);

    std::cout << "7) bind to a pointer to data member: ";
    auto f5 = std::bind(&Foo::data, _1);
    std::cout << f5(foo) << '\n';

    std::cout << "8) bind to a mem_fn that is a pointer to data member: ";
    auto ptr_to_data = std::mem_fn(&Foo::data);
    auto f6 = std::bind(ptr_to_data, _1);
    std::cout << f6(foo) << '\n';

    std::cout << "9) use smart pointers to call members of the referenced objects: ";
    std::cout << f6(std::make_shared<Foo>(foo)) << ' '
              << f6(std::make_unique<Foo>(foo)) << '\n';
}
#endif
