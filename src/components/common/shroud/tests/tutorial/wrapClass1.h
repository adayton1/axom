// wrapClass1.h
// This is generated code, do not edit
// wrapClass1.h
// For C users and C++ implementation

#ifndef WRAPCLASS1_H
#define WRAPCLASS1_H

#ifdef __cplusplus
extern "C" {
#endif

// declaration of wrapped types
#ifdef EXAMPLE_WRAPPER_IMPL
typedef void TUT_class1;
#else
struct s_TUT_class1;
typedef struct s_TUT_class1 TUT_class1;
#endif

// splicer begin class.Class1.C_definition
// splicer end class.Class1.C_definition

TUT_class1 * TUT_class1_new();

void TUT_class1_method1(TUT_class1 * self);

#ifdef __cplusplus
}
#endif

#endif  // WRAPCLASS1_H
