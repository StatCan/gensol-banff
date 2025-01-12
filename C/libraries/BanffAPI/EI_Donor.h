#ifndef EI_DONOR_H
#define EI_DONOR_H

#include <time.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "slist.h"
#include "adtlist.h"

#define FIELD_IMPUTE 1 /* Field to impute */
#define TERMNODE        -1        /* indicate terminal node */
#define ENDLIST         -1        /* indicate end of list */
#define ISFOUND          0
#define ISNOTFOUND      -1
#define BRANCH    1
#define NOBRANCH  0
#define OVERLAP   1
#define NOOVERLAP 0
#define DONOR_RANDNUM_FUZZ (double) 1.0e-12 /* 1.0-(maximum allowed value for random number values) */

/*
EIT_DONOR_STATUSFLAG - Status of donor edits fields/Statuts des champs des équations d'un donneur
This structure holds the status of the donor edits fields.

Cette structure contient les statuts des champs des équations d'un donneur.
*/
typedef struct {
        /*Status of the donor edits fields*/
        /*Statuts des champs des équations d'un donneur*/
    EIT_STATUS_OF_FIELDS *StatusFlag;
        /*Number of edits fields/Nombre de champs des équations*/
    int NumberofFields;
} EIT_DONOR_STATUSFLAG;

/*
EIT_ALL_DONORS - Transformed values of donor matching fields/Valeurs transformées des champs d'appariement des donneurs
This structure holds the transformed values of the donor matching fields
and the status of edits fields for these donors.

Cette structure contient les valeurs transformées des champs
d'appariement des donneurs et les statuts des champs des équations
pour ces donneurs.
*/
typedef struct EIT_ALL_DONORS {
        /*Donors key value/Valeur de la clé des donneurs*/
    char   **Key;
        /*Transformed values of donor matching fields*/
        /*Valeurs transformées des champs d'appariement des donneurs*/
    double **Value;
		/*Number of times the donor was used*/
		/*Nombre de fois ou le donneur a été sélectionné*/
	int * Gave;
        /*Status of edits fields for the donors*/
        /*Statuts des champs des équations pour les donneurs*/
    EIT_DONOR_STATUSFLAG * DonorStatusFlag;
        /*Number of donors/Nombre de donneurs*/
    int    NumberOfDonors;
        /*Number of fields/Nombre de champs*/
    int     NumberOfFields;
} EIT_ALL_DONORS;

/* EIT_KDTREE - Kd tree for search of donors/Arbre Kd pour recherche de donneurs*/
/* This structure holds the donors organized in a k-dimensional tree structure*/
/* where k is the number of matching fields.*/
/**/
/* Cette structure organise les donneurs dans un arbre à k dimensions où k*/
/* représente le nombre de champs d'appariement.*/
/**/
/* Structure variables/Variables de la structure:*/
/* int start;                The lowest index in DONOR AllDonors of the*/
/*                           donors belonging to the node.*/
/**/
/*                           /L'indice le plus bas dans DONOR AllDonors des*/
/*                           donneurs appartenant au noeud.*/
/**/
/* int end;                  The highest index in DONOR AllDonors of the*/
/*                           donors belonging to the node.*/
/**/
/*                           /L'indice le plus élevé dans DONOR AllDonors des*/
/*                           donneurs appartenant au noeud.*/
/**/
/* struct NODEDEF *leftson;  The child node defining the left branch of the*/
/*                           subtree defined by the current node. The branchid*/
/*                           and the splitval of the current node are used*/
/*                           to determine which donors of the node belong*/
/*                           to the leftson.*/
/**/
/*                           /Le noeud successeur représentant la branche de*/
/*                           gauche du sous-arbre du noeud courant.  Les*/
/*                           variables branchid et splitval du noeud courant*/
/*                           servent à déterminer quels donneurs du noeud*/
/*                           appartiennent au successeur de gauche.*/
/**/
/* struct NODEDEF *rightson; The child node defining the right branch of the*/
/*                           subtree defined by the current node.  The*/
/*                           branchid and the splitval of the current node*/
/*                           are used to determine which donors of the node*/
/*                           belong to the rightson.*/
/**/
/*                           /Le noeud successeur représentant la branche de*/
/*                           droite du sous-arbre du noeud courant.  Les*/
/*                           variables branchid et splitval du noeud courant*/
/*                           servent à déterminer quels donneurs du noeud*/
/*                           appartiennent au successeur de droite.*/
/**/
/* int branchid;             The index of the variable with the largest*/
/*                           range of values among the donors of the current*/
/*                           node.  In terminal nodes, the value is -1.*/
/**/
/*                           /L'indice de la variable dont l'étendue des*/
/*                           valeurs est la plus grande parmi les donneurs*/
/*                           du noeud courant.  Dans un noeud terminal,*/
/*                           la valeur est -1.*/
/**/
/* double splitval;          The median of the values of the variable*/
/*                           represented by branchid.  All donors with*/
/*                           values <= splitval will belong to the left*/
/*                           branch and all others will belong to the*/
/*                           right branch.  In terminal nodes, there is */
/*                           no splitting value.*/
/**/
/*                           /La médiane des valeurs de la variable*/
/*                           représentée par branchid.  Les donneurs dont*/
/*                           la valeur est <= splitval iront dans la branche*/
/*                           de gauche et les autres iront dans la branche*/
/*                           de droite.  Dans un noeud terminal, il*/
/*                           n'y a pas de splitval.*/
typedef struct NODEDEF {
    int start;
    int end;
    struct NODEDEF *leftson;
    struct NODEDEF *rightson;
    int branchid;
    double splitval;
} EIT_KDTREE;

