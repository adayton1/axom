//
// tutorial.hpp - wrapped routines
//

#include "tutorial.hpp"

static std::string last_function_called;

// These variables exist to avoid warning errors
static std::string global_str;
static int global_int;
static double global_double;




void Function1()
{
    last_function_called = "Function1";
    return;
}

double Function2(double arg1, int arg2)
{
    last_function_called = "Function2";
    return arg1 + arg2;
}

bool Function3(bool arg)
{
    last_function_called = "Function3";
    return ! arg;
}

const std::string& Function4a(const std::string& arg1, const std::string& arg2)
{
    last_function_called = "Function4a";
    global_str = arg1 + arg2;
    return global_str;
}
const std::string& Function4b(const std::string& arg1, const std::string& arg2)
{
    last_function_called = "Function4b";
    global_str = arg1 + arg2;
    return global_str;
}

double Function5(double arg1, int arg2)
{
    last_function_called = "Function5";
    return arg1 + arg2;
}

void Function6(const std::string& name)
{
    last_function_called = "Function6(string)";
    global_str = name;
    return;
}
void Function6(int indx)
{
    last_function_called = "Function6(int)";
    global_int = indx;
    return;
}

template<>
void Function7<int>(int arg)
{
    last_function_called = "Function7<int>";
    global_int = arg;
}

template<>
void Function7<double>(double arg)
{
    last_function_called = "Function7<double>";
    global_double = arg;
}

template<>
int Function8<int>()
{
    last_function_called = "Function8<int>";
    return global_int;
}

template<>
double Function8<double>()
{
    last_function_called = "Function8<double>";
    return global_double;
}

void Function9(double arg)
{
    last_function_called = "Function9";
    global_double = arg;
    return;
}


int Sum(int len, int *values)
{
    last_function_called = "Sum";

    int sum;
    for (int i=0; i < len; i++) {
	sum += values[i];
    }
    return sum;
}

//----------------------------------------------------------------------

void Class1::Method1()
{
    last_function_called = "Class1::Method1";
    return;
}

//----------------------------------------------------------------------

const std::string& LastFunctionCalled()
{
    return last_function_called;
}

