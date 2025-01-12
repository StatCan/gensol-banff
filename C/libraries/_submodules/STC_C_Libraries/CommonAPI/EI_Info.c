/* contient toutes les fonctions donnant de l'information sur le systeme. */

#include <stdio.h>
#include <stdlib.h>

#include "EI_Common.h"
#include "EI_Info.h"
#include "EI_Message.h"

/* Maximum size of a variable name */
static int mVariableNameMaxSize = EIM_VARIABLE_NAME_DEFAULT_SIZE;

/* List of valid character for a variable name */
static char * mVariableNameCharacterSet = EIM_VARIABLE_NAME_CHARACTER_SET;

/* List of valid character for the first character of a variable name */
static char * mVariableNameFirstCharacterCharacterSet =
    EIM_VARIABLE_NAME_FIRST_CHARACTER_CHARACTER_SET;


/*********************************************************************
Get valid characters for the variable name
*********************************************************************/
char * EI_InfoGetVariableNameCharacterSet (void) {
    return mVariableNameCharacterSet;
}
/*********************************************************************
Get valid characters for the first character of a variable name
*********************************************************************/
char * EI_InfoGetVariableNameFirstCharacterCharacterSet (void) {
    return mVariableNameFirstCharacterCharacterSet;
}
/*********************************************************************
returns the maximun size of a variable name.
*********************************************************************/
int EI_InfoGetVariableNameMaxSize (void)
{
    return mVariableNameMaxSize;
}
/*********************************************************************
Set valid characters for variable name
Must be called before EI_*Parse() to take effect.
*********************************************************************/
void EI_InfoSetVariableNameCharacterSet (
    char * VariableNameCharacterSet)
{
    mVariableNameCharacterSet = VariableNameCharacterSet;
}
/*********************************************************************
Set valid characters for the first character of a variable name
Must be called before EI_*Parse() to take effect.
*********************************************************************/
void EI_InfoSetVariableNameFirstCharacterCharacterSet (
    char * VariableNameFirstCharacterCharacterSet)
{
    mVariableNameFirstCharacterCharacterSet =
        VariableNameFirstCharacterCharacterSet;
}
/*********************************************************************
Set the maximun size of a variable name.
Must be called very early to be usefull.
Must be called before EI_*Parse() to take effect.
*********************************************************************/
void EI_InfoSetVariableNameMaxSize (
    int VariableNameMaxSize)
{
    if (VariableNameMaxSize <= 0 ||
            VariableNameMaxSize > EIM_VARIABLE_NAME_MAX_SIZE) {
        EI_AddMessage ("", 4, "EI_InfoSetVariableNameMaxSize(): %d exceeds the maximum size (%d) for a variable name.\n",
            VariableNameMaxSize, EIM_VARIABLE_NAME_MAX_SIZE);
        /* je me permet de mettre un exit() ici parce que ceci ne peut */
        /* arriver que pendant le developpement. */
        /* En production ca va toujours marcher */
        exit (-1); /* exit right away! */
    }

    mVariableNameMaxSize = VariableNameMaxSize;
}
