/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * mcsubLIB.h
 *
 * Archivo cabecera donde se declaran las funciones
 * que se describen en mcsub.c, usadas por el programa
 * Contiene las subrutinas:
 *		mcsub()
 *		RFresnel()
 *		SaveFile()
 *		RandomGen()
 *	y las rutinas de distribuci�n de memoria:
 *		nerror()
 *		*AllocVector()
 *		**AllocMatrix()
 *		FreeVector()
 *		FreeMatrix()
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
/* Librerias usadas */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
 
 /* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * FUNCIONES PROTOTIPO
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
/*
 * Subrutina Monte Carlo mcsub()
 *	Propieades del tejido:
 *			mua       = Coeficiente de absorci�n [cm^-1]
 *			mus       = Coeficiente de esparcimiento [cm^-1]
 *			g         = Coeficiente de anisotrop�a
 *			n1        = �ndice de refracci�n del tejido
 *			n2        = �ndice de refracci�n del medio externo
 *	Caracter�sticas del haz incidente:
 *			mcflag:   Selecci�n del tipo de haz incidente
                      0 = Colimado uniforme, 1 = Gaussiano, 2 = Puntual isotr�pico
 *		Posibles par�metros de las fuentes
 *			xs,ys,zs  = Posici�n de la fuente puntual isotr�pica (si mcflag = 2)
 *			boundaryflag = Es 1 si la frontera es aire/tejido , = 0 si el medio es semi infinto
 *			radius    = Radio del haz (si es Gaussiano esto es 1/e del ancho) (si mcflag < 2)
 *			waist     = 1/e Radio del foco (si es Gaussiano (si mcflag = 1)
 *			zfocus    = Profundidad del foco (si es Gaussiano (si mcflag = 1)
 *	Salida:
 *			J[ir]     = Vector de flujo de salida [cm^-2] contra la posici�n radial
 *			F[iz][ir] = Matriz del ritmo de fluencia normalizado [cm^-2] = [W/cm2 por W]
 *			Sptr      = Apuntador a S, reflectancia especular
 *			Aptr      = Apuntador a A, Fracci�n total absorbida
 *			Eptr      = Apuntador a E, Fracci�n total que escapo
 */
void mcsub(double mua, double mus, double g, double n1, double n2, 
           long NR, long NZ, double dr, double dz, double Nphotons,
           int mcflag, double xs, double ys, double zs, int boundaryflag,
           double radius, double waist, double zfocus,
           double *J, double **F, double *Sptr, double *Aptr, double *Eptr,
           short PRINTOUT);

/* 
	Calcula la reflectancia interna en la interfaz tejido/aire, se requiere:
	n1		= �ndice de refracci�n incidente
	n2		= �ndice de refracci�n transmisi�n
	ca1		= Coseno del �ngulo incidente a1, 0� < a1 < 90�
	ca2_Ptr	= Puntero al coseno del �ngulo de transmisi�n a2 > 0
 */ 
double RFresnel(double n1, double n2, double ca1, double *ca2_Ptr);

/* 
 * Salvar el archivo de salida "mcOUTi.dat" donde i = Narchivo.
 * Los par�metros de entrada son salvados, as� como la superficie de escape R(r)
 * y la distribuci�n del ritmo de fluencia F(z,r)
 * Salvar los resultados a archivos
 *   nombrados "Ji.dat" & "Fi.dat" donde i = Narchivo.
 * Salvar "Ji.dat" en el siguiente formato:
 *   Salvar r[ir] valores en la primera columna, (1:NR,1) = (filas,cols).
 *   Salvar Ji[ir] valores en la segunda columna, (1:NR,2) = (filas,cols).
 *   Como siempre, la �ltima fila esta desbordada por cuestiones de programaci�n
 * Salvar "Fi.dat" en el siguiente formato:
 *   El primer elemento (1,1) esta relleno con zeros y es ignorado.
 *   Salvar los valores z[iz] en la primera columan, (2:NZ,1) = (filas,cols).
 *   Salvar los valores r[ir] en la primera fila, (1,2:NZ) = (filas,cols).
 *   Salvar Fi[iz][ir] en (2:NZ+1, 2:NR+1).
 *   Las �ltimas filas y columans est�n desbordadas por proeblemas en la programaci�n y c�lculo.
 * Salvar "mcSAEi.dat" como 3 valores delimitados por un tabulador en una fila = [S A E].
 */
