CPP-Defer
=========

A small header that adds a Go style defer statement.

```c+++
void f () {
    int *dynamicArray = new int[10];
    DEFER (
        // code executes when the function returns or an error is thrown
        delete [] dynamicArray;
    )
    // ...
    // do something with the array
    // ...
    return;
}
````

Based on the blog post here: http://blog.korfuri.fr/post/go-defer-in-cpp/ 
