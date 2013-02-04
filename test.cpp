#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include "defer.h"


class HelperObject {
public:
    void Print(string s) { cout << s << endl;}
    HelperObject& operator<<(const string &rhs) {
        log << "   " << rhs << endl;
        return *this;
    }
    void DumpLog() { cout << log.str() << endl;}
private:
    stringstream log;
};

int sqr( int input) {
    if (input >= 1000 )
        throw string("Input is too large!");
    else
        return input*input;
}

/*
 * An obviously stupid squaring function that can throw errors, or return 
 * early, but will still clean up after itself...
 *
 */
int SquareSmallNum ( int input ) {
    // this example is paricuarly silly, really we should use unique_ptr here
    HelperObject *helper  = new HelperObject();
    DEFER (
        // clean up code to exit when ever we leave the function
        helper->Print("Performing final clean up");
        cout << "Log follows: " << endl;
        helper->DumpLog();
        delete  helper;
        cout << "All done..." << endl << endl;
    )

    *helper << "Checking for a positive input";
    if ( input < 0 ) {
        *helper << "Inavlid input, exiting";
        return -1;
    } 

    *helper << "Calling some library function that might throw";
    long ret = sqr(input);

    *helper << "Got response, returning...";
    return ret;
}

int main(int argc, const char *argv[])
{
    try {
        cout << "100 (normal return)    :" << endl;
        SquareSmallNum(100);
        cout << "-1: (early return)     :" << endl;
        SquareSmallNum(-1);
        cout << "1000 (throws an error) :"  << endl;
        SquareSmallNum (1000);
    } catch( string &s) {
        cout << "Got an error: " << s << endl;;
    }
    return 0;
}
