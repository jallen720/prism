#include <cstdlib>
#include <cstdio>
#include "prism/utilities.h"

namespace prism
{

void util_error_exit(const char * message, const char * subsystem)
{
    const char * system = subsystem ? subsystem : "PRISM";
    fprintf(stderr, "%s ERROR: %s\n", system, message);
    exit(EXIT_FAILURE);
}

} // namespace prism