/* EIT_MATCHVAL - Transformed values for one matching field/Valeurs Transformées d'un champ d'appariement*/
/* This structure holds the transformed values for one*/
/* matching field at a time.*/
/**/
/* Cette structure contient les valeurs transformées d'un*/
/* champ d'appariement à la fois.*/
/**/
/* Structure variables / Variables de la structure:*/
/* char **Key;                   Key value for each field value*/
/*                               Valeur de la clé pour chaque valeur du champ*/
/**/
/* double * Value;               Matching field transformed values*/
/*                               /Valeurs transformées des champs d'appariement*/
/**/
/* long NumberofAllocatedValues; Number of values up to a certain point*/
/*                               /Nombre de valeurs allouées jusqu'à maintenant*/
/**/
/* long NumberofValues;          Total number of values*/
/*                               /Nombre total de valeurs*/
typedef struct {
    char **Key;
    double * Value;
    long NumberofAllocatedValues;
    long NumberofValues;
} EIT_MATCHVAL;

/* EIT_NEARESTDONORS - Nearest donors/Plus proches donneurs*/
/* This structure holds the information on nearest donors found in the*/
/* k-d tree for one recipient.*/
/**/
/* Cette structure contient l'information sur les plus proches donneurs*/
/* trouvés dans l'arbre k-d pour un receveur.*/
/**/
/* Structure variables/Variables de la structure:*/
/* int *DonorsIndex;   Donors index in EIT_ALL_DONORS for nearest donors found*/
/**/
/*                     Index des donneurs dans EIT_ALL_DONORS pour les plus*/
/*                     proches donneurs trouvés*/
/**/
/* double *Distances;  Distances between nearest donors and one recipient*/
/**/
/*                     Distances entre les plus proches donneurs et un receveur*/
/**/
/* int NumberofDonors; Number of donors/Nombre de donneurs*/
typedef struct {
    int * DonorsIndex;
    double * Distances;
    int NumberofDonors;
} EIT_NEARESTDONORS;

/* EIT_TRANSFORMED - Values of all matching fields/Valeurs de tous les champs d'appariement*/
/* This structure holds the information for all */
/* matching fields.*/
/**/
/* Cette structure contient l'information pour tous */
/* les champs d'appariement.*/
/**/
/* Structure variables / Variables de la structure:*/
/* char ** FieldName;               Names of matching fields*/
/*                                  /Noms des champs d'appariement*/
/**/
/* int NumberofFields:              Number of matching fields*/
/*                                  /Nombre de champs d'appariement*/
/**/
/* EIT_MATCHVAL * TransformedValues; Pointer to matching field structure*/
/*                                  /Pointeur à la structure d'un*/
/*                                  champ d'appariement*/
typedef struct {
    char ** FieldName;
    int NumberofFields;
    EIT_MATCHVAL * TransformedValues;
} EIT_TRANSFORMED;

