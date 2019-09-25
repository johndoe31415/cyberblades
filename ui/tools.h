#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdint.h>
#include <sys/time.h>

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
double now(void);
void add_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds);
void get_timespec_now(struct timespec *timespec);
void get_abs_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
