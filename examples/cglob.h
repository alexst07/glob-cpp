/* cglob.h */
#ifndef CGLOB_H
#define CGLOB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Opaque handle representing a compiled glob pattern.
 */
typedef void* glob_t;

/* Flags are reserved for future use (currently ignored) */
#define GLOB_NO_FLAGS 0

/**
 * Compile a glob pattern.
 *
 * @param pattern  Null-terminated UTF-8 glob pattern (e.g. '*.txt').
 * @param flags    Reserved for future use; pass GLOB_NO_FLAGS.
 *
 * @return Pointer to compiled glob object on success, NULL on failure
 *         (invalid pattern, out of memory, etc.).
 */
glob_t glob_create(const char *pattern, int flags);

/**
 * Free a glob object previously created with glob_create().
 *
 * @param g  Handle returned by glob_create(), or NULL (no-op if NULL).
 */
void glob_free(glob_t g);

/**
 * Test whether a string matches a compiled glob pattern.
 *
 * @param g    Handle returned by glob_create() (must not be NULL).
 * @param str  Null-terminated UTF-8 string to test.
 *
 * @return 0  = match
 *         1  = no match
 *        -1  = error (NULL input or invalid handle)
 */
int glob_match(const glob_t g, const char *str);

/**
 * One-shot pattern compilation and matching (convenience function).
 *
 * Equivalent to glob_create() + glob_match() + glob_free().
 *
 * @param pattern  Null-terminated UTF-8 glob pattern.
 * @param str      Null-terminated UTF-8 string to test.
 * @param flags    Reserved for future use; pass GLOB_NO_FLAGS.
 *
 * @return Same return codes as glob_match().
 */
int glob_match_pattern(const char *pattern, const char *str, int flags);

#ifdef __cplusplus
}
#endif

#endif /* CGLOB_H */