/* EIT_TREESEARCH - specs for tree search/spécifications pour recherche dans l'arbre*/
/* This structure holds the information needed to search the k-d tree.*/
/**/
/* Cette structure contient l'information nécessaire à la recherche dans*/
/* l'arbre k-d.*/
/**/
/* Structure variables/Variables de la structure:*/
/*                    physical storage for p_list, d_list, m_list */
/**/
/*                    mémoire physique de d_list et m_list */
/**/
/*int    *d_list;     nearest donors list (grows upwards --> )    */
/**/
/*                    liste des plus proches donneurs (augmente)  */
/**/
/*int    *m_list;     max distance donors list (grows down <-- )  */
/**/
/*                    distances maximum de la liste de donneurs (diminue)*/
/**/
/*int    *next_dl;    slot for next donor in d_list               */
/**/
/*                    emplacement du prochain donneur dans d_list*/
/**/
/*int    *next_ml;    slot for next donor in m_list               */
/**/
/*                    emplacement du prochain donneur dans m_list*/
/**/
/*double *distances;  distances of corresponding donors in d_list */
/**/
/*                    distance des donneurs correspondants dans d_list*/
/**/
/*double *next_dist;  slot for distance of next donor in d_list   */
/**/
/*                    emplacement pour la distance du prochain donneur */
/*                    dans d_list*/
/**/
/*double  maxdist;    distance of all donors in m_list            */
/**/
/*                    distance de tous les donneurs dans m_list*/
/**/
/*int     dl_size;    number of donors in d_list/nombre de donneurs dans d_list*/
/**/
/*int     ml_size;    number of donors in m_list/nombre de donneurs dans m_list*/
/**/
/*int     needed;     number of donors to find/nombre de donneurs à trouver*/
typedef struct {
    int    *d_list;
    int    *m_list;
    int    *next_dl;
    int    *next_ml;
    double *distances;
    double *next_dist;
    double  maxdist;
    int     dl_size;
    int     ml_size;
    int     needed;
} EIT_TREESEARCH;

/* Return codes for the EI_AllocateRecipients function*/
/**/
/*Codes de retour pour la fonction EI_AllocateRecipients */
typedef enum {
    EIE_ALLOCATERECIPIENTS_FAIL=EIE_FAIL,
    EIE_ALLOCATERECIPIENTS_SUCCEED=EIE_SUCCEED
} EIT_ALLOCATERECIPIENTS_RETURNCODE;

/* Return codes for the EI_AllocateRespondents function*/
/**/
/*Codes de retour pour la fonction EI_AllocateRespondents */
typedef enum {
    EIE_ALLOCATERESPONDENTS_FAIL=EIE_FAIL,
    EIE_ALLOCATERESPONDENTS_SUCCEED=EIE_SUCCEED
} EIT_ALLOCATERESPONDENTS_RETURNCODE;

/* Return codes for the EI_FindNearestDonors function*/
/**/
/* Codes de retour pour la fonction EI_FindNearestDonors*/
typedef enum {
    EIE_FINDNEARESTDONORS_FAIL,
    EIE_FINDNEARESTDONORS_SUCCEED,
    EIE_FINDNEARESTDONORS_NOTFOUND
} EIT_FINDNEARESTDONORS_RETURNCODE;

/* Return codes for the EI_BuildKDTree function*/
/**/
/* Codes de retour pour la fonction EI_BuildKDTree*/
typedef enum {
    EIE_KDTREE_FAIL=EIE_FAIL,
    EIE_KDTREE_SUCCEED=EIE_SUCCEED
} EIT_KDTREE_RETURNCODE;

/* Return codes for the EI_FindMatchingFields function*/
/**/
/* Codes de retour pour la fonction EI_FindMatchingFields*/
typedef enum {
    EIE_MATCHFIELDS_FAIL=EIE_FAIL,
    EIE_MATCHFIELDS_SUCCEED=EIE_SUCCEED
} EIT_MF_RETURNCODE;

