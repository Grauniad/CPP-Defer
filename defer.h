#include <functional>
/*
 * Some notes on the c++11 features used to make this work
 *
 * *** References of References ***
 * (see: http://thbecker.net/articles/rvalue_references/section_07.html, 
 *       http://thbecker.net/articles/rvalue_references/section_08.html )
 * In c++11 it is legitimate to take a reference to a reference, since c++11 
 * introduces reference collapsing rules:
 *   (T&)&   -> T&
 *   (T&&)&  -> T&
 *   (T&)&&  -> T&
 *   (T&&)&& -> T&&
 *
 * *** Perfect forwarding ***
 * Specifying that a template function takes an r-value, allows to pass EITHER 
 * an l-value ref or an r-value ref (by a result of the above):
 *    <template T>
 *    void f(T&& a) { 
 *       doSomething<T>(a); 
 *    }
 *    long l = 10;
 *    f(l); // resolves to f<long&>(l) -> f(long& l) { doSomething<long& >(l); }
 *    f(10); //      f<long&&>(10) -> f(long&& l=10) { doSomething<long&&>(l); }
 *
 * *** Lambda Expressions **
 * (see: http://www.cprogramming.com/c++11/c++11-lambda-closures.html )
 *      string name = "Grauniad";
 *      // capture nothing
 *      std::function<void(string)> f = [] (string s) -> long { 
 *                                        cout << "hello, " << s << endl; 
 *                                        return 0; 
 *                                     };
 *      // capture any named variables by value
 *      auto fvalue = [=] () { 
 *                             //name = "lukeh"; // compile time error: is const 
 *                             cout << "hello, " << name << endl; 
 *                           };
 *      // capture any named variables by reference
 *      auto fref   = [&] () { 
 *                             name = "luke"; 
 *                             cout << "hello, " << name << endl; 
 *                           };
 *      // name == "luke"
 *
 *
 * *** Variadic templates ***
 * (see: http://en.wikipedia.org/wiki/Variadic_templates)
 * A template can now take an arbitrary number of arguments:
 *    template<typename... Args>
 *    void f(Args&&... args) {
 *        sum(args...);      //sum(arg1, arg2, arg3);
 *        sum(val(args)...); //sum(val(arg1), val(arg2), val(arg3);
 *    }
 * When the ... operator occurrs before a variable name, it declares a 
 * /parameter pack/
 * When the ... operator occurs afterwards it defines an expansion, it expands
 * the /expression/ that precedes it (notes the second example)
 *
 * *** std::bind ***
 * (see: http://en.cppreference.com/w/cpp/utility/functional/bind)
 * Is a standard library utility function to wrap a function, and "bind" some of its arguments.
 * Here we use it to reduce some function of many args to a void f(void);
 *
 * *** std::forward ***
 * (see: http://en.cppreference.com/w/cpp/utility/forward)
 * Used to "forward" a variable to another function, ensuring its gets it in 
 * the same state we did (const flags, ref-type etc, is preserved)
 */
using namespace std;

/*
 * Simple class that takes a function pointer which it calls when it is 
 * destroyed. Function must be of the form (we don't care about the return type):
 *      void fp(void);
 *
 * The copy constructors are disabled to prevent the function being
 * called twice.
 *
 * The move constructor is implemented to allow it to be returned from
 * a factory function.
 */
class RunOnDeath {
public:
    RunOnDeath(std::function<void()>&& f): proc(f) {}

    ~RunOnDeath() {  
        // if the func-pointer hasn't been null'd, execute it
        if ( proc ) 
            proc();
    }

    // copying would result in the action being done twice!
    RunOnDeath(const RunOnDeath& rhs) = delete;
    RunOnDeath& operator=(const RunOnDeath & rhs) = delete;
    RunOnDeath& operator=(RunOnDeath && rhs) = delete;

    // Enable the temp returned from defer to be moved
    RunOnDeath(RunOnDeath&& src): proc(src.proc) {
        // null the target, preventing the src from triggering the call
        src.proc = nullptr;
    }
private:
    std::function<void()> proc;
};


/*
 * Convert an arbitrary function call to the form void fp(void) and 
 * return an object that will run that function when it is destroyed.
 *
 * Arguments should be valid for a call to std::bind 
 *
 * For the DEFER statement this bind is unnecessary, but this allows any 
 * function call to be bound without having to declare a lambda
 */
template<typename... TypePack>
RunOnDeath defer(TypePack&&...bindArg) {
    return RunOnDeath(std::bind(std::forward<TypePack>(bindArg)...));
}


/* 
 * Finally declare some helpful pre-processor
 */
// Take the macros x and y and return x_y
#define PASTER( x, y) x ## _ ## y
#define EVALUATOR(x, y) PASTER(x, y)

// substitute in the strings var and cmd
#define CMD_BUILDER(var, cmd) auto var = defer( [&] () { cmd });

/* Declare a lampda and create a RunOnDeath that wraps it, allowing:
 *
 * DEFER
 * (
 *     // Clean up code will be executed when the stack is cleaned up
 *     delete resource;
 *     parent.Notify(SHUTTING_DOWN);
 * )
 * Note we're appending the line number onto the end, to support multiple 
 * DEFER statements per stack layer
 */
#define DEFER(cmd) CMD_BUILDER(EVALUATOR(dummy,__LINE__),cmd)
