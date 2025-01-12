/* ------------------------------------------------------------------ */
/*                                                                    */
/* NAME            EI_UserData                                        */
/*                                                                    */
/* DESCRIPTION     This function will set and get UserData            */
/*                 for availability to CALLBACK read and write        */
/*                 functions.                                         */
/*                                                                    */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */

#include "EI_UserData.h"

static void * mUserData;

void * EI_GetUserData (void) {
    return mUserData;
}

void EI_SetUserData (void * UserData) {
    mUserData = UserData;
}