/* Return codes for the EI_RandomDonor function*/
/**/
/* Codes de retour pour la fonction EI_RandomDonor */
typedef enum {
    EIE_RANDOMDONOR_FAIL,
    EIE_RANDOMDONOR_SUCCEED,
    EIE_RANDOMDONOR_NOTFOUND
} EIT_RANDOMDONOR_RETURNCODE;

/* Return codes for the EI_OneRecipient_AllocateFTIFields function*/
/**/
/*Codes de retour pour la fonction EI_OneRecipient_AllocateFTIFields */
typedef enum {
    EIE_ONERECIPIENTALLOCATE_FAIL=EIE_FAIL,
    EIE_ONERECIPIENTALLOCATE_SUCCEED=EIE_SUCCEED
} EIT_ONERECIPIENTALLOCATE_RETURNCODE;

/* Return codes for the EI_Transform function*/
/**/
/* Codes de retour pour la fonction EI_Transform*/
typedef enum {
    EIE_TRANSFORM_FAIL=EIE_FAIL,
    EIE_TRANSFORM_SUCCEED=EIE_SUCCEED
} EIT_TRF_RETURNCODE;

/*
ALLOCATE RECIPIENTS / ALLOUER MEMOIRE POUR LES RECEVEURS
This function allocates initial memory for the structure that holds
the data for all recipients.

Cette fonction alloue de la mémoire pour la structure qui contient
les données de tous les receveurs.
*/
CLASS_DECLSPEC EIT_ALLOCATERECIPIENTS_RETURNCODE EI_AllocateRecipients (
        /* Structure for all recipients / Structure de tous les receveurs */
    EIT_ALL_RECIPIENTS * AllRecipients
);

/*
ALLOCATE RESPONDENTS / ALLOUER MEMOIRE POUR LES REPONDANTS
This function allocates initial memory for the structure that holds
the data for all respondents.

Cette fonction alloue de la mémoire pour la structure qui contient
les données de tous les répondants.
*/
CLASS_DECLSPEC EIT_ALLOCATERESPONDENTS_RETURNCODE EI_AllocateRespondents (
        /* Structure for all respondents / Structure de tous les répondants */
    EIT_ALL_RESPONDENTS * AllRespondents
);

/* BUILD K-D TREE / CONSTRUCTION DE L'ARBRE K-D */
/* This function builds the k-d tree used in the search of a closest donor. */
/**/
/* Cette fonction construit l'arbre k-d utilisé dans la recherche d'un plus*/
/* proche donneur.*/

CLASS_DECLSPEC EIT_KDTREE_RETURNCODE EI_BuildKDTree (
    /* Pointer to structure with transformed values of all matching fields*/
    /* for donors only*/
    /**/
    /* Pointeur à la structure contenant les valeurs transformées de*/
    /* tous les champs d'appariement pour les donneurs seulement*/
    EIT_ALL_DONORS * AllDonors,
    /* K-D Tree Structure/Structure de l'arbre K-D*/
    EIT_KDTREE ** KDTree,
    /* Transformed values of all fields/Valeurs transformées de tous*/
    /* les champs.*/
    EIT_TRANSFORMED * AllTransformed,
    /* All matching fields structure */
    /**/
    /* Structure de tous les champs d'appariement */
    EIT_FIELDNAMES * MatchingFields,
    /* All Respondents Structure / Structure de tous les répondants */
    EIT_ALL_RESPONDENTS * AllRespondents
);

CLASS_DECLSPEC int EI_CalculateDonorLimit (int NLimit, double Mrl, int NumberOfRecipients, int NumberOfDonors);
CLASS_DECLSPEC int EI_CountDonorsReachedDonorLimit (EIT_ALL_DONORS * AllDonors, int DonorLimit);

/* DESTRUCT KD TREE / DETRUIRE L'ARBRE KD */
/* This function will free the memory of the KD Tree */
/**/
/* Cette fonction libère la mémoire de l'arbre KD */

CLASS_DECLSPEC void EI_DestructKDTree (
    /* KD Tree / Arbre KD */
    EIT_KDTREE * KDTree
);

