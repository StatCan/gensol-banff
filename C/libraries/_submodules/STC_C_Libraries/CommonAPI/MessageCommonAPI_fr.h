#ifndef MESSAGECOMMONAPI_FR_H
#define MESSAGECOMMONAPI_FR_H

/**************************************************************/
/* M10000 series - Information messages                       */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
/* STC_Logging: messages related to logging */
#define M10001 "Le fichier de serveur personnalis� est introuvable. La journalisation se poursuivra avec le serveur sp�cifi� par d�faut."
#define M10002 "Journalisation ignor�e. Le fichier de serveur personnalis� a host=none."

/**************************************************************/
/* M20000 serie - Warning messages                            */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'AVERTISSEMENT:' added at the beginning*/
/**************************************************************/
#define M20001 "La variable '%s' a �t� supprim�e parce que son coefficient (ou la somme de ses coefficients) est plus petit que la plus petite valeur acceptable %g. La valeur %g d�pend de la plateforme utilis�e."
#define M20002 "La valeur de la constante a �t� remplac�e par 0.0 parce que cette valeur est plus petite que la plus petite valeur acceptable %g. La valeur %g d�pend de la plateforme utilis�e."
#define M20003 "La variable '%s' a �t� supprim�e parce que son coefficient est 0.0."
#define M20004 "Deux constantes ont �t� additionn�es entre elles."
#define M20005 "Les coefficients de la variable '%s' ont �t� additionn�s parce que cette variable a �t� sp�cifi�e plus d'une fois."
#define M20006 "La variable '%s' a �t� supprim�e parce que la somme de ses coefficients est 0.0."
/* STC_Logging: messages related to logging */
#define M20007 "Journalisation ignor�e. Le fichier de serveur personnalis� est invalide."
#define M20008 "Journalisation ignor�e. Le fichier de serveur personnalis� existe mais ne peut pas �tre ouvert."

/**************************************************************/
/* M30000 serie - Error messages                              */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERREUR:' added at the beginning       */
/**************************************************************/
#define M30001 "Impossible de converger."
#define M30002 "Il est impossible d'inverser une matrice singuli�re dans la fonction ludcmp()."
#define M30003 "Il y a trop d'erreurs. La compilation doit s'arr�ter."
#define M30004 "Il n'y a pas de variables dans les r�gles."
#define M30005 "Une r�gle PASS ne peut pas avoir un op�rateur !=."
#define M30006 "Une r�gle FAIL ne peut pas avoir un op�rateur =."
#define M30007 "Cherchons '%s' mais avons trouv� '%s' � la place."
#define M30008 "Cherchons 'Coefficient' ou 'Variable' mais avons trouv� '%s' � la place."
#define M30009 "Nom de variable trop long!"
#define M30010 "%s a �chou�."
/* STC_Logging: messages related to logging */
//#define M30011 "L'acc�s au journal n'a pas r�ussi. La proc�dure va se poursuivre."
//#define M30012 "L'acc�s au journal n'a pas r�ussi. La proc�dure va se poursuivre."
#define M30013 "L'acc�s au journal n'a pas r�ussi. La proc�dure va se poursuivre. (curl_no_init)"
#define M30014 "L'acc�s au journal n'a pas r�ussi. La proc�dure va se poursuivre.\n(curl_rc:%d, http_rc:%d, \n URL:%s)"
#define M30015 "L'acc�s au journal n'a pas r�ussi. La proc�dure va se poursuivre."

/**************************************************************/
/* M40000 serie - Statistics (report)                         */
/**************************************************************/

/**************************************************************/
/* M00000 serie - Miscellaneous                               */
/**************************************************************/
#define M00001 "Parseur des r�gles"
#define M00002 "Erreur pr�s ou avant"
#define M00003 "Coefficient"
#define M00004 "Deux points"
#define M00005 "Fin"
#define M00006 "Erreur"
#define M00007 "Moins"
#define M00008 "Modificateur"
#define M00009 "Op�rateur"
#define M00010 "Plus"
#define M00011 "Point virgule"
#define M00012 "Ast�rique"
#define M00013 "Variable"
#define M00014 "Caract�re non reconnu"
#define M00015 "R�gle d'�galit�: '%d' rend les r�gles d�terministes."
#define M00016 "R�gle d'�galit�: '%d' est une combinaison lin�aire d'autres r�gles. Elle est redondante."
#define M00017 "R�gle d'�galit�: '%d' rend les r�gles incoh�rentes."
#define M00018 "R�gle d'�galit� cach�e: '%d' rend les r�gles d�terministes."
#define M00019 "R�gle d'�galit� cach�e: '%d' rend les r�gles incoh�rentes."
#define M00020 "R�gle d'in�galit�: '%d' est une in�galit� cach�e."
#define M00021 "R�gle d'in�galit�: '%d' est une in�galit� cach�e redondante."
#define M00022 "R�gle d'in�galit�: '%d' est stricte, mais non restrictive."
#define M00023 "R�gle d'in�galit�: '%d' est enti�rement en dehors de la r�gion d'acceptation. Elle est redondante."
#define M00024 "Pr�s ou avant"







#endif
