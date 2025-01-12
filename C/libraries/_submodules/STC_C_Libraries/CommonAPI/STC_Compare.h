#ifndef STC_COMPARE_H
#define STC_COMPARE_H

/* this file contains things related to comparation of doubles */

/* doubles are compared using X digits (use 1.0e-X) */
#define STC_COMPARE_EPSILON (1.0e-14) /* we use 14 digits */

/* compare number with default epsilon value */
#define STC_COMPARE_GT(X,Y) (STC_CompareNumber((X),(Y),STC_COMPARE_EPSILON)==1)
#define STC_COMPARE_GE(X,Y) (STC_CompareNumber((X),(Y),STC_COMPARE_EPSILON)>=0)
#define STC_COMPARE_EQ(X,Y) (STC_CompareNumber((X),(Y),STC_COMPARE_EPSILON)==0)
#define STC_COMPARE_LE(X,Y) (STC_CompareNumber((X),(Y),STC_COMPARE_EPSILON)<=0)
#define STC_COMPARE_LT(X,Y) (STC_CompareNumber((X),(Y),STC_COMPARE_EPSILON)==-1)

/* compare numbers with caller's epsilon value */
#define STC_COMPARE_GT_EPS(X,Y,eps) (STC_CompareNumber((X),(Y),(eps))==1)
#define STC_COMPARE_GE_EPS(X,Y,eps) (STC_CompareNumber((X),(Y),(eps))>=0)
#define STC_COMPARE_EQ_EPS(X,Y,eps) (STC_CompareNumber((X),(Y),(eps))==0)
#define STC_COMPARE_LE_EPS(X,Y,eps) (STC_CompareNumber((X),(Y),(eps))<=0)
#define STC_COMPARE_LT_EPS(X,Y,eps) (STC_CompareNumber((X),(Y),(eps))==-1)

/*
Compares two numbers.

Should never be use directly.
Please use the provided macros instead of this function.

STC_COMPARE_EQ(X,Y)
STC_COMPARE_GT(X,Y)
STC_COMPARE_GE(X,Y)
STC_COMPARE_LE(X,Y)
STC_COMPARE_LT(X,Y)
*/
CLASS_DECLSPEC int STC_CompareNumber (
        /* A number. */
    double X,
        /* Another number. */
    double Y,
        /* The epsilon value. */
    double Epsilon);

#endif
