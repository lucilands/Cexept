# Cexept - Exceptions in C 

If you for some reason need exceptions, try catch statements etc. in C, here you go

## Usage

This is a basic example that throws an error and exits
```C
#define CEXEPT_IMPLEMENTATION // We want the functions in cexept.h to be defined
#include <cexept.h>


#define INDEX_EXCEPTION 1 // We define our index exception with the ID of 1

void some_function() {
    CEXEPT_THROW(INDEX_EXCEPTION); // Throw our exception
}

int main() {
    CEXEPT_ERROR_DESCRIPTION(INDEX_EXCEPTION, "Index out of range"); // Define the error message in our exception
    some_function(); // Call the function that causes the error
    return 0;
}
```
Compile with 
```bash
gcc example.c -rdynamic -o example -ldl
```

However, if we expect this exception to be thrown or just want to stop the program from exiting if it does we can use a try-catch construct.
```C
...

void some_function() {
    CEXEPT_THROW(INDEX_EXCEPTION); // Throw our exception
}

int main() {
    ...

    CEXEPT_TRY(
        some_function(); // Call the function that causes the error, this time its wrapped in CEXEPT_TRY
    )
    CEXEPT_CATCH(error, // Variable name for the error that got caught
        printf("Error caught: %s (%i)\n", error.descr, error.code); // Print out a message instead of exiting
    )
    
    return 0;
}
```
Same compile command here
```bash
gcc example.c -rdynamic -o example -ldl
```


# Limitations

Since this is not a compiler hack, its just a lot of macro magick there are a couple limitations, for example there may only be *CEXEPT_MAX_EXCEPTIONS* unique exceptions. *CEXEPT_MAX_EXCEPTIONS* defaults to 255, but it may be redefined before including to increase or decrease it.
Because of this any exception ID should be between 0 and *CEXEPT_MAX_EXCEPTIONS* aswell.
This is because the library internally stores the error messages in an array with the size *CEXEPT_MAX_EXCEPTIONS* and uses it as a lookup table for the error messages.

To get function names in the stacktrace the binary has to be compiled with `-rdynamic`.

# Features
 - C99 compatable
 - Stacktrace
 - Try, Catch
 - Custom Exceptions

# TODO
- [x] Custom error descriptions
- [x] Better stacktraces
- [ ] Being cross-compatable
  - [x] Linux GCC
  - [ ] Windows MinGW
  - [ ] Windows MSVC
  - [ ] MacOS (Untested)
- [x] Memory safe
- [x] Proper try catch

etc...
