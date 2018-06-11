#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include "prism/utilities.h"

namespace prism
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_PURPLE "\x1b[35m"

#define OUTPUT_MESSAGE(OUTPUT) \
    va_list args; \
    va_start(args, message); \
    vfprintf(OUTPUT, message, args); \
    va_end(args);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
utilErrorExit(const char * subsystem, const char * errorName, const char * message, ...)
{
    const char * system = subsystem != nullptr ? subsystem : "PRISM";

    if(errorName)
    {
        fprintf(stderr, ANSI_BOLD ANSI_COLOR_RED "%s ERROR -> %s" ANSI_RESET ": ", system, errorName);
    }
    else
    {
        fprintf(stderr, ANSI_BOLD ANSI_COLOR_RED "%s ERROR" ANSI_RESET ": ", system);
    }

    OUTPUT_MESSAGE(stderr)
    exit(EXIT_FAILURE);
}

void
utilLog(const char * subsystem, const char * message, ...)
{
    fprintf(stdout, ANSI_BOLD ANSI_COLOR_GREEN "%s LOG" ANSI_RESET ": ", subsystem ? subsystem : "PRISM");
    OUTPUT_MESSAGE(stdout)
}

void
utilWarning(const char * subsystem, const char * message, ...)
{
    fprintf(stdout, ANSI_BOLD ANSI_COLOR_PURPLE "%s WARNING" ANSI_RESET ": ", subsystem ? subsystem : "PRISM");
    OUTPUT_MESSAGE(stdout)
}

} // namespace prism
