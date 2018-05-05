/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Se incluyen algunas rutinas modificadas de Numerical Recipes in C,
 *	entre las que se encuentran reporte de errores, declaracion y
 *	y liberacion de arreglos y matrices
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Librerias a incluir */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Reportar el mensaje de error a stderr, y luego salir
 *	del programa con la señal de exit(1)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void nrerror(char error_text[])    
{	
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...saliendo del sistema...\n");
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
	
	//Asignar el tamaño de memoria
	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) //Si algo falla en la asignacion de memoria
		nrerror("La asignacion en el vector() fallo");
  
	v -= nl;
	for(i=nl;i<=nh;i++) 
		v[i] = 0.0;	//Iniciar a cero elementos del vector
	return v;	//Regresamos el vector asigando e iniciado
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Asignar una matriz con indice de columna de nrl a nrh 
 *	inclusive, e indice de columan de ncl a nch
 *	inclusive.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double **AllocMatrix(short nrl,short nrh, short ncl,short nch)
{
	short i,j;
	double **m;
  
	//Asignar el tamaño de la memoria double*
	m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*));
	if (!m) 
		nrerror("Fallo en asignacion de memoria 1 en la matriz()");
	m -= nrl;
  
	for(i=nrl;i<=nrh;i++) 
	{
		//Asignar el tamaño de la memoria double
		m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) 
			nrerror("Fallo en asignacion de memoria 2 en la matriz()");
		m[i] -= ncl;
	}
  
	//Iniciar a cero los elementos de la matriz
	for(i=nrl;i<=nrh;i++)
		for(j=ncl;j<=nch;j++) 
			m[i][j] = 0.0;
	return m;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Liberar la memoria del vector
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeVector(double *v,short nl,short nh)
{
	free((char*) (v+nl));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Liberar la memoria de la matriz
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeMatrix(double **m,short nrl,short nrh,short ncl,short nch)
{
	short i;
  
	for(i=nrh;i>=nrl;i--) 
		free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}
