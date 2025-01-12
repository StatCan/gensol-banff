#include "verifyed_JIO.h"


IO_RETURN_CODE SP_init(SP_verifyed* sp, T_in_parm in_parms) {
    TIME_WALL_START(SP_init);
    TIME_CPU_START(SP_init);

    /* INITIALIZE SP_generic struct */
    IO_RETURN_CODE rc_io = SPG_init(&sp->spg, in_parms, UPO__COUNT, DSIO__COUNT, DSOO__COUNT);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* GET PARAMETERS */
    TIME_CPU_START(read_parms);
    /****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] ***************************/
    PARM_init(&sp->spg, UPO_ACCEPT_NEGATIVE,     (PARM_generic*) &sp->accept_negative,    BPN_ACCEPT_NEGATIVE,        IOPT_FLAG);
    PARM_init(&sp->spg, UPO_EXTREMAL,            (PARM_generic*) &sp->extremal,           BPN_EXTREMAL,               IOPT_INT);
    PARM_init(&sp->spg, UPO_IMPLY,               (PARM_generic*) &sp->imply,              BPN_IMPLY,                  IOPT_INT);
    PARM_init(&sp->spg, UPO_EDITS,               (PARM_generic*) &sp->edits,              BPN_EDITS,                  IOPT_STRING);
    /****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
    TIME_CPU_STOPDIFF(read_parms);

    /* stop timers: all expensive operations now complete */
    TIME_CPU_STOPDIFF(SP_init);
    TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}

/* SP_wrap
    For verifyedits, there is nothing to "wrap"
    This function is defined for consistency, and future use
    */
PROC_RETURN_CODE SP_wrap(SP_verifyed* sp) {

    TIME_WALL_START(SP_wrap);
    TIME_CPU_START(SP_wrap);
    // add wrap-up code here
    TIME_CPU_STOPDIFF(SP_wrap);
    TIME_WALL_STOPDIFF(SP_wrap);

    return PRC_SUCCESS;
}
