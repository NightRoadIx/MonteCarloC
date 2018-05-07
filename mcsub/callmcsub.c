/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * callmcsub.c
 *
 * Programa principal que:
 *  1. Define los parámetros para la simulación Monte Carlo
 *  2. Llama a la rutina mcsub()
 *  3. Salva los resultados en un archivo 
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

 /*Librerias a utilizar*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include "mcsublib.h"

/* * * * * * * * DEFINICIONES * * * * * * * */
#define BINS        80      // Número de "recipientes" NZ & NR para z & r

/* * * * * * * * PROGRAMA PRINCIPAL * * * * * * * */
void main() 
{
	/* * * * * * * * DECLARACIÓN DE VARIABLES * * * * * * * */
	/* Número de archivo a salvar */
	int      Nfile = 1;       /* salvar como mcOUTi.dat, donde i = Narchivo *
	/* PARÁMETROS DEL TEJIDO */
	double   mua  = 1.0;      // Coeficiente de absorción [cm^-1] *
	double   mus  = 100;      // Coeficiente de esparcimiento [cm^-1]
	double   g    = 0.90;     // Coeficiente de anisotropia
	double   n1   = 1.40;     // Indice de refracción del medio
	double   n2   = 1.00;     // Indice de refracción del medio externo
	/* PARÁMETROS DEL HAZ */
	short    mcflag = 0;	//Tipo de fuente: 
							//0 = haz colimado, 1 = Gaussiana enfocada, 
							//2 = puntual isotrópica
	double   radius = 0;    // Usado si mcflag = 0 ó 1
	double   waist  = 0.10; // Usado si mcflag = 1
	double   zfocus = 1.0;    // Usado si mcflag = 1
	double   xs     = 0.0;    // Usado si mcflag = 2, posición x fuente puntual
	double   ys     = 0.0;    // Usado si mcflag = 2, posición y fuente puntual
	double   zs     = 0.0;    // Usado si mcflag = 2, posición z fuente puntual, ó mcflag = 0
	int      boundaryflag = 1; // 0 = medio infinito, 1 = frontera aire/tejido
	/* PARÁMETROS DE EJECUCIÓN */
	double   Nphotons = 10000; // Número de fotones a ser lanzados
	double   dr     = 0.0100; // tamaño del "recipiente" radial [cm]
	double   dz     = 0.0100; // tamaño del "recpiente" en profundidad [cm]
	short    PRINTOUT = 1;
	/* PARÁMETROS DE CONFIGURACIÓN DE SALIDA, VECTORES, ARREGLOS */
	double    S;    // Reflectancia especular en la frontera aire/tejido
	double    A;    // Fracción total de luz absorbida por el tejido
	double    E;    // Fracción total de luz que escapó del tejido
	double*   J;    // Flujo de escape, J[ir], [W/cm2 por W incidente]
	double**  F;    // Ritmo de fluencia, F[iz][ir], [W/cm2 por W incidente]
	short     NR = BINS;    // Número de "recipientes" radiales
	short     NZ = BINS;    // Número de "recipientes" en profunidad
	/*Otros parámetros*/
	double albedo, Nsteps, THRESH, t, tperstep;
	char *arhiva;
	
	//Definiciones iniciales
	//Asignar memoria para los "recipientes" del ritmo de fluencia
	J       = AllocVector(1, NR);        // para el flujo de escape
	F       = AllocMatrix(1, NZ, 1, NR); // para el ritmo de fluencia absorbido
	
	/**** ENTRADA
	   Entrada de propiedades opticas
	   Entrada del tamaño del paquete y el arreglo
	   Entrada del número de fotones
	*****/
	textmode(C4350);	//Modo de texto 80 columnas x 43||50 filas
	clrscr();			//Limpiar la pantalla
	/* El usuario añade las características ópticas del tejido */
	//printf("Número de fotones a lanzar N = "); scanf("%lf", &Nphotons);
	gotoxy(1,1); cprintf("<< Parámetros ópticos del medio >>");
	gotoxy(1,2); cprintf("[Coef. de absorcion]     [cm^-1] mua = "); scanf("%lf", &mua);
	gotoxy(1,3); cprintf("[Coef. de esparcimiento] [cm^-1] mus = "); scanf("%lf", &mus);
	gotoxy(1,4); cprintf("[Coef. de anisotropia]             g = "); scanf("%lf", &g);
	gotoxy(1,5); cprintf("[Indices de refracción]:");
	gotoxy(1,6); cprintf("(medio)                           n1 = "); scanf("%lf", &n1);
	gotoxy(1,7); cprintf("(externo)                         n1 = "); scanf("%lf", &n2);
	gotoxy(1,8); cprintf(">>Tipo de Fuente:n");
	gotoxy(1,9); cprintf("(0)Haz colimado");
	gotoxy(1,10); cprintf("(1)Gaussiana");
	gotoxy(1,11); cprintf("(2)Puntual isotrópica");
	gotoxy(1,12); cprintf("-->"); scanf("%d",&mcflag); fflush(stdin);
	//Seleccionar los parámetros acorde a la fuente seleccionada
	if(mcflag==0)
	{
		gotoxy(2,13); cprintf("\n{{HAZ COLIMADO}}\n");
		gotoxy(2,14); cprintf("[Radio fuente]  [cm]     radius = "); scanf("%lf", &radius);
		gotoxy(2,15); cprintf("[Posición z]    [cm]         zs = "); scanf("%lf", &zs);
	}
	else if(mcflag==1)
	{
		gotoxy(2,13); cprintf("\n{{GAUSSIANA}}\n");
		gotoxy(2,14); cprintf("[Radio fuente]              [cm] radius = "); scanf("%lf", &radius);
		gotoxy(2,15); cprintf("[Distancia axial (cintura)] [cm]  waist = "); scanf("%lf", &waist);
		gotoxy(2,16); cprintf("[Distancia focal en z]      [cm]     zs = "); scanf("%lf", &zs);
	}
	else if(mcflag==2)
	{
		gotoxy(2,13); cprintf("\n{{PUNTUAL ISOTRÓPICA}}\n");
		gotoxy(2,14); cprintf("[Posición x] (0,%.2lf)       xs = ",BINS*dr); scanf("%lf", &xs);
		gotoxy(2,15); cprintf("[Posición y] (0,%.2lf)       ys = ",BINS*dr); scanf("%lf", &ys);
		gotoxy(2,16); cprintf("[Posición z] (0,%.2lf)       zs = ",BINS*dr); scanf("%lf", &zs);
	}
	// Tamaño de los pasos de los "recipientes" para filas y columnas (radial y en profundidad)
	dr = 0.0020;
	dz = 0.0020;
	/* 
		A partir del número de bins definidos y el tamaño de estos pasos, se obtiene la distancia que
		se cubrirá en la simulación, por medio de:
		BINS * dr = distancia radial máxima
		BINS x dz = diatancia en profundidad máxima
	*/

	// Elegir Nfotones
	THRESH = 1e-4;
	//Sacar el albedo, los pasos de los fotones,
	albedo = mus/(mua + mus);
	Nsteps = log(THRESH)/log(albedo);
	tperstep = 249e-9;
	t = 30;
	Nphotons = (double)( (long)( t/(tperstep*Nsteps) ) );
	
	_setcursortype(_NOCURSOR); //Quitar el cursor
	/* Puede modificarse si se desea imprimir los parámetros ON=1 ó OFF=0 */
	if (1) 
	{
		clrscr();	//Limpiar la pantalla
		/* imprimir las propiedades de la simulación como resumen */
		gotoxy(1,1); cprintf("----- PARAMETROS INICIALES -----");
		gotoxy(1,2); cprintf("<<EXCITACIÓN>>");
		gotoxy(1,3); cprintf("muax 	    = %1.3f cm^-1",mua);
		gotoxy(1,4); cprintf("musx 	    = %1.3f cm^-1",mus);
		gotoxy(1,5); cprintf("gx 		= %1.3f\n",g);
		gotoxy(1,6); cprintf("n1 		= %1.3f\n",n1);
		gotoxy(1,7); cprintf("n2 		= %1.3f\n",n2);
		gotoxy(1,8); cprintf("<<FUENTE ");
		if(mcflag == 0) { gotoxy(10,8); cprintf("HAZ COLIMADO>>"); }
		else if(mcflag == 1) { gotoxy(10,8); cprintf("GASUSSIANA>>"); }
		else if(mcflag == 2) { gotoxy(10,8); cprintf("PUNTUAL ISOTRÓPICA>>"); }
		gotoxy(2,9);  cprintf("radio 				= %1.4f cm",radius);
		gotoxy(2,10); cprintf("distancia axial 	    = %1.4f cm",waist);
		gotoxy(2,11); cprintf("distancia focal z	= %1.4f cm",radius);
		gotoxy(2,12); cprintf("Posición en x 		= %1.4f",xs);
		gotoxy(2,13); cprintf("Posición en y 		= %1.4f",ys);
		gotoxy(2,14); cprintf("Posición en z 		= %1.4f",zs);
		gotoxy(1,15); cprintf("<<OTROS PARÁMETROS>>");
		gotoxy(1,16); cprintf("#fotones = %5.4e", Nphotons);
		gotoxy(1,17); cprintf("dr                       = %1.4f cm",dr);
		gotoxy(1,18); cprintf("dz                       = %1.4f cm",dz);
		gotoxy(1,19); cprintf("---------------");
	}	
	
	/* Llamada a la subrutina Monte Carlo */
	mcsub(mua, mus, g, n1, n2,
		NR, NZ, dr, dz, Nphotons,
		mcflag, xs, ys, zs, boundaryflag,
		radius, waist, zfocus,
		J, F, &S, &A, &E,
		PRINTOUT);               /* va a regresar J, F, S, A, E */

	if (1==1)
	{
		//Obtener el número de archivo para salvar de la forma: mcOUTi.dat, donde i es la entrada
		fflush(stdin);
		gotoxy(1,19); printf("\nNúmero de archivo: "); scanf("%d", &Nfile);
		SaveFile(Nfile, J, F, S, A, E,       // salvar a "mcOUTi.dat", i = Nfile
				mua, mus, g, n1, n2,
				mcflag, radius, waist, xs, ys, zs,
				NR, NZ, dr, dz, Nphotons);
	}

	//Mostrar al final los resultados y el nombre del archivo
	sprintf(arhiva, "mcOUT%d.dat", Nfile);
	gotoxy(1,21); cprintf("\n\nArchivo %s", arhiva);
	gotoxy(1,22); cprintf("Nfotones  = %5.1e\n", Nphotons);
	gotoxy(1,23); cprintf("Especular = %5.6f\n", S);
	gotoxy(1,24); cprintf("Absorbido = %5.6f\n", A);
	gotoxy(1,25); cprintf("Escapado  = %5.6f\n", E);
	gotoxy(1,26); cprintf("Total     = %5.6f\n", S+A+E); getch();

	//Liberar la memoria
	FreeVector(J, 1, NR);
	FreeMatrix(F, 1, NZ, 1, NR);
}