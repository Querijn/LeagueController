#pragma once

#if NDEBUG
#define EXPOSE_FILE "CODE"
#else
#define EXPOSE_FILE __FILE__
#endif

#define SINGLE_ARG(...) __VA_ARGS__

#if _DEBUG
#define IF_DEBUG(...) __VA_ARGS__
#define IF_DEBUG_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_DEBUG_ELSE(a, b) SINGLE_ARG(b)
#define IF_NOT_DEBUG(a)

#else
#define IF_DEBUG(a) 
#define IF_DEBUG_ELSE(a, ...) __VA_ARGS__
#define IF_NOT_DEBUG_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_DEBUG(...) __VA_ARGS__

#endif

#if LEACON_SUBMISSION
#define IF_SUBMISSION(...) __VA_ARGS__
#define IF_SUBMISSION_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_SUBMISSION_ELSE(a, b) SINGLE_ARG(b)
#define IF_NOT_SUBMISSION(a)

#else
#define IF_SUBMISSION(a) 
#define IF_SUBMISSION_ELSE(a, ...) __VA_ARGS__
#define IF_NOT_SUBMISSION_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_SUBMISSION(...) __VA_ARGS__

#endif

#if NDEBUG && LEACON_SUBMISSION
#define LEACON_FINAL 1

#define IF_FINAL(...) __VA_ARGS__
#define IF_FINAL_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_FINAL_ELSE(a, b) SINGLE_ARG(b)
#define IF_NOT_FINAL(a)

#else
#define LEACON_FINAL 0

#define IF_FINAL(a) 
#define IF_FINAL_ELSE(a, ...) __VA_ARGS__
#define IF_NOT_FINAL_ELSE(a, b) SINGLE_ARG(a)
#define IF_NOT_FINAL(...) __VA_ARGS__

#endif