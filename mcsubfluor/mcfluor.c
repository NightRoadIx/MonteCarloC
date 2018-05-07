/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * mcfluor.c
 * El progrma realiza lo siguiente:
 *  1. Establece los parámetros para simular la fluorescencia en un medio por Monte Carlo
 *  2. Llama a la rutina mcsub() [en la libreria mcsubLIB.h] para simular 
 *     Excitación en el tejido
 *     Emisión uniforme debido a la fluorescencia de fondo
 *     Emisión debido al fluoroforo no centrado (el añadido externamente)
 *  3. Salvar los resultados en los siguientes archivos
 *     mcOUT101.dat -> excitación
 *     mcOUT102.dat -> Emisión uniforme de los fluoroforos de fondo
 *     mcOUT103.dat -> Emisión del fluoroforo descentrado en la posición +x
 *     mcOUT104.dat -> Emisión del fluoroforo descentrado en la posición -x
 *
 * Utiliza la libreria mcsubLIB.h
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

 /*Librerias a utilizar*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include "mcsubLIB.h"

/* * * * * * * * DEFINICIONES * * * * * * * */
#define BINS        45      // Número de "recipientes" NZ & NR para z & r

/* * * * * * * * FUNCIONES PROTOTIPO * * * * * * * */
/*
	Función limpieza de un pedazo de pantalla
	xi -> posición en x inicial
	yi -> posición en y inicial
	xf -> posición en x final
	yf -> posición en y final
*/
void cecran(int xi, int yi, int xf, int yf);