CLASS_DECLSPEC EIT_BOOLEAN EI_EnoughDonors (int NumberOfRespondents, int NumberOfRecipients,
    int MinimumNumberOfDonors, double MinimumPercentOfDonors);

/*
FIND SYSTEM MATCHING / IDENTIFIER LES CHAMPS D'APPARIEMENT
This function will identify a set of matching fields for each recipient
using a set of linear equations.  The set is different for each recipient.

Cette fonction identifie un ensemble de champs d'appariement pour
chaque receveur.  L'ensemble des champs sera différent pour chaque receveur.
*/

CLASS_DECLSPEC EIT_MF_RETURNCODE EI_FindMatchingFields (
        /* All Recipients Structure

        Structure avec tous les receveurs */
    EIT_ALL_RECIPIENTS * AllRecipients,
        /* Structure for System Matching Fields

        Structure pour les champs d'appariement identifiés par le système*/
    EIT_FIELDNAMES * SystemMatchingFields,
        /* Edits variables

        Variables des équations*/
    EIT_EDITS  * Edits
);

/* IDENTIFY NEAREST DONORS/ IDENTIFIER LES PLUS PROCHES DONNEURS*/
/* For one recipient data, this function will search in the k-d tree to find*/
/* the N closest donors.*/
/* Pour un receveur donné, cette fonction recherche dans l'arbre k-d*/
/* les N plus proches donneurs.*/

CLASS_DECLSPEC EIT_FINDNEARESTDONORS_RETURNCODE EI_FindNearestDonors (
    /* Number of Donors to Find*/
    /**/
    /* Nombre de donneurs à trouver */
    int N,
    /* One Recipient data */
    /**/
    /* Données pour un receveur */
    EIT_DATAREC * RecipientData,
    /* Key of recipient */
    /**/
    /* Clé du receveur */
    char * RecipientKey,
    /* List indexes of FTI fields for one recipient*/
    /**/
    /* Liste des indices des champs FTI pour un receveur*/
    int * FTIFields,
    /* All Matching Fields */
    /**/
    /* Tous les champs d'appariement */
    EIT_FIELDNAMES * AllMatchingFields,
    /* Table of tranformed values of all matching fields */
    /**/
    /* Tableau des valeurs transformées des champs d'appariement */
    EIT_TRANSFORMED * AllTransformed,
    /* Table of donors with values of FTI fields*/
    /**/
    /* Tableau de donneurs avec les valeurs des champs à imputer */
    EIT_ALL_DONORS *AllDonors,
    /* K-D Tree holding all donors/Arbre k-d avec tous les donneurs*/
    EIT_KDTREE * KDTree,
    /* Pointer to structure of nearest donors found for a recipient */
    /**/
    /* Pointeur à la structure des plus proches donneurs trouvés pour un receveur*/
    EIT_NEARESTDONORS  ** NearestDonors,
	int DonorLimit
);

/* FREE DONORS / LIBERER LA MEMOIRE DES DONNEURS */
/* This function will free the memory allocated to the donors structure*/
/**/
/* Cette fonction libère la mémoire allouée pour la structure des donneurs */

CLASS_DECLSPEC void EI_FreeDonors (
    EIT_ALL_DONORS  *AllDonors
);

/* FREE RECIPIENTS / LIBERER LA MEMOIRE DES RECEVEURS */
/* This function will free the memory allocated to the recipients structure*/
/**/
/* Cette fonction libère la mémoire allouée pour la structure des receveurs*/

CLASS_DECLSPEC void EI_FreeRecipients(
    /* All Recipients Structure / Structure pour tous les receveurs */
    EIT_ALL_RECIPIENTS * AllRecipients
);

/* FREE RESPONDENTS / LIBERER LA MEMOIRE DES REPONDANTS */
/* This function will free the memory allocated to the respondents structure*/
/**/
/* Cette fonction libère la mémoire allouée pour la structure des répondants*/

CLASS_DECLSPEC void EI_FreeRespondents(
    /* All Respondents Structure / Structure pour tous les répondants*/
    EIT_ALL_RESPONDENTS * AllRespondents
);

/* FREE TRANSFORMED / LIBERER LA MEMOIRE DES VALEURS TRANSFORMEES */
/* This function will free the memory allocated to the transformed values structure*/
/**/
/* Cette fonction libère la mémoire allouée pour la structure des données transformées*/

