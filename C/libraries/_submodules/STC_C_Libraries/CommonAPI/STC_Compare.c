#include <math.h>

/*********************************************************************
compare 2 numbers, d1 and d2
returns  0 if d1 = d2
returns  1 if d1 > d2
returns -1 if d1 < d2
*********************************************************************/
int STC_CompareNumber (
    double d1,
    double d2,
    double Epsilon)
{
    double Delta;
    double Difference;
    int Exponent;

    Difference = d1 - d2;

	/* Frexp() calculates the exponent value based on  */
    /* d1 = mantissa * 2^Exponent  or                  */
    /* d2 = mantissa * 2^Exponent                      */

    frexp (fabs (d1) > fabs (d2) ? d1 : d2, &Exponent);

	/* ldexp returns the floating point value from multiplying */
    /* Epsilon * 2^Exponent                                    */
    /* Therefore, Delta will be equal to full floating point   */
    /* value of this calculation.                              */

	
    if (fabs(Difference) < 1.e-08) 
		Delta = 1;                   /* consider d1 and d2 as identical if difference is < 1.e-08 */
	else
        Delta = ldexp (Epsilon, Exponent);

	/*printf ("differ = %f , Delta = %f\n", Difference, Delta);*/

    if (Difference > Delta)
        return 1; /* d1 > d2 */
    else if (Difference < -Delta)
        return -1; /* d1 < d2 */

    return 0; /* d1 == d2 */
}