void SaveFile(int Nfile, double *J, double **F, double S, double A, double E, 
	double mua, double mus, double g, double n1, double n2, 
	short mcflag, double radius, double waist, double xs, double ys, double zs, 
	short NR, short NZ, double dr, double dz, double Nphotons);

/* Generador de n�meros aleatorios
   Iniciado por RandomGen(0,1,NULL)
   Usar como: double rnd = RandomGen(1,0,NULL) */
double RandomGen(char Type, long Seed, long *Status); 

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Rutinas para la asignaci�n de memoria usadas originalmente en el programa:
 * MCML ver. 1.0, 1992 L. V. Wang, S. L. Jacques,
 * Las cuales son versiones modificadas de Recetas Num�ricas en C 
* * * * * * * * * * * * * * * * * * * * * * * * * * */
void   nrerror(char error_text[]);
double *AllocVector(short nl, short nh);
double **AllocMatrix(short nrl,short nrh,short ncl,short nch);
void   FreeVector(double *v,short nl,short nh);
void   FreeMatrix(double **m,short nrl,short nrh,short ncl,short nch);

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * DECLARACI�N DE FUNCIONES
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

//Rutina de Monte Carlo
void mcsub(double mua, double mus, double g, double n1, double n2, 
           long NR, long NZ, double dr, double dz, double Nphotons,
           int mcflag, double xs, double ys, double zs, int boundaryflag,
           double radius, double waist, double zfocus,
           double * J, double ** F, double * Sptr, double * Aptr, double * Eptr,
           short PRINTOUT) 
{
	/* Constantes */
	double	PI          = 3.1415926;	//Definici�n de Pi
	short	ALIVE       = 1;            //Si el fot�n no ha terminado a�n
	short	DEAD        = 0;            //Si el fot�n esta a terminar
	double	THRESHOLD   = 0.0001;       //Usado en la ruleta para terminar el fot�n (umbral)
	double	CHANCE      = 0.1;          //Usado en la ruleta

	/* Par�metros variables */
	double	mut, albedo, absorb, rsp, Rsptot, Atot;
	double	rnd, xfocus, S, A, E;
	double	x,y,z, ux,uy,uz,uz1, uxx,uyy,uzz, s,r,W,temp;
	double	psi,costheta,sintheta,cospsi,sinpsi;
	long   	iphoton, ir, iz, CNT;
	short	photon_status;

	/*  INICIALIZACI�N  */
	RandomGen(0, -(int)time(NULL)%(1<<15), NULL); //Iniciar con una semilla = 1, � cualquier otro long int
	CNT = 0;
	mut    = mua + mus;
	albedo = mus/mut;
	Rsptot = 0.0; //Acumular la reflectancia especular por fot�n
	Atot   = 0.0; //Acumular el peso de los fotones absorbidos
	
	/* Inicializar los arreglos a cero */
	for (ir=1; ir<=NR; ir++) 
	{
		J[ir] = 0.0;
		for (iz=1; iz<=NZ; iz++)
			F[iz][ir] = 0.0;
  	}

	/*
	==============================================================
	======================= CORRER N fotones =====================
	Lanzar N fotones, inicializar cada uno antes de la propagaci�n.
	==============================================================
	*/
	for (iphoton=1; iphoton<=Nphotons; iphoton++) 
	{
		/* Imprimir el progreso si mcflag < 3 */
		temp = (double)iphoton;
		if ((PRINTOUT == 1) & (mcflag < 3) & (temp >= 100)) 
		{
			gotoxy(50,4); cprintf("Fotones: ");
			if (temp<1000) 
			{
				if (fmod(temp,100)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
			if (temp<10000) 
			{
				if (fmod(temp,1000)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
			else if (temp<100000) 
			{
				if (fmod(temp,10000)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
			else if (temp<1000000) 
			{
				if (fmod(temp,100000)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
			else if (temp<10000000) 
			{
				if (fmod(temp,1000000)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
			else if (temp<100000000) 
			{
				if (fmod(temp,10000000)==0){ gotoxy(59,4); cprintf("%1.0f",temp);}
			}
		}
	
		/* 
			LANZAR FOTONES
		Inicializar la posici�n de los fotones y la trayectoria.
		*/

		//HAZ COLIMADO UNIFORME INCIDENTE EN z=zs
		if (mcflag == 0) 
		{
			/*
			* Lanzar en (r,zs) = (radius*sqrt(rnd), 0).
			* Debido a la geometr�a cilindrica, la posici�n radial de lanzamiento es
			* asignada a x mientras y=0
			* radius = radio del haz uniforme. */
			/* Posici�n inicial */
			rnd = RandomGen(1,0,NULL); 
			x = radius*sqrt(rnd); 
			y = 0;
			z = zs;
			/* Trayectoria inicial en cosenos directores */
			ux = 0;
			uy = 0;
			uz = 1;  
			/* Reflectancia especular */
			temp   = n1/n2; // Valor temporal de la relaci�n entre los indices de refracci�n interno/externo
			temp   = (1.0 - temp)/(1.0 + temp); //calculo de la reflectancia especular
			rsp    = temp*temp; // reflectancia especular en la fronteras
		}
		//HAZ DEL TIPO GAUSSIANO
		else if (mcflag == 1) 
		{
			/* Lanzar en (r,z) = (radius*sqrt(-log(rnd)), 0).
			* Debido a la simetr�a cilindrica, la posici�n radial de lanzamiento es
			* asignada a x mientras y=0. 
			* radius = 1/e radio del haz Gaussiano en la superficie
			* ancho  = 1/e radio del foco Gaussiano
			* zfocus = profundidad del punto focal. */
			/* Posici�n inicial */
			while ((rnd = RandomGen(1,0,NULL)) <= 0.0); // evitar que rnd = 0
			x = radius*sqrt(-log(rnd));
			y = 0.0;
			z = 0.0;
			/* Trayectoria inicial en cosenos directores */
			/* Debido a la simetr�a cilindrica, la trayectoria radial de lanzamiento es
			* asignada a ux & uz mientras uy=0. */
			while ((rnd = RandomGen(1,0,NULL)) <= 0.0); // evitar que rnd = 0
			xfocus = waist*sqrt(-log(rnd));
			temp = sqrt((x - xfocus)*(x - xfocus) + zfocus*zfocus);
			sintheta = -(x - xfocus)/temp;
			costheta = zfocus/temp;
			ux = sintheta;
			uy = 0.0;
			uz = costheta;
			/* Reflexi�n especular y refracci�n, la parte anterior de los c�lculos se realiz�
				sin considerar el "mismatch" de los �ndices de refracci�n*/
			rsp = RFresnel(n2, n1, costheta, &uz); /* nuevo uz calculado a partir de la funci�n RFresnel */
			ux  = sqrt(1.0 - uz*uz); /* nuevo ux */
			//Cada fot�n es lanzado en un diferente �ngulo y experimenta por lo tanto una diferente rsp
			//La refracci�n en la frontera "mismatched" donde n1 > n2 causa que el punto focal se mueva hacia dentro del medio
		}
		/* FUENTE PUNTUAL ISPTR�PICA EN LA POSICI�N xs, ys, zs */
		else if  (mcflag == 2) 
		{
			/* Posici�n incial */
			x = xs;
			y = ys;
			z = zs;
			/* Trayectoria inicial en cosenos directores */
			costheta = 1.0 - 2.0*RandomGen(1,0,NULL); // costheta esta entre -1 y 1
			sintheta = sqrt(1.0 - costheta*costheta); // sintheta es siempre positivo
			psi = 2.0*PI*RandomGen(1,0,NULL);	// el valor de psi esta entre 0 y 2PI
			cospsi = cos(psi);
			/* Se calcula sin(psi) de acuerdo a si es o no menor a PI
			   adem�s se usa sqrt() ya que es m�s r�pida que usar sin() [esto en C]
			*/
			if (psi < PI)
				sinpsi = sqrt(1.0 - cospsi*cospsi); 
			else
				sinpsi = -sqrt(1.0 - cospsi*cospsi);
			ux = sintheta*cospsi;
			uy = sintheta*sinpsi;
			/* La reflexi�n especular en este caso no existe pues se encuentra la fuente dentro del medio */
			uz = costheta;
			rsp = 0.0;
		}
		else 
		{
			printf("Elegir fuente entre 0 & 2\n");
		}

		W             = 1.0 - rsp;  // Colocar el peso inicial del fot�n
		Rsptot       += rsp; // Acumular la reflectancia especular por fot�n
		photon_status = ALIVE; //El estado inicial del fot�n es VIVO

		/* HOP_DROP_SPIN_CHECK
		   Propagar un fot�n hasta que desaparezca como se determina por la ROULETTE
		   Cada fot�n es repetidamente permitido a saltar (HOP) a una nueva posici�n
		   perder/tirar (DROP) algo de peso debido a la absorci�n
		   girar (SPIN) en una nueva trayectoria debido al esparcimiento 
		   y revisar (CHECK) donde el fot�n debe desaparecer debido a la ROULETTE
		*/
		do 
		{
			/**** HOP
			   Tomar un paso a la nueva posici�n
			   s = tama�o del paso
			   ux, uy, uz son los cosenos de la trayectoria actual del fot�n
			*****/
			while ((rnd = RandomGen(1,0,NULL)) <= 0.0);   /* evitar que rnd = 0 */
			s = -log(rnd)/mut;   /* Tama�o del paso.  Nota: log() es base e */
			/* Actualizar posiciones. */
			x += s*ux;
			y += s*uy;
			z += s*uz;
		
			/* �El fot�n escap� en la superficie? ... z <= 0? <SECCI�N ESCAPE FOT�N>*/
			if ( (boundaryflag == 1) & (z <= 0)) 
			{
				rnd = RandomGen(1,0,NULL);
				/* Revisar la reflectancia de Fresnel en la frontera de la superficie */
				if (rnd > RFresnel(n1, n2, -uz, &uz1))
				{  
					/* Photon escapes at external angle, uz1 = cos(angle) */
					/* Regresar a la posici�n original */
					x -= s*ux;
					y -= s*uy;
					z -= s*uz;
					s  = fabs(z/uz); // Calcular el tama�o del paso para alcanzar la superficie
					x += s*ux;       // Paso parcial para alcanzar la superficie
					y += s*uy;
					r = sqrt(x*x + y*y);   // Encontrar la posici�n radial r
					ir = (long)(r/dr) + 1; // Redondear a 1 <= ir */
					if (ir > NR) ir = NR;  // ir = NR es el "recipiente" desbordado (�ltimo)
					J[ir] += W;      // Incrementar el flujo de escape...
					photon_status = DEAD;	//... y el fot�n por lo tanto desaparece
				}
				else 
				{
					z = -z;   // Reflexi�n interna total
					uz = -uz; // El fot�n regresa a las andadas
				}
			}

			//Si el fot�n despu�s de todo permance VIVO
			if (photon_status  == ALIVE) 
			{
				/* HOP_DROP_SPIN_CHECK
				Propagar un fot�n hasta que desaparezca como se determina por la ROULETTE
				Cada fot�n es repetidamente permitido a saltar (HOP) a una nueva posici�n
				perder/tirar (DROP) algo de peso debido a la absorci�n
				girar (SPIN) en una nueva trayectoria debido al esparcimiento 
				y revisar (CHECK) donde el fot�n debe desaparecer debido a la ROULETTE
				*/

				/**** DROP
					Quitar el peso del fot�n (W) en el paquete local.
				*****/
				absorb = W*(1 - albedo);      /* Peso del fot�n absorbido en este paso */
				W -= absorb;                  /* decrementar WEIGHT por la cantidad absorbida */
				Atot += absorb;               /* acumular el peso absorbido del fot�n */
				/* Depositar la potencia en las coordenadas silindricas z,r */
				r  = sqrt(x*x + y*y);         /* Posici�n actual radial cilindrica */
				ir = (long)(r/dr) + 1;        /* redondear a 1 <= ir */
				iz = (long)(fabs(z)/dz) + 1;  /* redondear a 1 <= iz */
				if (ir >= NR) ir = NR;        /* el �ltimo "recipiente" es para desbordar */
				if (iz >= NZ) iz = NZ;        /* el �ltimo "recipiente" es para desbordar */
				F[iz][ir] += absorb;          /* DROP absorbe el peso del fot�n en un "recipiente" */

				/**** SPIN 
					El fot�n esparcido en la nueva trayectoria definido por theta y psi
					Theta es especificado por cos(theta), el cual es determinado
					basado en la funci�n de esparcimiento de Henyey-Greenstein
					Convertir theta y psi en cosenos ux, uy y uz
				*****/
				/* Muestreo para costheta 
					El �ngulo de deflexi�n describe como el fot�n es deflectado de su trayectoria actual. Un n�mero
					aleatorio (rnd) selecciona un elemento para el valor cos(theta), llamado costheta. Se calcula tmb
					el valor sintheta.
				*/
				rnd = RandomGen(1,0,NULL);
				if (g == 0.0)
					costheta = 2.0*rnd - 1.0;
				else if (g == 1.0)
					costheta = 1.0;
				else 
				{
					temp = (1.0 - g*g)/(1.0 - g + 2*g*rnd);
					costheta = (1.0 + g*g - temp*temp)/(2.0*g);
				}
				sintheta = sqrt(1.0 - costheta*costheta);/*sqrt faster than sin()*/

				/* Muestreo de psi 
					La deflexi�n se asume a ser dirigida con probabilidad igual en cualquier �ngulo azimutal (psi). 
					Un n�mero aleatorio (rnd) selecciona un valor para psi el cual es usado para generar valores para
					cos(psi) y sin(psi), los cuales son cospsi y sinpsi
				*/
				psi = 2.0*PI*RandomGen(1,0,NULL);
				cospsi = cos(psi);
				if (psi < PI)
					sinpsi = sqrt(1.0 - cospsi*cospsi); /* sqrt() es m�s r�pido que sin() */
				else
					sinpsi = -sqrt(1.0 - cospsi*cospsi);

				/* Nueva trayectoria. */
				if (1 - fabs(uz) <= 1.0e-12) 
				{  /* Si es cercano a la perpendicular se calcula uxx, uyy, uzz. */
					uxx = sintheta*cospsi;
					uyy = sintheta*sinpsi;
					uzz = costheta*((uz)>=0 ? 1:-1);
				} 
				else 
				{   
					/* Usualmente se usa esta opci�n */
					temp = sqrt(1.0 - uz*uz);
					uxx = sintheta*(ux*uz*cospsi - uy*sinpsi)/temp + ux*costheta;
					uyy = sintheta*(uy*uz*cospsi + ux*sinpsi)/temp + uy*costheta;
					uzz = -sintheta*cospsi*temp + uz*costheta;
				}

				/* Actfualizar trayectoria */
				ux = uxx;
				uy = uyy;
				uz = uzz;

				
				/**** CHECAR ROULETTE 
					Si el peso del fot�n esta por debajo del THRESHOLD, entonces se termina el fot�n usando la t�cnica de la ruleta
					El fot�n tiene la posibilidad (CHANCE) de tener su peso incrementado por el factor 1/CHANCE
					y la probabilidad de 1-CHANCE de terminar.
				*****/
				if (W < THRESHOLD)
				{
					rnd = RandomGen(1,0,NULL);
					if (rnd <= CHANCE) W /= CHANCE;
					else photon_status = DEAD;
				}

			}
		}	/* finalizar la etapa de STEP_CHECK_HOP_SPIN */ 
		while (photon_status == ALIVE);
		//Cuanto lleva el proceso
		gotoxy(50,5); cprintf("Completado: %3.2f %", 100*(iphoton / Nphotons) );
		
		//El fot�n "desaparece" y entonces se lanza un nuevo fot�n
	} 

	/*
	 NORMALIZAR 
		J[ir]		densidad de flujo de escape [W/cm^2 por W incidente] 
					donde bin = 2.0*PI*r[ir]*dr [cm^2].
		F[iz][ir]  	ritmo de fluencia [W/cm^2 por W incidente] 
					donde bin = 2.0*PI*r[ir]*dr*dz [cm^3].
	*/
	temp = 0.0;
	//Recorrer todos los "recipientes" radiales
	for (ir=1; ir<=NR; ir++) 
	{
		r = (ir - 0.5)*dr;
		temp += J[ir];    /* acumular el total de peso de fotones que escap� */
		J[ir] /= 2.0*PI*r*dr*Nphotons;                /* densidad de flujo */
		for (iz=1; iz<=NZ; iz++)
			F[iz][ir] /= 2.0*PI*r*dr*dz*Nphotons*mua; /* ritmo de fluencia */
	}

	//Almacenar todos estos datos
	//Reflexi�ne specular
	*Sptr = S = Rsptot/Nphotons;
	//Potencia absorbida
	*Aptr = A = Atot/Nphotons;
	//Potencia que escap�
	*Eptr = E = temp/Nphotons;

}

/***********************************************************
 *	REFLECTANCIA DE FRESNEL
 * Calcula la reflectancia cuando el fot�n pasa de un medio 1
 * a un medio 2 con indices de refracci�n n1, n2. El �ngulo 
 * incidenten a1 est� especificado por el valor del coseno ca1 = cos(a1).
 * El programa regresa el valor para el �ngulo transmitido a1 como el
 * valor en *ca2_Ptr = cos(a2).
 ****/
double RFresnel(double n1, double n2, double ca1,double *ca2_Ptr)
{
	double r;

	/* Indices de refracci�n "Matched" */
	if(n1==n2) 
	{ 
		*ca2_Ptr = ca1;
		r = 0.0;
	}
	/* Incidencia normal */
	else if(ca1>(1.0 - 1.0e-12)) 
	{
		*ca2_Ptr = ca1;
		r = (n2-n1)/(n2+n1);
		r *= r;
	}
	/* Caso de incidencia muy inclinada (quasi perpendicular) */
	else if(ca1< 1.0e-6)  
	{
		*ca2_Ptr = 0.0;
		r = 1.0;
	}
	/* Caso general */
	else  
	{
		double sa1, sa2; 
		double ca2;      /* coseno del �ngulo de transmisi�n */
		sa1 = sqrt(1-ca1*ca1); 	//Seno del �ngulo de incidencia
		sa2 = n1*sa1/n2;		//Seno del �ngulo de transmisi�n
		//Si sa2 es mayor o igual a 1
		if(sa2>=1.0) 
		{	
			/* doble verificaci�n para la reflexi�n interna total. */
			*ca2_Ptr = 0.0;
			r = 1.0;
		}
		else 
		{
			double cap, cam;	// cosenos de la sum ap � difd am de los dos �ngulos:
								// ap = a1 + a2, am = a1 - a2. */
			double sap, sam;	/* senos */
			*ca2_Ptr = ca2 = sqrt(1-sa2*sa2);
			cap = ca1*ca2 - sa1*sa2; /* c+ = cc - ss. */
			cam = ca1*ca2 + sa1*sa2; /* c- = cc + ss. */
			sap = sa1*ca2 + ca1*sa2; /* s+ = sc + cs. */
			sam = sa1*ca2 - ca1*sa2; /* s- = sc - cs. */
			r = 0.5*sam*sam*(cam*cam+cap*cap)/(sap*sap*cam*cam); 
		}
	}
	return(r);
}

/***********************************************************
 * SALVAR LOS RESULTADOS A ARCHIVOS
***********************************************************/
void SaveFile(int Nfile, double *J, double **F, double S, double A, double E, 
	double mua, double mus, double g, double n1, double n2, 
	short mcflag, double radius, double waist, double xs, double ys, double zs, 
	short NR, short NZ, double dr, double dz, double Nphotons)
{
	double	r, z, r1, r2;
	long  	ir, iz;
	char	name[20];
	FILE*	target;

	/*Abrir el archivo para escritura*/
	sprintf(name, "mcOUT%d.dat", Nfile);
	target = fopen(name, "w");

	/* imprimir los resultados */
	fprintf(target, "%0.3e\tmua, coeficiente de absorcion [1/cm]\n", mua);
	fprintf(target, "%0.4f\tmus, coeficiente de esparcimiento [1/cm]\n", mus);
	fprintf(target, "%0.4f\tg, anisotropia [-]\n", g);
	fprintf(target, "%0.4f\tn1, indice de refracci�n del tejido\n", n1);
	fprintf(target, "%0.4f\tn2, indice de refracci�n del medio externo\n", n2);
	fprintf(target, "%d\tFuente\n", mcflag);
	fprintf(target, "%0.4f\tradius, radio del haz plano o radio 1/e del haz Gaussiano [cm]\n", radius);
	fprintf(target, "%0.4f\twaist, ancho de 1/e del foco [cm]\n", waist);
	fprintf(target, "%0.4f\txs, posicion en x de la fuente isotropica [cm]\n", xs);
	fprintf(target, "%0.4f\tys, posicion y\n", ys);
	fprintf(target, "%0.4f\tzs, posicion z\n", zs);
	fprintf(target, "%d\tNR\n", NR);
	fprintf(target, "%d\tNZ\n", NZ);
	fprintf(target, "%0.5f\tdr\n", dr);
	fprintf(target, "%0.5f\tdz\n", dz);
	fprintf(target, "%0.1e\tNfotones\n", Nphotons);

	/* imprimir los valores SAE */
	fprintf(target, "%1.6e\tSpecular reflectance\n", S);
	fprintf(target, "%1.6e\tAbsorbed fraction\n", A);
	fprintf(target, "%1.6e\tEscaping fraction\n", E);

	/* imprimir r[ir] a la fila */
	fprintf(target, "%0.1f", 0.0); /* ignorar el elemento superior izquierdo de la matriz */
	//Recorrer todos los "recipientes" radiales
	for (ir=1; ir<=NR; ir++) 
	{ 
		r2 = dr*ir;
		r1 = dr*(ir-1);
		r = 2.0/3*(r2*r2 + r2*r1 + r1*r1)/(r1 + r2);
		fprintf(target, "\t%1.5f", r);
	}
	fprintf(target, "\n");

	/* imprimir J[ir] a la siguiente fila */
	fprintf(target, "%0.1f", 0.0); /* ignorar el primer elemento de la segunda fila */
	//Recorrer todos los "recipientes" radiales
	for (ir=1; ir<=NR; ir++) 
	{
		fprintf(target, "\t%1.12e", J[ir]);
	}
	fprintf(target, "\n");

	/* imprimir z[iz], F[iz][ir] en las filas restantes */
	for (iz=1; iz<=NZ; iz++) 
	{
		z = (iz - 0.5)*dz; /* valores en z para la posici�n en profundidad en la primera columna */
		fprintf(target, "%1.5f", z);
		for (ir=1; ir<=NR; ir++)
			fprintf(target, "\t %1.6e", F[iz][ir]);
		fprintf(target, "\n");
	}
	fclose(target); //Cerrar el archivo de escritura
}


/**************************************************************************
 *	RandomGen
 *		Un generador aleatorio de n�meros que genera uniformemente
 *		n�meros aleatorios distribuidos entre 0 y 1 inclusive.
 *		El algortimo esta basado en:
 *      W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P.
 *      Flannery, "Numerical Recipes in C," Cambridge University
 *      Press, 2nd edition, (1992).
 *      &
 *      D.E. Knuth, "Seminumerical Algorithms," 2nd edition, vol. 2
 *      of "The Art of Computer Programming", Addison-Wesley, (1981).
 *
 *		Cuando el tipo es 0, colocar Seed como la semilla. Asegurar 0 < Seed < 320000.
 *		Cuando el tipo es 1, regresar el n�mero aleatorio.
 *		Cuando el tipo es 2, obtener el estado del generador.
 *		Cuando el tipo es 3, restaurar el estado del generador.
 *
 *		El estado del generador es representado por Status[0...56].
 *
 *		Asegurar que se incializa la semilla antes de obtener el n�mero aleatorio sino se
 *		obtienen resultados bien confusos
 *
 **************************************************************************/
 /*DEFINIR ALGUNOS TERMINOS*/
#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC 1.0E-9
double RandomGen(char Type, long Seed, long *Status)
{
	/* Variables locales */
	static long i1, i2, ma[56];   // ma[0] no se usa.
	long        mj, mk;
	short       i, ii;

	if (Type == 0) // Colocar la semilla
	{
		mj = MSEED - (Seed < 0 ? -Seed : Seed);
		mj %= MBIG;
		ma[55] = mj;
		mk = 1;
		for (i = 1; i <= 54; i++) 
		{
			ii = (21 * i) % 55;
			ma[ii] = mk;
			mk = mj - mk;
			if (mk < MZ)
				mk += MBIG;
			mj = ma[ii];
		}
		for (ii = 1; ii <= 4; ii++)
			for (i = 1; i <= 55; i++) 
			{
				ma[i] -= ma[1 + (i + 30) % 55];
				if (ma[i] < MZ)
					ma[i] += MBIG;
			}
		i1 = 0;
		i2 = 31;
	} 
	else if (Type == 1) // Obtener el n�mero
	{
		if (++i1 == 56)
			i1 = 1;
		if (++i2 == 56)
			i2 = 1;
		mj = ma[i1] - ma[i2];
		if (mj < MZ)
			mj += MBIG;
		ma[i1] = mj;
		return (mj * FAC);
	}
	else if (Type == 2) // Obtener el estado
	{
		for (i = 0; i < 55; i++)
			Status[i] = ma[i + 1];
		Status[55] = i1;
		Status[56] = i2;
	} 
	else if (Type == 3) // Restablecer el estado
	{
		for (i = 0; i < 55; i++)
			ma[i + 1] = Status[i];
		i1 = Status[55];
		i2 = Status[56];
	} 
	else
		puts("Parametros err�neos para RandomGen().");
	return (0);
} 
#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

/***********************************************************
 * ASIGNACI�N DE MEMORIA
 * REPORTAR MENSAJE DE ERROR a stderr, luego salir del programa
 * con la se�al 1.
 ***********************************************************/
void nrerror(char error_text[])
{
	//Mostrar en que fall� la asignaci�n de memoria
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...Salir del sistema...\n"); getch();
	exit(1);	//Salir del programa
}

/***********************************************************
 * ASIGNACI�N DE MEMORIA
 *  Lihong Wang para MCML version 1.0, 1992.
 *	ASIGNAR UN ARREGLO 1D con indice de n1 a nh inclusive
 *	Matriz original y vector de Numerical Recipes in C
 *	no iniciar los elementos a cero.
 *	Esto se be accomplished by the following functions. 
 ***********************************************************/
double *AllocVector(short nl, short nh)
{
	double *v;
	short i;
	// Asignaci�n de memoria de tama�o double x (nh-nl+1)
	v = (double *)malloc( (unsigned)(nh-nl+1)*sizeof(double) );
	//Si falla la asignaci�n de memoria, entonces mandar mensaje de error y salir
	if (!v) nrerror("Fallo 1 asignaci�n en vector()");
	v -= nl;		// Mover un espacio nl hacia atr�s
	for(i=nl;i<=nh;i++) v[i] = 0.0;	// Iniciar el vector a ceros
	return v;
}

/***********************************************************
 * ASIGNACI�N DE MEMORIA
 *	ASIGNAR UN ARREGLO 2D con �ndice de fila de nr1 a nrh
 *	inclusive, y un �ndice de columan de nc1 a nch inclusive.
 ***********************************************************/
double **AllocMatrix(short nrl,short nrh,short ncl,short nch)
{
	char *yt, *cn;
	short i,j;
	double **m;
	// Asignar un doble apuntador de tama�o double x (nrh-nrl+1)
	m=(double **) malloc( (unsigned)(nrh-nrl+1)*sizeof(double*) );
	// Mandar mensaje de error en asignaci�n de memoria
	if (!m) nrerror("Fallo asignaci�n 1 en matriz()");
	m -= nrl;		// Retroceder en nrl
	// Despu�s recorrer de nrl a nrh
	for(i=nrl;i<=nrh;i++) 
	{
		// Asignar memoria para cada m[i] de tama�o double x (nch-ncl+1)
		m[i]=(double *) malloc( (unsigned)(nch-ncl+1)*sizeof(double) );
		if (!m[i])		//Si no se pudo asignar memoria
		{
			itoa(i,cn,10);		//Convertir a string el n�mero de col i donde se detuvo
			// Mandar mensaje de error, con la col donde se produjo
			yt=strcat("Fallo 2 asignaci�n en matrix() ", cn);	
			nrerror(yt);
		}		
		m[i] -= ncl;
	}
	// Iniciar valores
	for(i=nrl;i<=nrh;i++)
		for(j=ncl;j<=nch;j++) 
			m[i][j] = 0.0;
	return m;
}

/***********************************************************
 * ASIGNACI�N DE MEMORIA
 * LIBERAR MEMORIA PARA UN ARREGLO 1D
 ***********************************************************/
void FreeVector(double *v,short nl,short nh)
{
	free((char*) (v+nl));
}

/***********************************************************
 * ASIGNACI�N DE MEMORIA
 *	LIBERAR MEMORIA PARA UNA MATRIZ 2D
 ***********************************************************/
void FreeMatrix(double **m,short nrl,short nrh, short ncl,short nch)
{
	short i;
	for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}