CLASS_DECLSPEC void EI_FreeTransformed (
    EIT_TRANSFORMED  *AllTransformed
);

/*
ALLOCATE FTI FIELDS / ALLOUER MEMOIRE POUR LES CHAMPS FTI
This function allocates memory for list indexes of FTI fields for one recipient.

Cette fonction alloue la mémoire pour la liste des indices des champs FTI pour
un receveur.
*/
CLASS_DECLSPEC EIT_ONERECIPIENTALLOCATE_RETURNCODE EI_OneRecipient_AllocateFTIFields(
    /* Pointer to list indexes of FTI fields for one recipient*/
    /**/
    /* Pointeur vers la liste des indices des champs FTI pour un receveur*/
    int ** FTIFields,
    /* Number of fields in Edits (included constant term)*/
    /**/
    /* Nombre de champs dans les Edits (incluant le terme constant)*/
    int NumberofEditsFields
);

/*
COPY ONE DONOR VALUES / COPIER LES DONNÉES D'UN DONNEUR 
This function copies values of the nearest donor to the recipient fields
to impute.

Cette fonction copie les valeurs du plus proche donneur dans les champs à
imputer du receveur.
*/
CLASS_DECLSPEC void EI_OneRecipient_CopyDonorValues (
        /* Donor index in EIT_ALL_DONORS having the smallest distance*/
        /* with the recipient processed*/
        /**/
        /* Index du donneur dans EIT_ALL_DONORS  ayant la plus faible*/
        /* distance avec le receveur en cours de traitement*/
    int NearestDonorIndex, 
        /* Pointer to All Donors keys*/
        /**/
        /* Pointeur vers les clés de tous les donneurs*/
    char ** AllDonorsKeys,
        /* AllRespondents structure (sorted in ascending order of "keys")*/
        /**/
        /* Structure AllRespondents (triée en ordre ascendant des "clés")*/
    EIT_ALL_RESPONDENTS * AllRespondents,
        /* List indexes of FTI fields for one recipient*/
        /**/
        /* Liste des indices des champs FTI pour un receveur*/
    int * FTIFields,
        /* Pointer to data of the recipient processed*/
        /**/
        /* Pointeur vers les données du receveur en cours de traitement*/
    EIT_DATAREC * RecipientData
);

/* FIND NEXT DONOR INDEX / IDENTIFIER L'INDEX DU PROCHAIN DONNEUR*/
/* This function will identify index of the next donor having the smallest*/
/* distance with the recipient processed, index found among the neareast*/
/* donors returned by the search in KD tree. This index corresponds also*/
/* to the index of this donor in structure EIT_ALL_DONORS.*/
/**/
/* Cette fonction identifie l'index du prochain donneur ayant la plus faible*/
/* distance avec le receveur en traitement, index identifié parmi les plus*/
/* proches donneurs trouvés lors de la recherche dans l'arbre KD. Cet index*/
/* correspond aussi à celui de ce donneur dans la structure EIT_ALL_DONORS.*/

CLASS_DECLSPEC int EI_OneRecipient_FindNextNearestDonor (
    /* Current index in EIT_NEARESTDONORS Structure*/
    /**/
    /* Index courant dans la structure EIT_NEARESTDONORS*/
    int i,
    /* Structure for all nearest donors found in KD tree*/
    /**/
    /*Structure des plus proches donneurs trouvés dans l'arbre KD*/
    EIT_NEARESTDONORS * NearestDonors
);

CLASS_DECLSPEC int EI_OneRecipient_FindNextNearestDnAlt (
    /* Current index in EIT_NEARESTDONORS Structure*/
    /**/
    /* Index courant dans la structure EIT_NEARESTDONORS*/
    int i,
    /* Structure for all nearest donors found in KD tree*/
    /**/
    /*Structure des plus proches donneurs trouvés dans l'arbre KD*/
    EIT_NEARESTDONORS * NearestDonors,
    EIT_ALL_DONORS * AllDonors,
    tSList   * randkey_list,
    tADTList * randnum_list,
    char     * AllRecipients_Key_i
);

