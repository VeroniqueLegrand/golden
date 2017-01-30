/* error.h - Error functions */

#ifndef __ERROR_H_
#define __ERROR_H_

#define SUCCESS_ALL_FOUND 0
#define SUCCESSS_NOT_ALL_FOUND 1
#define FAILURE 2

/* Functions prototypes */
void error_fatal(const char *, const char *);  // TODO : remove these functions and use those in err.h instead.
void error_warn(const char *, const char *);
#endif /* __ERROR_H_ */
