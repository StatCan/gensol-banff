/*
EIP_Common.h

Define private macro, type and enums needed in all of EI
but that we do not won't to publish in EI_Common.h
*/

#ifndef EIP_COMMON_H
#define EIP_COMMON_H

#include <float.h>
#include <math.h>

#define EIM_DBL_EPSILON      ((double)FLT_EPSILON) /* test this value */
#define EIM_DBL_EQ(a,b)      (fabs((a)-(b)) < EIM_DBL_EPSILON)
#define EIM_DBL_LT(a,b)      (((b)-(a)) >= EIM_DBL_EPSILON)
#define EIM_DBL_GT(a,b)      (((a)-(b)) >= EIM_DBL_EPSILON)
#define EIM_DBL_LE(a,b)      (EIM_DBL_LT((a),(b)) || EIM_DBL_EQ((a),(b)))
#define EIM_DBL_GE(a,b)      (EIM_DBL_GT((a),(b)) || EIM_DBL_EQ((a),(b)))
#define EIM_DBL_MAX(a,b)     (EIM_DBL_LT((a),(b))?(a):(b))
#define EIM_DBL_MIN(a,b)     (EIM_DBL_LT((a),(b))?(b):(a))

#define EIM_MIN(a,b)         ((a)<(b)?(a):(b))
#define EIM_MAX(a,b)         ((a)<(b)?(b):(a))


/* Positivity options that can be set by users of procedures */
typedef enum {
    EIPE_REJECT_NEGATIVE_DEFAULT = 1,
    EIPE_REJECT_NEGATIVE = 2,
    EIPE_ACCEPT_NEGATIVE = 3,
    EIPE_ACCEPT_AND_REJECT_NEGATIVE = 4
} EIPT_POSITIVITY_OPTION;


#endif
