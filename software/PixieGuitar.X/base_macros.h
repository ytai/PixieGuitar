#ifndef BASE_MACROS_H
#define	BASE_MACROS_H


#define WARN_UNUSED __attribute__((warn_unused_result))

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#endif	/* BASE_MACROS_H */

