#ifndef EI_DATAREC_H
#define EI_DATAREC_H

#include "EI_Common.h"

CLASS_DECLSPEC EIT_RETURNCODE EI_DataRecAllocate (char **, int, EIT_DATAREC *);
CLASS_DECLSPEC void EI_DataRecFree (EIT_DATAREC *);
CLASS_DECLSPEC void EI_DataRecPrint (EIT_DATAREC *);
CLASS_DECLSPEC int EI_DataRecSearch (EIT_DATAREC *, char *);
CLASS_DECLSPEC void EI_DataRecSetFlag (EIT_DATAREC *);
CLASS_DECLSPEC void EI_DataRecSetFlagForDebug (EIT_DATAREC *);
CLASS_DECLSPEC void EI_DataRecSetMissingFlag (EIT_DATAREC *);

#endif