/* * * * * * * * PROGRAMA PRINCIPAL * * * * * * * */
void main()
{
	/* * * * * * * * DECLARACIÓN DE VARIABLES * * * * * * * */
	/* Número de archivo a salvar */
	int Nfile = 1;		// salvar como mcOUTi.dat, donde i = Narchivo
	int Nfilex= 1;		// Se obtiene como (NFile * 100) + i { 1 exc., 2 fluor. de fondo, 3 fluor+, 4 fluor- }
	/* PARÁMETROS DEL TEJIDO (Excitación)*/
	double muax = 0.5;		// Coeficiente de absorción de excitación [cm^-1]
	double musx = 20.0;	// Coeficiente de esparcimiento de excitación [cm^-1]
	double gx = 0.75;		// Coeficiente de anisotropia
	double mua  = 1.0;      // Coeficiente de absorción [cm^-1]
	double mus  = 100;      // Coeficiente de esparcimiento [cm^-1]
	double g    = 0.90;     // Coeficiente de anisotropia
	double n1 = 1.33;		// Indice de refracción del medio
	double n2 = 1.00;		// Indice de refracción del medio externo
	/* PARÁMETROS DEL HAZ */
	short  mcflag = 0;		//Tipo de fuente:
							//0 = haz colimado, 1 = Gaussiana enfocada,
							//2 = puntual isotrópica
	double radius = 0.01;	// Usado si mcflag = 0 ó 1
	double waist = 0.0;		// Usado si mcflag = 1 Distancia axial fuente gaussiana
	double zfocus = 0.0;	// Usado si mcflag = 1 Distancia focal en z
	double xs = 0.0;		// Usado si mcflag = 2 posición x fuente puntual
	double ys = 0.0;		// Usado si mcflag = 2 posición y fuente puntual
	double zs = 0.0;		// Usado si mcflag = 2 posición z fuente puntual
	int    boundaryflag = 1;// 0 = medio infinito, 1 = frontera aire/tejido
	/* PARAMETROS DE SALIDA */
	double S;				// Reflectancia especular en la frontera aire/tejido
	double A;				// Fracción total de luz absorbida por el tejido
	double E;				// Fracción total de luz que escapó del tejido
	short  PRINTOUT = 1;	// PRINTOUT = 1 habilita la impresión del # de fotones lanzados
	/* FLUORESCENCIA DE FONDO */
	double muaf = 5.0;		// Coeficiente de absorción de fluorescencia [cm^-1]
	double musf = 50.0;		// Coeficiente de esparcimiento de fluorescencia [cm^-1]
	double gf = 0.0;		// Coeficiente de anisotropia de fluorescencia
	double eC = 1.0;		// Coeficiente de extinción x concentración del fluoroforo [cm^-1] */
	double Y = 1.0;			// Producción de energía por fluorescencia [W de fluorescencia /W de excitación absrobido]
	/* HETEROGENEIDAD (FLUOROFORO) */
	double xh = 0.2;		// Posición de la Heterogeneidad en x
	double yh = 0.0;		// Posición de la Heterogeneidad en y
	double zh = 0.3;		// Posición de la Heterogeneidad en z
	double heC = 0.1;		// eC de heterogeneidad
	double hY = 1.0;		// Producción energética de heterogeneidad
	double hrad = 0.01;		// Radio de la esfera de heterogeneidad
	/* OTROS PARÁMETROS */
	double Nruns = 1;		// Número de fotones lanzados = Nruns x 1e6 (la exactitud de los resultados es
							// proporcional a sqrt(Nphotons))
	double dr = 0.0500;		// Tamaño del "recipiente" radial [cm]
	double dz = 0.0500;		// Tamaño del "recipiente" en profunidad [cm]
							// A pesar de que los parámetros iniciales se toman en coordenadas cartesianas, el problema
							// presenta una simetría cilindrica por tanto se simula y presenta el resultado en coordenadas
							// cilindricas
	char label[1];			//Caracter
	double PI = 3.1415926;	//Definición de Pi hasta 7 cifras
	double Nphotons;		// Número de fotones a ser lanzados
	long ir, iz, iir, iiz, ii;
	//Algunas variables temporales
	double temp, temp3, temp4, r, r1, r2;
	double timeperEx, timeperEm;  // minutos por fotón
	//Variables para el reloj
	double start_time, finish_time1, finish_time2, finish_time3;
	double timeA, timeB;
	time_t now;		// Estructura para obtener el tiempo actual
	double *Jx, *Jf, *temp1;	// Flujo de escape, J[ir], [W/cm2 por W incidente]
	double **Fx, **Ff, **temp2;	// Ritmo de fluencia, F[iz][ir], [W/cm2 por W incidente]
	long NR = BINS; // Número de "recipientes" radiales
	long NZ = BINS; // Número de "recipientes" en profunidad

	//Definiciones iniciales
	//Asignar memoria para los "recipientes" del ritmo de fluencia
	Jx		= AllocVector(1,BINS);				// Flujo de escape
	Jf		= AllocVector(1,BINS);				// Flujo de escape
	temp1	= AllocVector(1,BINS);				// Matriz temporal
	Fx		= AllocMatrix(1, BINS, 1, BINS); 	// Para la excitación absorbida
	Ff		= AllocMatrix(1, BINS, 1, BINS); 	// Para la fluorescencia absorbida
	temp2	= AllocMatrix(1, BINS, 1, BINS); 	// Matriz temporal
	
	/**** ENTRADA
	   Entrada de propiedades opticas
	   Entrada del tamaño del paquete y el arreglo
	   Entrada del número de fotones
	*****/
	textmode(C4350);	//Modo de texto 80 columnas x 43||50 filas
	clrscr();			//Limpiar la pantalla
	/* El usuario añade las características ópticas del tejido */
	//printf("Número de fotones a lanzar N = "); scanf("%lf", &Nphotons);
	gotoxy(1,1); cprintf("<< Parametros opticos del medio >>");
	gotoxy(1,2); cprintf("[Coef. de absorcion]     [cm^-1] mua = "); scanf("%lf", &muax);
	gotoxy(1,3); cprintf("[Coef. de esparcimiento] [cm^-1] mus = "); scanf("%lf", &musx);
	gotoxy(1,4); cprintf("[Coef. de anisotropia]             g = "); scanf("%lf", &gx);
	gotoxy(1,5); cprintf("[Indices de refracción]:");
	gotoxy(1,6); cprintf("(medio)                           n1 = "); scanf("%lf", &n1);
	gotoxy(1,7); cprintf("(externo)                         n1 = "); scanf("%lf", &n2);
	gotoxy(1,8); cprintf(">>Tipo de Fuente:n");
	gotoxy(3,9); cprintf("(0)Haz colimado");
	gotoxy(3,10); cprintf("(1)Gaussiana");
	gotoxy(3,11); cprintf("(2)Puntual isotrópica");
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
	gotoxy(1,17); cprintf("<<Parametros fluorescencia de fondo del medio>>\n");
	gotoxy(1,18); cprintf("[Coef. de absorcion]         muaf = "); scanf("%lf", &muaf);
	gotoxy(1,19); cprintf("[Coef. de esparcimiento]     musf = "); scanf("%lf", &musf);
	gotoxy(1,20); cprintf("[Coef. de anisotropia]         gf = "); scanf("%lf", &gf);
	gotoxy(1,21); cprintf("[Coef. de extinción x Conc.]   eC = "); scanf("%lf", &eC);
	gotoxy(1,22); cprintf("[Eficiencia cuántica]           Y = "); scanf("%lf", &Y);
	gotoxy(1,23); cprintf("<<Parametros de un fluoroforo en el medio>>\n");
	gotoxy(1,24); cprintf("[Pos. en x] (0,%.2lf)           xh = ",BINS*dr); scanf("%lf", &xh);
	gotoxy(1,25); cprintf("[Pos. en y] (0,%.2lf)           yh = ",BINS*dz); scanf("%lf", &yh);
	gotoxy(1,26); cprintf("[Posición en z]                zh = "); scanf("%lf", &zh);
	gotoxy(1,27); cprintf("[Coef. de extinción x Conc.]  heC = "); scanf("%lf", &heC);
	gotoxy(1,28); cprintf("[Eficiencia cuantica]          hY = "); scanf("%lf", &hY);
	gotoxy(1,29); cprintf("[Radio fluoroforo]           hrad = "); scanf("%lf", &hrad);
	gotoxy(1,30); cprintf("[# Archivo]                       ="); scanf("%d", &Nfile);

	//printf("Presionar cualquier tecla para iniciar programa-->>"); getch(); clrscr();
	// Inicio del programa e inicia el contador del tiempo
	start_time = clock();
	now = time(NULL);
	gotoxy(1,30); cprintf("%s\n", ctime(&now));	// Imprimir el tiempo inicial
	
	_setcursortype(_NOCURSOR); //Quitar el cursor
	/* Puede modificarse si se desea imprimir los parámetros ON=1 ó OFF=0 */
	if (1) 
	{
		clrscr();	//Limpiar la pantalla
		/* imprimir las propiedades de la simulación como resumen */
		gotoxy(1,1); cprintf("----- PARAMETROS INICIALES -----");
		gotoxy(1,2); cprintf("<<EXCITACION>>");
		gotoxy(1,3); cprintf("muax 	= %1.3f cm^-1",muax);
		gotoxy(1,4); cprintf("musx 	= %1.3f cm^-1",musx);
		gotoxy(1,5); cprintf("gx 		= %1.3f",gx);
		gotoxy(1,6); cprintf("n1 		= %1.3f",n1);
		gotoxy(1,7); cprintf("n2 		= %1.3f",n2);
		gotoxy(1,8); cprintf("<<FUENTE ");
		if(mcflag == 0) { gotoxy(10,8); cprintf("HAZ COLIMADO>>"); }
		else if(mcflag == 1) { gotoxy(10,8); cprintf("GASUSSIANA>>"); }
		else if(mcflag == 2) { gotoxy(10,8); cprintf("PUNTUAL ISOTRÓPICA>>"); }
		gotoxy(2,9); cprintf("radio 				= %1.4f cm",radius);
		gotoxy(2,10); cprintf("distancia axial 	= %1.4f cm",waist);
		gotoxy(2,11); cprintf("distancia focal z	= %1.4f cm",radius);
		gotoxy(2,12); cprintf("Posición en x 		= %1.4f",xs);
		gotoxy(2,13); cprintf("Posición en y 		= %1.4f",ys);
		gotoxy(2,14); cprintf("Posición en z 		= %1.4f",zs);
		gotoxy(1,15); cprintf("<<FLUORESCENCIA DE FONDO>>");
		gotoxy(1,16); cprintf("muaf 	= %1.3f cm^-1",muaf);
		gotoxy(1,17); cprintf("musf 	= %1.3f cm^-1",musf);
		gotoxy(1,18); cprintf("gf 		= %1.3f",gf);
		gotoxy(1,19); cprintf("eC 		= %1.3f cm^-1",eC);
		gotoxy(1,20); cprintf("Y 		= %1.3f W/W",Y);
		gotoxy(1,21); cprintf("<<HETEROGENEIDAD (FLUOROFORO)>>");
		gotoxy(1,22); cprintf("xh 		= %1.4f",xh);
		gotoxy(1,23); cprintf("yh 		= %1.4f",yh);
		gotoxy(1,24); cprintf("zh 		= %1.4f",zh);
		gotoxy(1,25); cprintf("heC 	= %1.4f",heC);
		gotoxy(1,26); cprintf("hY 		= %1.4f",hY);
		gotoxy(1,27); cprintf("hrad 	= %1.4f",hrad);
		gotoxy(1,28); cprintf("<<OTROS PARÁMETROS>>");
		gotoxy(1,29); cprintf("Nruns 	= %1.1f @1e6 fotones/ejecucion", Nruns);
		gotoxy(1,30); cprintf("dr 		= %1.4f cm",dr);
		gotoxy(1,31); cprintf("dz 		= %1.4f cm",dz);
		gotoxy(1,32); cprintf("---------------");
		//gotoxy(1,33); cprintf("Presionar cualquier tecla para iniciar programa-->>"); getch(); clrscr();
	}

	// Inciar los arreglos a ceros
	for (ir=1; ir<=NR; ir++) 
	{
		Jx[ir] = 0.0;
		Jf[ir] = 0.0;
		temp1[ir] = 0.0;
		for (iz=1; iz<=NR; iz++) 
		{
			Fx[iz][ir] = 0.0;
			Ff[iz][ir] = 0.0;
			temp2[iz][ir] = 0.0;
		}
	}

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/* * * * * * * * ESTIMACIÓN DE TIEMPO DE EJECUCIÓN * * * * * * * */
	gotoxy(1,35); cprintf("< < < ESTIMACION DEL TIEMPO DE EJECUCION > > >");
	/* EXCITACIÓN */
	// Inciar el temporizador para calcular el tiempo que tarda en ejecutarse el programa
	timeA = clock();
	PRINTOUT = 0;	// Quitar la impresión de fotones lanzados
	/* Llamada a la subrutina Monte Carlo */
	mcsub(muax, musx, gx, n1, n2,
		NR, NZ, dr, dz, 999,
		mcflag, xs, ys, zs, boundaryflag,
		radius, waist, zfocus,
		Jx, Fx, &S, &A, &E,
		PRINTOUT);         /* va a regresar Jx, Fx, S, A, E */
	// Finaliza el temporizador
	timeB = clock();
	// Calcular el tiempo de Minutos por fotón
	timeperEx = (timeB - timeA)/CLOCKS_PER_SEC/60/999;
	gotoxy(2,36); cprintf("Irradiacion: %5.3e min/EX foton", timeperEx);

	/* EMISIÓN */
	temp = NZ/2*dz; // zs es a medio camino
	// Inciar el temporizador para calcular el tiempo que tarda en ejecutarse el programa
	timeA = clock();
	PRINTOUT = 0;	// Quitar la impresión de fotones lanzados
	/* Llamada a la subrutina Monte Carlo */
	mcsub(muaf, musf, gf, n1, n2,
		NR, NZ, dr, dz, 999,
		2, 0, 0, temp, boundaryflag,
		radius, waist, zfocus,
		Jx, Fx, &S, &A, &E,        
		PRINTOUT);		/* va a regresar Jx, Fx, S, A, E */
	// Finaliza el temporizador
	timeB = clock();
	// Calcular el tiempo de Minutos por fotón
	timeperEm = (timeB - timeA)/CLOCKS_PER_SEC/60/999;
	gotoxy(2,37); cprintf("Emisión: %5.3e min/EM fotón", timeperEm);

	gotoxy(2,38); cprintf("\nTiempo total estimado para ejecucion = %5.2f min", \
		timeperEx*1e6*Nruns + timeperEm*NZ*NR*100 + timeperEm*1e6*Nruns); 
	gotoxy(1,40); cprintf("\nPresionar una tecla para iniciar->"); getch();
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */	
	
	//Limpiar la parte inferior de la pantalla
	cecran(0,36,79,40);
	
	/* * * * * * * * INICIO DE LA SIMULACIÓN * * * * * * * */
	/* EXCITACIÓN */
	// Cálculo del # de fotones requeridos para la simulación por medio de Nruns * 10^6
	Nphotons = 1e6*Nruns;
	gotoxy(1,36); cprintf("EXCITACION\n");
	gotoxy(1,37); cprintf("Tiempo estimado = %5.2f min", timeperEx*Nphotons);
	PRINTOUT = 1;	// Encender el mostrar fotones
	/* Llamada a la subrutina Monte Carlo */
	mcsub(muax, musx, gx, n1, n2,
		NR, NZ, dr, dz, Nphotons,
		mcflag, xs, ys, zs, boundaryflag,
		radius, waist, zfocus,
		Jx, Fx, &S, &A, &E,
		PRINTOUT);		/* va a regresar Jx, Fx, S, A, E */
	/* SALVAR ESTA PARTE (excitación) */
	Nfilex = (Nfile*100) + 1; // mcOUTx01 = excitación
	// Salvar a "mcOUTi.dat", donde i = NFile
	SaveFile(Nfilex, Jx, Fx, S, A, E,
			muax, musx, gx, n1, n2,
			mcflag, radius, waist, xs, ys, zs,
			NR, NZ, dr, dz, Nphotons);
	//gotoxy(,); cprintf("------------------------------------------------------\n");
	finish_time1 = clock();
	gotoxy(1,38); cprintf("------------------------------------------------------");
	gotoxy(1,39); cprintf("Tiempo transcurrido para la excitacion = %5.3f min",
	(double)(finish_time1-start_time)/CLOCKS_PER_SEC/60);
	now = time(NULL);
	gotoxy(1,40); cprintf("%s\n", ctime(&now)); gotoxy(1,41); cprintf("-->"); getch();

	//Limpiar la parte inferior de la pantalla
	cecran(0,36,79,40);
	
	/* FLUORESCENCIA DE FONDO */
	Nphotons = 100;  // fotones lanzados por iso-punto en el fondo
	gotoxy(1,36); cprintf("FLUORESCENCIA DE FONDO");
	gotoxy(1,37); cprintf("Tiempo estimado = %5.2f min", timeperEm*NZ*NR*Nphotons);
	/*
		Acumular la fluorescencia debido a cada fuente "recipiente"
		ponderada por la fuerza de excitación absorbida en cada 
		fuente "recipiente", Fx[iz][ir]. No incluye los últimos "recipientes"
		[NZ][NR] los cuales están desbordados
	*/
	temp = 0.0; // Contar el número total de fotones lanzados
	for (ir=1; ir<NR; ir++) 
	{
		temp3 = 0.0; // Número total de fotones lanzados en la fila [ir]
		temp4 = eC*Y*2*PI*(ir - 0.5)*dr*dr*dz; // Conversión de fluorescencia

		for (iz=1; iz<NZ; iz++) 
		{
			temp += Nphotons;
			temp3 += Nphotons;
		
			// Establece para lanzar como fuente puntual isotrópica en el "recipiente"
			r2 = ir*dr;
			r1 = (ir-1)*dr;
			r = 2.0/3*(r2*r2 + r2*r1 + r1*r1)/(r2 + r1);
			xs = r;
			ys = 0;
			zs = (iz - 0.5)*dz;
			mcflag = 2; // Fuente puntual isotrópica de fluorescencia (IMPORTANTE!)
			PRINTOUT = 0; // Desactivar el imprimir fotones
			/* Llamada a la subrutina Monte Carlo */
			mcsub(muaf, musf, gf, n1, n2,
				NR, NZ, dr, dz, Nphotons,
				mcflag, xs, ys, zs, boundaryflag,
				radius, waist, zfocus,
				temp1, temp2, &S, &A, &E,
				PRINTOUT);		/* va a regresar Jx, Fx, S, A, E */
					
			/* Acumular los resultados Monte Carlo */
			for (iir=1; iir<=NR; iir++) 
			{
				Jf[iir] += temp1[iir]*Fx[iz][ir]*temp4;
				for (iiz=1; iiz<=NZ; iiz++)
					Ff[iiz][iir] += temp2[iiz][iir]*Fx[iz][ir]*temp4;
			} /* Finalizar iir */
		} /* Finalizar iz */
		
		/* Imprimir el progreso para el usuario */
		if (ir < 10) 
		{
			gotoxy(1,38); cprintf("%1.0f Fotones de fluorescencia \t@ ir = %ld \t total = %1.0f",temp3,ir, temp);
		}
		else if (fmod((double)ir,10)==0) 
		{
			gotoxy(1,38); cprintf("%1.0f Fotones de fluorescencia \t@ ir = %ld \t total = %1.0f",temp3,ir, temp);
		}
	} /* Fin ir */

	gotoxy(1,38); cprintf("%1.0f Fotones de fluorescencia \t@ ir = %ld \t total = %1.0f", temp3,ir, temp);
	gotoxy(1,39); cprintf("%1.0f Total de Fotones de fluorescencia",temp);
	finish_time2 = clock();
	gotoxy(1,40); cprintf("------------------------------------------------------");
	gotoxy(1,41); cprintf("Tiempo utilizado para la fluorescencia de fondo = %5.3f min",
	(double)(finish_time2-finish_time1)/CLOCKS_PER_SEC/60);
	gotoxy(1,42); cprintf("-->"); getch();
	/* SALVAR LA FLUORESCENCIA DE FONDO */
	Nfilex = (Nfile*100) + 2; // mcOUTx02 = fluorescencia de fondo
	SaveFile(Nfilex, Jf, Ff, S, A, E,       // salvar a "mcOUTi.dat", i = Nfile
			muaf, musf, gf, n1, n2, 
			mcflag, radius, waist, xs, ys, zs,
			NR, NZ, dr, dz, Nphotons);  
	
	//Limpiar la parte inferior de la pantalla
	cecran(0,36,79,42);
			
	/* FLUORESCENCIA HETEROGENEA */
	Nphotons = Nruns*1e6;
	gotoxy(1,36); cprintf("HETEROGENEIDAD FLUORESCENTE");
	gotoxy(1,37); cprintf("Tiempo estimado = %5.3f min", timeperEm*Nphotons);
	/* 
		Heterogeneidad fluorescente en (xh,yh,zh)
		como una esfera pequeña con un radio específico.
		Notar que los resultados son Jf(x), F(z,x) en el plano y=0.
		Usualmente se dejará yh = 0.
		Lanzar en xs = 0, ys = 0, zs = zs 
	*/
	/* Llamada a la subrutina Monte Carlo -> Respuesta fluorescente al impulso */
	xs = 0; ys = 0; zs = zh;
	mcflag = 2; /* Fuente puntual isotrópica */
	PRINTOUT = 1;
	/* Llamada a la subrutina Monte Carlo */
	mcsub(muaf, musf, gf, n1, n2,
		NR, NZ, dr, dz, Nphotons,
		mcflag, xs, ys, zs, boundaryflag,
		radius, waist, zfocus,
		temp1, temp2, &S, &A, &E,
		PRINTOUT);			/* va a regresar Jx, Fx, S, A, E */

	// Lado z positivo de la respuesta
	/* Inicializar los arreglos Jf[ ] & Ff[ ][ ] */
	for (ir=1; ir<=NR; ir++) 
	{
		Jf[ir] = 0.0;
		for (iz=1; iz<=NR; iz++)
			Ff[iz][ir] = 0.0;
	}
	/*
		Convolucionar la respuesta al impulso contra la fuente de fluorescencia en (xh,yh,zh).
		Ponderado por Fx[iz][ir]*4/3*PI*hrad*hrad*hrad*heC*hY
		el cual es la potencia de fluorescencia de heterogeneidad [W/W]. 
	*/
	temp4 = 4.0/3*PI*hrad*hrad*hrad*heC*hY;
	ir = (long)(sqrt( xh*xh + yh*yh )/dr) + 1; // ir, origen heterog
	iz = (long)(zh/dz) + 1; // iz, para Fx[iz][ir]
	for (iir=1; iir<=NR; iir++) 
	{ /* for Jf[iir], Ff[iiz][ir] */
		r2 = iir*dr; // Posición radial del observador radial
		//r1 = (iir-1)*dr;
		//r = 2.0/3*(r2*r2 + r2*r1 + r1*r1)/(r2 + r1);
		r = sqrt( (r2 - xh)*(r2 - xh) + yh*yh ); /* heterog-observador */
		ii = (long)(r/dr) + 1; /* for temp1[ii], temp2[iz][ii] */
		if (ii > NR) ii = NR;
		Jf[iir] += temp1[ii]*Fx[iz][ir]*temp4;
		for (iiz=1; iiz<=NZ; iiz++) /* for Ff[iiz][ir], temp2[iz][ii] */
			Ff[iiz][iir] += temp2[iiz][ii]*Fx[iz][ir]*temp4;
	} /* finalizar iir */

	/* SALVAR FLUORESCENCIA HETEROGENEA */
	Nfilex = (Nfile*100) + 3; // mcOUTx03 = lado positivo z de la fluorescencia heterogenea
	SaveFile(Nfilex, Jf, Ff, S, A, E,  // salvar a "mcOUTi.dat", i = Nfile
			muaf, musf, gf, n1, n2, 
			mcflag, radius, waist, xs, ys, zs,
			NR, NZ, dr, dz, Nphotons);  

	// Lado negativo en z de la respuesta
	/* Inicializar los arreglos Jf[ ] & Ff[ ][ ] */
	for (ir=1; ir<=NR; ir++) 
	{
		Jf[ir] = 0.0;
		for (iz=1; iz<=NR; iz++)
			Ff[iz][ir] = 0.0;
	}
	/*
		Convolucionar la respuesta al impulso contra la fuente de fluorescencia en (xh,yh,zh).
		Ponderado por Fx[iz][ir]*4/3*PI*hrad*hrad*hrad*heC*hY
		el cual es la potencia de fluorescencia de heterogeneidad [W/W]. 
	*/
	temp4 = 4.0/3*PI*hrad*hrad*hrad*heC*hY;
	ir = (long)(sqrt( xh*xh + yh*yh )/dr) + 1; // ir, origen heterog
	iz = (long)(zh/dz) + 1; /* iz, for Fx[iz][ir] */
	for (iir=1; iir<=NR; iir++) { /* for Jf[iir], Ff[iiz][ir] */
		r2 = iir*dr; // Posición radial del observador radial
		//r1 = (iir-1)*dr;
		//r = 2.0/3*(r2*r2 + r2*r1 + r1*r1)/(r2 + r1);
		r = sqrt( (r2 + xh)*(r2 + xh) + yh*yh ); /* heterog-observador */
		ii = (long)(r/dr) + 1; /* for temp1[ii], temp2[iz][ii] */
		if (ii > NR) ii = NR;
		Jf[iir] += temp1[ii]*Fx[iz][ir]*temp4;
		for (iiz=1; iiz<=NZ; iiz++) /* for Ff[iiz][ir], temp2[iz][ii] */
			Ff[iiz][iir] += temp2[iiz][ii]*Fx[iz][ir]*temp4;
	} /* finalizar iir */

	/* SALVAR LA FLUORESCENCIA HETEROGENA */
	Nfile = (Nfile*100) + 4; // mcOUTx04 = lado negativo z de la fluorescencia heterogena
	SaveFile(Nfile, Jf, Ff, S, A, E,  // salvar a "mcOUTi.dat", i = Nfile
			muaf, musf, gf, n1, n2, 
			mcflag, radius, waist, xs, ys, zs,
			NR, NZ, dr, dz, Nphotons);  

	finish_time3 = clock();
	gotoxy(1,38); cprintf("Tiempo utilizado para la fluorescencia heterogena = %5.3f min", \
		(double)(finish_time3-finish_time2)/CLOCKS_PER_SEC/60);
	gotoxy(1,39); cprintf("------------------------------------------------------\n");
	gotoxy(1,40); cprintf("------------------------------------------------------\n");
	now = time(NULL);
	gotoxy(1,41); cprintf("%s", ctime(&now));
	gotoxy(1,42); cprintf("Tiempo estimado total = %5.2f min", \
		timeperEx*1e6*Nruns + timeperEm*NZ*NR*100 + timeperEm*1e6*Nruns);
	gotoxy(1,43); cprintf("Tiempo utilizado = %5.2f min", \
		(double)(finish_time3-start_time)/CLOCKS_PER_SEC/60);
	gotoxy(1,44); cprintf("-->"); getch();

	// Limpiar las matrices utilizadas
	FreeVector(Jx, 1, BINS);
	FreeVector(Jf, 1, BINS);
	FreeVector(temp1, 1, BINS);
	FreeMatrix(Fx, 1, BINS, 1, BINS);
	FreeMatrix(Ff, 1, BINS, 1, BINS);
	FreeMatrix(temp2, 1, BINS, 1, BINS);

}

/* * * * * * * * DECLARACION DE FUNCIONES * * * * * * * */
//Función limpieza de un pedazo de pantalla
void cecran(int xi, int yi, int xf, int yf)
{
	int kx, qu;		//El contador de siempre mas uno más
	
	//Limpieza de la parte inferior de la pantalla
	for(kx=yi; kx <= yf; kx++)
	{
		for(qu=xi; qu <= xf; qu++)
		{
			gotoxy(qu,kx); cprintf(" ");
		}
	}
}