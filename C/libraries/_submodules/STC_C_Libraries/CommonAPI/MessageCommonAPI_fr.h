#ifndef MESSAGECOMMONAPI_FR_H
#define MESSAGECOMMONAPI_FR_H

/**************************************************************/
/* M10000 series - Information messages                       */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
/* STC_Logging: messages related to logging */
#define M10001 "Le fichier de serveur personnalisé est introuvable. La journalisation se poursuivra avec le serveur spécifié par défaut."
#define M10002 "Journalisation ignorée. Le fichier de serveur personnalisé a host=none."

/**************************************************************/
/* M20000 serie - Warning messages                            */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'AVERTISSEMENT:' added at the beginning*/
/**************************************************************/
#define M20001 "La variable '%s' a été supprimée parce que son coefficient (ou la somme de ses coefficients) est plus petit que la plus petite valeur acceptable %g. La valeur %g dépend de la plateforme utilisée."
#define M20002 "La valeur de la constante a été remplacée par 0.0 parce que cette valeur est plus petite que la plus petite valeur acceptable %g. La valeur %g dépend de la plateforme utilisée."
#define M20003 "La variable '%s' a été supprimée parce que son coefficient est 0.0."
#define M20004 "Deux constantes ont été additionnées entre elles."
#define M20005 "Les coefficients de la variable '%s' ont été additionnés parce que cette variable a été spécifiée plus d'une fois."
#define M20006 "La variable '%s' a été supprimée parce que la somme de ses coefficients est 0.0."
/* STC_Logging: messages related to logging */
#define M20007 "Journalisation ignorée. Le fichier de serveur personnalisé est invalide."
#define M20008 "Journalisation ignorée. Le fichier de serveur personnalisé existe mais ne peut pas être ouvert."

/**************************************************************/
/* M30000 serie - Error messages                              */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERREUR:' added at the beginning       */
/**************************************************************/
#define M30001 "Impossible de converger."
#define M30002 "Il est impossible d'inverser une matrice singulière dans la fonction ludcmp()."
#define M30003 "Il y a trop d'erreurs. La compilation doit s'arrêter."
#define M30004 "Il n'y a pas de variables dans les règles."
#define M30005 "Une règle PASS ne peut pas avoir un opérateur !=."
#define M30006 "Une règle FAIL ne peut pas avoir un opérateur =."
#define M30007 "Cherchons '%s' mais avons trouvé '%s' à la place."
#define M30008 "Cherchons 'Coefficient' ou 'Variable' mais avons trouvé '%s' à la place."
#define M30009 "Nom de variable trop long!"
#define M30010 "%s a échoué."
/* STC_Logging: messages related to logging */
//#define M30011 "L'accès au journal n'a pas réussi. La procédure va se poursuivre."
//#define M30012 "L'accès au journal n'a pas réussi. La procédure va se poursuivre."
#define M30013 "L'accès au journal n'a pas réussi. La procédure va se poursuivre. (curl_no_init)"
#define M30014 "L'accès au journal n'a pas réussi. La procédure va se poursuivre.\n(curl_rc:%d, http_rc:%d, \n URL:%s)"
#define M30015 "L'accès au journal n'a pas réussi. La procédure va se poursuivre."

/**************************************************************/
/* M40000 serie - Statistics (report)                         */
/**************************************************************/

/**************************************************************/
/* M00000 serie - Miscellaneous                               */
/**************************************************************/
#define M00001 "Parseur des règles"
#define M00002 "Erreur près ou avant"
#define M00003 "Coefficient"
#define M00004 "Deux points"
#define M00005 "Fin"
#define M00006 "Erreur"
#define M00007 "Moins"
#define M00008 "Modificateur"
#define M00009 "Opérateur"
#define M00010 "Plus"
#define M00011 "Point virgule"
#define M00012 "Astérique"
#define M00013 "Variable"
#define M00014 "Caractère non reconnu"
#define M00015 "Règle d'égalité: '%d' rend les règles déterministes."
#define M00016 "Règle d'égalité: '%d' est une combinaison linéaire d'autres règles. Elle est redondante."
#define M00017 "Règle d'égalité: '%d' rend les règles incohérentes."
#define M00018 "Règle d'égalité cachée: '%d' rend les règles déterministes."
#define M00019 "Règle d'égalité cachée: '%d' rend les règles incohérentes."
#define M00020 "Règle d'inégalité: '%d' est une inégalité cachée."
#define M00021 "Règle d'inégalité: '%d' est une inégalité cachée redondante."
#define M00022 "Règle d'inégalité: '%d' est stricte, mais non restrictive."
#define M00023 "Règle d'inégalité: '%d' est entièrement en dehors de la région d'acceptation. Elle est redondante."
#define M00024 "Près ou avant"







#endif
