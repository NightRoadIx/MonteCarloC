/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Se incluyen algunas rutinas modificadas de Numerical Recipes in C,
 *	entre las que se encuentran reporte de errores, declaracion y
 *	y liberacion de arreglos y matrices
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * #include <stdlib.h> #include <stdio.h> #include <math.h>
 */
/* Librerias a incluir */
#include "conv.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Reportar el mensaje de error a stderr, y luego salir
 *	del programa con la señal de exit(1)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void nrerror(char error_text[])
{
	fprintf(stderr, "%s.\n", error_text);
	fprintf(stderr, "...saliendo del sistema...\n");
	exit(1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Asignar un arreglo con indice de n1 a nh inclusive.
 *
 *	La matriz y el vector original del Numerical Recipes in C
 *	no inicializa los elementos a cero. Esto se logra mediante
 *	las siguientes funciones.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double *AllocVector(short nl, short nh)
{
	double *v;
	short i;

	v = (double *) malloc((unsigned) (nh - nl + 1) * sizeof(double));
	if (!v)
		nrerror("La asignacion en el vector() fallo");

	for (i = nl; i <= nh; i++)
		v[i] = 0.0;			/* init. */
	return v - nl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Asignar una matriz con indice de columna de nrl a nrh 
 *	inclusive, e indice de columan de ncl a nch
 *	inclusive.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double **AllocMatrix(short nrl, short nrh, short ncl, short nch)
{
	short i, j;
	double **m;

	//Asignar el tamaño de la memoria double*
	m = (double **) malloc((unsigned) (nrh - nrl + 1) * sizeof(double *));
	if (!m)
		nrerror("Fallo en asignacion de memoria 1 en la matriz()");
	m -= nrl;

	for (i = nrl; i <= nrh; i++) 
	{
		//Asignar el tamaño de la memoria double
		m[i] = (double *) malloc((unsigned) (nch - ncl + 1) * sizeof(double));
		if (!m[i])
			nrerror("Fallo en asignacion de memoria 2 en la matriz()");
		m[i] -= ncl;
	}

	//Iniciar a cero los elementos de la matriz
	for (i = nrl; i <= nrh; i++)
		for (j = ncl; j <= nch; j++)
			m[i][j] = 0.0;
	return m;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Liberar la memoria del vector
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeVector(double *v, short nl, short nh)
{
	free((char *) (v + nl));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Liberar la memoria de la matriz
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeMatrix(double **m, short nrl, short nrh, short ncl, short nch)
{
	short i;

	for (i = nrh; i >= nrl; i--)
		free((char *) (m[i] + ncl));
	free((char *) (m + nrl));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Metodo numerico de Integracion trapezoidal
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
//Se define FUNC(x) que sera utilizado a lo largo de la funcion
#define FUNC(x) ((*func)(x))

float trapzd(float (*func) (float), float a, float b, int n)
{
	float x, tnm, sum, del;
	static float s;
	static int  it;
	int j;

	if (n == 1) 
	{
		it = 1;
		s = 0.5 * (b - a) * (FUNC(a) + FUNC(b));
	} 
	else 
	{
		tnm = it;
		del = (b - a) / tnm;
		x = a + 0.5 * del;
		for (sum = 0.0, j = 1; j <= it; j++, x += del)
			sum += FUNC(x);
		it *= 2;
		s = 0.5 * (s + (b - a) * sum / tnm);
	}
	return (s);
}
//Se quita la defincion de FUNC, que ya no sera utilizado
#undef FUNC

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Permite al usuario cambiar EPS
 *  Modificado para hacer al menos tres trapzd() en el caso
 *  de los datos es ruidoso.  9/26/1994 Lihong Wang.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define JMAX 30
float qtrap(float (*func) (float), float a, float b, float EPS)
{
	int j;
	float s, s_old = 0;

	for (j = 1; j <= JMAX; j++) 
	{
		s = trapzd(func, a, b, j);
		if (j <= 3 || fabs(s - s_old) > EPS * fabs(s_old))
			s_old = s;
		else
			break;
	}
	return (s);
}
#undef JMAX

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Funcion modificada de Bessel exp(-x) I0(x), para x >=0.
 *	Fue modificado de la original bessi0(). Instead of
 *	En ligar de I0(x) en su, regresa I0(x) exp(-x).
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double BessI0(double x)
{
	double ax, ans;
	double y;

	if ((ax = fabs(x)) < 3.75) 
	{
		y = x / 3.75;
		y *= y;
		ans = exp(-ax) * (1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2))))));
	} 
	else 
	{
		y = 3.75 / ax;
		ans = (1 / sqrt(ax)) * (0.39894228 + y * (0.1328592e-1
		+ y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2
	    + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1
					       + y * 0.392377e-2))))))));
	}
	return ans;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Obtener una entrada de tipo short
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
short GetShort(short Lo, short Hi)
{
	char in_str[STRLEN];
	short x;

	//Obtener la entrada
	gets(in_str);
	sscanf(in_str, "%hd", &x);
	//Verificar que este dentro de los intervalos permitidos
	while (x < Lo || x > Hi) 
	{
		printf("...Parametro erroneo.  Ingresar de nuevo: ");
		gets(in_str);
		sscanf(in_str, "%hd", &x);
	}
	return (x);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Obtener una entrada de tipo float
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float GetFloat(float Lo, float Hi)
{
	char in_str[STRLEN];
	float x;

	//Obtener la entrada
	gets(in_str);
	sscanf(in_str, "%f", &x);
	//Verificar que este dentro de los intervalos permitidos
	while (x < Lo || x > Hi) 
	{
		printf("...Parametro erroneo.  Ingresar de nuevo: ");
		gets(in_str);
		sscanf(in_str, "%f", &x);
	}
	return (x);
}
