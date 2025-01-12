#ifndef EI_CHECK_H
#define EI_CHECK_H

#include "EI_Common.h"
#include "EI_Edits.h"


/* Return codes for EI_Check*/
typedef enum {
    EIE_CHECK_FAIL     = EIE_FAIL,
    EIE_CHECK_SUCCEED  = EIE_SUCCEED
} EIT_CHECK_RETURNCODE;	  



CLASS_DECLSPEC EIT_CHECK_RETURNCODE EI_Check (EIT_EDITS *, EIT_PRINTF);

#endif
