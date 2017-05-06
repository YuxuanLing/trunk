#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
 
struct Base
{
    Base() { std::cout << "  Base::Base()\n"; }
    // 注意：这里可以使用非虚的析构器
    ~Base() { std::cout << "  Base::~Base()\n"; }
};
 
struct Derived: public Base
{
    Derived() { std::cout << "  Derived::Derived()\n"; }
    ~Derived() { std::cout << "  Derived::~Derived()\n"; }
};
 
void thr(std::shared_ptr<Base> p, int n)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
	//std::cout << "thread num:" << n << '\n';
    std::shared_ptr<Base> lp = p; // 类型安全，即使
                                  // 共享的 use_count 增加
	
	{
        static std::mutex io_mutex;
        std::lock_guard<std::mutex> lk(io_mutex);
        std::shared_ptr<Base> lp = p; // 类型安全，即使
     	std::cout << "local pointer in a thread:"<<n<<'\n'
                  << "  lp.get() = " << p.get()
                  << ", lp.use_count() = " << p.use_count() << '\n';
    }
}
 
int main()
{
    std::shared_ptr<Base> p = std::make_shared<Derived>();
   // std::thread t1(thr, p, 0); //, t2(thr, p, 1), t3(thr, p, 2);
    std::cout << "Created a shared Derived (as a pointer to Base)\n"
              << "  p.get() = " << p.get()
              << ", p.use_count() = " << p.use_count() << '\n';
    std::thread t1(thr, p, 0), t2(thr, p, 1), t3(thr, p, 2);
    //std::thread  t2(thr, p, 1), t3(thr, p, 2);
    //p.reset(); // 从 main 中释放所有权
    std::cout << "Shared ownership between 3 threads and released\n"
              << "ownership from main:\n"
              << "  p.get() = " << p.get()
              << ", p.use_count() = " << p.use_count() << '\n';
    t1.join(); t2.join(); t3.join();
    std::cout << "All threads completed, the last one deleted Derived\n";
	getchar();
}
