// cglob.cpp
// Plain C wrapper for glob-cpp

#include "glob.h"
#include "cglob.h"
#include <string>
#include <new>

// This code assumes glob::glob public APIs are exception-safe (no throw on invalid pattern)

glob_t glob_create(const char *pattern, int /*flags*/)
{
    if (pattern == nullptr) {
    	return nullptr;
	}
	
    // Non-throwing new returns nullptr on allocation failure
    return new (std::nothrow) glob::glob(pattern);
}

void glob_free(glob_t g)
{
    delete static_cast<glob::glob*>(g); // delete is null-safe
}

int glob_match(const glob_t g, const char *str)
{
    if (g == nullptr || str == nullptr) {
    	return -1;
	}
	
    auto *matcher = static_cast<glob::glob*>(g);
    return glob::glob_match(std::string(str), *matcher) ? 0 : 1;
}

int glob_match_pattern(const char *pattern, const char *str, int /*flags*/)
{
    if (pattern == nullptr || str == nullptr) {
    	return -1;
	}
	
    glob::glob g(pattern);
    return glob::glob_match(std::string(str), g) ? 0 : 1;
}
