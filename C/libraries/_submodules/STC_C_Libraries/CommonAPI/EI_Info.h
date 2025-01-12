#ifndef EI_INFO_H
#define EI_INFO_H

CLASS_DECLSPEC char * EI_InfoGetVariableNameCharacterSet (void);
CLASS_DECLSPEC char * EI_InfoGetVariableNameFirstCharacterCharacterSet (void);
CLASS_DECLSPEC int EI_InfoGetVariableNameMaxSize (void);
CLASS_DECLSPEC void EI_InfoSetVariableNameCharacterSet (char *);
CLASS_DECLSPEC void EI_InfoSetVariableNameFirstCharacterCharacterSet (char *);
CLASS_DECLSPEC void EI_InfoSetVariableNameMaxSize (int);

#endif