/*
FREE MEMORY FOR FTI FIELDS / LIBERE MEMOIRE POUR LES CHAMPS FTI
This function free memory allocated for list indexes of FTI fields for one
recipient.
 
Cette fonction libère la mémoire allouée pour la liste des indices des champs
FTI pour un receveur.
*/
CLASS_DECLSPEC void EI_OneRecipient_FreeFTIFields (
    int * FTIFields
);

/* FREE NEAREST DONORS / LIBERER LA MEMOIRE DES PLUS PROCHES DONNEURS */
/* This function will free the memory allocated to the nearest donors*/
/* structure for one recipient. */
/**/
/* Cette fonction libère la mémoire allouée pour la structure des plus*/
/* proches donneurs pour un receveur.*/

CLASS_DECLSPEC void EI_OneRecipient_FreeNearestDonors(
    /* Nearest Donors Structure/ Structure des plus proches donneurs */
    EIT_NEARESTDONORS * NearestDonors
);

/*
BUILD LIST INDEXES OF FTI FIELDS / CONSTRUIRE LISTE DES INDICES DES CHAMPS FTI
This function gets indexes of FTI fields for one recipient and a keyword is
put at the end of the list.

Cette fonction identifie les indices des champs FTI pour un receveur et un
mot-clé identifie la fin de la liste.
*/
CLASS_DECLSPEC void EI_OneRecipient_GetFTIFields (
    /* List indexes of FTI fields for one recipient*/
    /**/
    /* Liste des indices des champs FTI pour un receveur*/
    int * FTIFields,
    /* Number of fields in Edits (included constant term)*/
    /**/
    /* Nombre de champs dans les Edits (incluant le terme constant)*/
    int NumberofEditsFields,
    /* One Recipient data*/
    /**/
    /* Données pour un receveur*/
    EIT_DATAREC * RecipientData
);

/*
AT LEAST ONE MF OF ANY TYPES / AU MOINS UN MF DE TOUS TYPES
This function verify if a recipient has at least one matching field (MF)
of any types (MFU, MFS or MFB) and returns 1 if it is the case.
 
Cette fonction vérifie si un receveur a au moins un champ d'appariement (MF) 
de n'importe quel type (MFU, MFS ou MFB) et retourne 1 si c'est le cas.
*/
CLASS_DECLSPEC int EI_OneRecipient_HasMatchingFields_AllTypes (EIT_DATAREC *);

/*
AT LEAST ONE MF OF TYPE MFU / AU MOINS UN MF DE TYPE MFU
This function verify if a recipient has at least one matching field (MF)
of type MFU only and returns 1 if it is the case.
 
Cette fonction vérifie si un receveur a au moins un champ d'appariement (MF) 
de type MFU seulement et retourne 1 si c'est le cas.
*/
CLASS_DECLSPEC int EI_OneRecipient_HasMatchingFields_TypeMFU (EIT_DATAREC *);

CLASS_DECLSPEC void EI_PrintKDTree (EIT_KDTREE * KDTree, int Depth);

/* RANDOMLY SEARCH FOR A DONOR - V1 / RECHERCHE ALEATOIRE D'UN DONNEUR - V1*/
/* This function will randomly search and try the values of a donor for one*/
/* recipient without matching fields until one suitable is found or all*/
/* donors have been tried. Post-Imputation Edits must be an input paramater.*/
/**/
/* Cette fonction cherche un donneur potentiel de façon aléatoire et essaie */
/* les valeurs de celui-ci au receveur jusqu'à ce qu'un donneur satisfaisant*/
/* ait été trouvé ou que tous les donneurs aient été essayés. Les règles de*/
/* vérification post-imputation sont un paramètre d'entrée.*/

