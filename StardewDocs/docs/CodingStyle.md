## Coding Style Aesthetics

- opening curly brackets go on a new line
- types are in PascalCase
- local variables and parameters are in camelCase
    - if a variable is a pointer, name it like so:
        ```
        pMyVar
        ```
    - if a variable is a bool, name it like so:
        ```
        bMyVar
        ```
    - if a variable is global, name it like so:
        ```
        gMyVar
        ```
    - if a variable is "local" but static name it like so:
        ```
        sMyVar
        ```
    - if the variable is a handle, name it like so:
        ```
        hMyVar
        ```
    - if it fits into more than one of these categories, favour the g and s prefixes
- use the C style #ifndef include guard not #pragma once

## Coding Style

- add a doxygen comment to all newly created public functions
- add unit tests for anything that's likely to be heavily reused, or anything that just feels complicated or brittle
- don't use macros unless absolutely necessary (and they're never **really** necessary) (in some places I've violated this)
- include the minimum amount of headers, especially in other headers
- don't typedef structs (in some places I have done). I don't like this because it makes them harder to forward declare.
- always typedef function pointers (in some places I haven't done). The function pointer syntax should be kept hidden away, isolated to one place.
- C code has to compile with both msvc and gcc (I've not found this to be an issue really)
    - constantly check CI pipelines that both builds are succeeding and tests passing
- prefer to use python for any tools and build / testing scripts
    - any python scripts should use argparse to parse command line args and should provide help strings for different flags
- Try to make the github CI script simply call scripts that also work in a local environment
    - the only shell script in the yml files should be super trivial, just calling other scripts
    - no github environment variables used in any scripts, they should all work in a local environment
    - try to not use any external python libraries