CLASS_DECLSPEC EIT_RANDOMDONOR_RETURNCODE EI_RandomDonorWithPostEdits (
    /* Recipient data/Données des receveurs*/
    EIT_DATAREC * RecipientData,
    /* List indexes of FTI fields for one recipient*/
    /**/
    /* Liste des indices des champs FTI pour un receveur*/
    int * FTIFields,
    /* Data for all respondents/Données de tous les répondants*/
    EIT_ALL_RESPONDENTS * AllRespondents,
    /* Post-Imputation Edits/Règles de vérification post-imputation*/
    EIT_EDITS * PostEdits,
    /* Transformed values of matching fields for all donors*/
    /**/
    /* Valeurs transformées de tous les champs d'appariement pour tous les*/
    /* donneurs*/
    EIT_ALL_DONORS * AllDonors,
    /* Pointer to index of random donor found */
    /**/
    /* Pointeur à l'indice du donneur aléatoirement sélectionné*/
    int * RamdomDonorRow,
	int DonorLimit,
    int RandNumFlag,
    /* 1 if user has included "randnum" statement, 0 otherwise */
    tSList   * randkey_list,
    /* tSList sorted in ascending order containing the keys of all of the records read for the current by group */
    tADTList * randnum_list,
    /* tADTList sorted in the same order as "randkey_list" containing the value of the random number field of all of the records read for the current by group */
    char * AllRecipients_Key_i,
    /* key of recipient for which a matching donor is sought */
    int LoggingVerbosityLevel
);

/* RANDOMLY SEARCH FOR A DONOR - V2 / RECHERCHE ALEATOIRE D'UN DONNEUR - V2*/
/* This function will randomly search a donor for one recipient without*/
/* matching fields until one suitable is found. Verification by means of*/
/* post-imputation edits is not requested.*/
/**/
/* Cette fonction cherche un donneur de façon aléatoire pour un receveur*/
/* sans champ d'appariement jusqu'à ce qu'un donneur satisfaisant ait été*/
/* trouvé. La vérification au moyen de règles post-imputation n'est pas*/
/* demandée.*/

CLASS_DECLSPEC EIT_RANDOMDONOR_RETURNCODE EI_RandomDonorWithoutPostEdits (
    /* Recipient data/Données des receveurs*/
    EIT_DATAREC * RecipientData,
    /* List indexes of FTI fields for one recipient*/
    /**/
    /* Liste des indices des champs FTI pour un receveur*/
    int * FTIFields,
    /* Data for all respondents/Données de tous les répondants*/
    EIT_ALL_RESPONDENTS * AllRespondents,
    /* Transformed values of matching fields for all donors*/
    /**/
    /* Valeurs transformées de tous les champs d'appariement pour tous les*/
    /* donneurs*/
    EIT_ALL_DONORS * AllDonors,
    /* Pointer to index of random donor found */
    /**/
    /* Pointeur à l'indice du donneur aléatoirement sélectionné*/
    int * RamdomDonorRow,
	int DonorLimit
);


CLASS_DECLSPEC void EI_RecipientsPrint (EIT_ALL_RECIPIENTS *);

CLASS_DECLSPEC void EI_RespondentsPrint (EIT_ALL_RESPONDENTS *);

/* TRANSFORM MATCHING FIELD VALUES/TRANSFORMATION DES VALEURS DES CHAMPS D'APPARIEMENT*/
/* This function will read all the valid values of one matching field at a time and store*/
/* the information in a structure of type EIT_MATCHVAL.  Once the values are transformed,*/
/* they will be stored in a variable pointing to a table of EIT_MATCHVAL.*/
/**/
/* Cette fonction lit toutes les valeurs valides d'un champ d'appariement à la fois et met*/
/* l'information dans une structure de type EIT_MATCHVAL.  Une fois les valeurs transformées,*/
/* elles seront stockées dans une variable qui pointera à un tableau de EIT_MATCHVAL.*/

CLASS_DECLSPEC EIT_TRF_RETURNCODE EI_Transform (
    /* Matching field Name / Nom du champ d'appariement */
    EIT_FIELDNAMES * MatchingFields,
    /* Callback function to read matching field data */
    /**/
    /* This function will read the data for*/
    /* one field at a time*/
    /**/
    /* Fonction de lecture des valeurs des champs d'appariement*/
    /**/
    /* Cette fonction lit les données pour un champ à la fois*/
    EIT_MFREADCALLBACK ReadMFData,
    /* Structure to hold transformed data*/
    /**/
    /* Structure de valeurs transformées*/
    EIT_TRANSFORMED * AllTransformed
);

#endif
