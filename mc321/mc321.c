/********************************************
 *  mc321.c 
 *
 * La simulación de MonteCarlo produce respuestas esféricas, cilindricas y planares para
 * una fuente ó puntual isotrópica ó colimada ó un haz finito
 * en un medio infinito homogéneo sin fronteras.
 * Este es un programa mínimo de MonteCarlo que puntua las distribuciones de fotones 
 * del tipo esférico, cilindrico y planar.
 *
 ********************************************/

/*LIBRERIAS*/ 
 //Se incluyen las librerias con las subrutinas necesarias para el programa
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <time.h>

/*DEFINICIONES*/
#define	PI          3.1415926	/* valor de pi */
#define	LIGHTSPEED	2.997925E10 /* velocidad de la luz en el vacío [cm/s] */
#define ALIVE       1   		/* Si el fotón no ha terminado */
#define DEAD        0    		/* Si el fotón ha terminado */
#define THRESHOLD   0.01		/* Usado en la ruleta */
#define CHANCE      0.1  		/* Usado en la ruleta */
#define COS90D      1.0E-6		/* Coseno de 90° */
/* Si el cos(theta) <= COS90D, theta >= PI/2 - 1e-6 rad. */     
#define ONE_MINUS_COSZERO 1.0E-12
/* Si 1-cos(theta) <= ONE_MINUS_COSZERO, fabs(theta) <= 1e-6 rad. */
/* Si 1+cos(theta) <= ONE_MINUS_COSZERO, fabs(PI-theta) <= 1e-6 rad. */
#define SIGN(x)           ((x)>=0 ? 1:-1)
/* Aqui se inicia la semilla para la subrutina generadora de números aleatorios */
#define InitRandomGen    (double) RandomGen(0, 1, NULL)
/* Inicia la semilla para el generador de números alaeatorios */
#define RandomNum        (double) RandomGen(1, 0, NULL)

/* DECLARACION DE FUNCIONES */
//Generador aleatorio de números
double RandomGen(char Type, long Seed, long *Status);

/* FUNCION PRINCIPAL */
void main() 
{
	/* Parámetros de propagación */
	double	x, y, z;    /* Posición del fotón */
	double	ux, uy, uz; /* Trayectoria del fotón en cosenos directores */
	double  uxx, uyy, uzz;	/* Valores temporales usados durante SPIN */
	double	s;          /* tamaño del paso s = -log(RND)/mus [cm] */
	double	costheta;   /* cos(theta) */
	double  sintheta;   /* sin(theta) */
	double	cospsi;     /* cos(psi) */
	double  sinpsi;     /* sin(psi) */
	double	psi;        /* ángulo azimutal */
	double	i_photon;   /* Fotón en uso */
	double	W;          /* Peso del fotón */
	double	absorb;     /* peso depositado o perdido en el paso debido a la absorción */
	short   photon_status;  /* flag = ALIVE=1 or DEAD=0 */

	/* otras variables */
	double	Csph[2001];  /* esférico   	concentración de fotones CC[ir=0..100] */
	double	Ccyl[2001];  /* cilindrico 	concentración de fotones CC[ir=0..100] */
	double	Cpla[2001];  /* plano      	concentración de fotones CC[ir=0..100] */
	double	Fsph;       /* Fluencia en una cáscara esférica */
	double	Fcyl;       /* Fluencia en una cáscara cilindrica */
	double	Fpla;       /* Fluencia en una cáscara plana */
	double	mua;        /* Coeficiente de absorción [cm^-1] */
	double	mus;        /* coeficiente de esparcimiento [cm^-1] */
	double	g;          /* anisotropia [-] */
	double	albedo;     /* albedo del tejido */
	double	nt;         /* índice de refracción del tejido */
	double	Nphotons;   /* Número de fotones en la simulación */
	short	NR;         /* Número de posiciones radiales  */
	double	radial_size;  /* Tamaño máximo radial */
	double	r;          /* Posición radial */
	double  dr;         /* Tamaño radial del paquete */
	short	ir;         /* indice a la posición radial */
	double  shellvolume;  /* volumen del cascarón en la posición radial r */
	double 	CNT;        /* conteo total del peso de fotones sumados sobre todas los paquetes  */

	/* Variables extras */
	double  rnd;        /* valor aleatorio asigando 0-1 */
	short	i, j;       /* indices */
	double	u, temp;    /* más variables */
	int sourcet;		/* tipo de fuente */
	char *nst;			/* nombre de la fuente */
	FILE*	target;     /* apuntador al archivo de salida */
	char *namf;			/* nombre del archivo de salida */

	/* Algunos parámetros extras para una fuente de diámetro finito */
	double phi;			/* Posición ángular del lanzamiento */
	double a;			/* Radio del haz circular */

	/**** ENTRADA
	   Entrada de propiedades opticas
	   Entrada del tamaño del paquete y el arreglo
	   Entrada del número de fotones
	*****/

	clrscr();
	/* El usuario añade las características ópticas del tejido */
	printf("Número de fotones a lanzar N= "); scanf("%lf", &Nphotons);
	printf("[Coef. de absorcion]     æa = "); scanf("%lf", &mua);
	printf("[Coef. de esparcimiento] æs = "); scanf("%lf", &mus);
	printf("[Coef. de anisotropia]    g = "); scanf("%lf", &g);
	printf("[Indice de refracción]   nt = "); scanf("%lf", &nt);
	printf("Tipo de Fuente: \n");
	printf("(1)Puntual isotrópica\n");
	printf("(2)Colimada\n");
	printf("(3)Haz finito\n");
	printf("-->"); scanf("%d",&sourcet); fflush(stdin);
	if(sourcet==1) strcpy(nst,"Fuente puntual isotrópica");
	else if(sourcet==2) strcpy(nst,"Fuente colimada");
	else if(sourcet==3) strcpy(nst,"Haz finito");
	printf("\n%s\n", nst);

//	mua         = 1.0;     	/* [cm^-1] */
//	mus         = 0.0;  	/* [cm^-1] */
//	g           = 0.90;  	/* [adimensional] */
//	nt          = 1.33;		/* [adimensional] */
//	Nphotons    = 20000; 	/* Colocar el número de fotones en la simulación a lanzarse */
	radial_size = 5.0;   	/* cm, intervalo total sobre el cual el paquete se extiende */
	NR          = 2000;	 	/* Colocar el número de paquetes */
	dr          = radial_size/NR;  	/* cm */
	albedo      = mus/(mus + mua);	/* albedo */


	/**** INICIALIZACIÓN
	*****/
	i_photon = 0;	/* El fotón en uso */
	//InitRandomGen;	/* Comenzar el generador de números aleatorios */
	randomize();
	RandomGen(0,random(100),NULL);
	//Posición radial
	for (ir=0; ir<=NR; ir++)
	{
	   Csph[ir] = 0;
	   Ccyl[ir] = 0;
	   Cpla[ir] = 0;
	}
	//Colocar el contador de cuanto falta
	gotoxy(2,15); cprintf("Completo: ");  
	/**** INICIAR
	   Lanzar N fotones, iniciando cada uno antes de la propagación.
	   El programa que lanza y propaga cada  fotón en el tejido esta dentro de este do-while,
	   el cual lanza N fotones
	*****/
	do 
	{
		/**** LANZAMIENTO
		   Iniciar la posición y trayectoria del fotón
		   Implementar la fuente
		*****/

		if(sourcet==1){
		/* 
		  FUENTE ISOTRÓPICA
		  Aleatoriamente colocar la trayectoria del fotón para producir una fuente isotrópica 
		  Para la fuente el valor inicial theta es colocado aleatoriamente al seleccionar un valor para cos(theta),
		  llamado costheta entre -1 y 1 (correspondiente a 180° y 0° con respecto al eje z).
		  El valor inicial psi se coloca aleatoriamente entre 0 y 2Pi
		  El término sintheta es una variable temporal relacionado a costheta.
		  Los valores costheta, sintheta y psi son usados para proyectar el vector de trayectoria en los eje x, y, z
		  para producir los valores ux, uy & uz. El vector de trayectoria (ux,uy,uz) tienen unidades de longitud.
		*/
		i_photon += 1;				/* incrementar el contador del fotón */
		W = 1.0;                    /* poner el peso del fotón a 1 */
		photon_status = ALIVE;      /* el fotón esta en modo ALIVE */
		/* La posición inicial del fotón en el origen		*/
		x = 0;
		y = 0;
		z = 0;
		/* Trayectoria */
		costheta = 2.0*RandomNum - 1.0;		/* costheta esta entre -1 y 1 */
		sintheta = sqrt(1.0 - costheta*costheta);	/* sintheta es siempre positivo */
		psi = 2.0*PI*RandomNum;				/* el valor de psi esta entre 0 y 2PI */
		ux = sintheta*cos(psi);
		uy = sintheta*sin(psi);
		uz = costheta;
		}
		else if(sourcet==2){
		/*
		  FUENTE COLIMADA 
		  Lanzar fotones colimados, involucra el colocar la trayectoria inicial de los fotones en una dirección.
		  El código lanzará fotones en dirección del eje z, entrando al tejido en el punto de origen.
		*/
		i_photon += 1;				/* incrementar el contador del fotón */
		W = 1.0;                    /* poner el peso del fotón a 1 */
		photon_status = ALIVE;      /* el fotón esta en modo ALIVE */
		/* La posición inicial del fotón en el origen		*/
//		x = 0;
//		y = 0;
//		z = 0;
		
		a = 20;					/* el radio del haz */
		/* Generar la parte aleatoria de la posición radial y angular */
		r = a*sqrt(RandomNum);
		phi = 2.0*PI*(RandomNum);
		/* La posición inicial, x & y se eligen basados en r & phi */
		x = r*cos(phi);
		y = r*sin(phi);
		z = 0;	
		
		/*Trayectoria, no hay en x, ni en y, solo en z*/
		ux = 0;
		uy = 0;
		uz = 1.0;
		}
		else if(sourcet==3){
		/*
		  HAZ FINITO
		  Iniciar la posición del fotón y trayectoria
		  Implementa una fuente colimada.
          En este caso, la posición de la puesta en marcha se debe variar de forma aleatoria, 
		  para que una distribución especial uniforme de lanzamientos de fotones sea lograda.
		  El haz va a tener un radio a, la probabilidad del lanzar en una posición radial r es p(r).
		  
		  p(r) = (2 pi r)/(pi a^2)
		  
		  El método de MonteCarlo para seleccionar r de p(r) usando un número aleatorio, rnd,
		  uniformemente distribuido entre 0 y 1 inclusivemente, es:
		  
		  rnd = (r^2)/(a^2)
		  
		  Reordenar la ecuación anterior para resolver para r como una función de rnd. La expresión
		  resultante permitirá la selección de la posición radial r para el lanzamiento de un fotón 
		  basado en el número aleatorio rnd:
		  
		  r = a sqrt(rnd)
		  
		  Los fotones deben de ser lanzados uniformemente en el plano x-y dentro del haz de radio a
		  La magnitud radial r ha sido elegida. Ahora, se escoge al ángulo phi tal que las coordenadas radiales
		  (r, phi) define el punto de lanzamiento. Se escoge phi relativo al eje x. Entonces phi es especificado
		  al segundo número aleatorio:
		  
		  phi = 2 pi rnd
		  
		*/
		i_photon += 1;				/* incrementar el contador del fotón */
		W = 1.0;                    /* poner el peso del fotón a 1 */
		photon_status = ALIVE;      /* el fotón esta en modo ALIVE */
		a = 0.5;					/* el radio del haz */
		/* Generar la parte aleatoria de la posición radial y angular */
		r = a*sqrt(RandomNum);
		phi = 2.0*PI*(RandomNum);
		/* La posición inicial, x & y se eligen basados en r & phi */
		x = r*cos(phi);
		y = r*sin(phi);
		z = 0;
		/* Trayectorias, va dirigida en el eje z */
		ux = 0;
		uy = 0;
		uz = 1.0;
		}
		
		/* HOP_DROP_SPIN_CHECK
		   Propagar un fotón hasta que desaparezca como se determina por la ROULETTE
		   Cada fotón es repetidamente permitido a saltar (HOP) a una nueva posición
		   perder/tirar (DROP) algo de peso debido a la absorción
		   girar (SPIN) en una nueva trayectoria debido al esparcimiento 
		   y revisar (CHECK) donde el fotón debe desaparecer debido a la ROULETTE
		*******/
		do 
		{
			
			/**** HOP
			   Tomar un paso a la nueva posición
			   s = tamaño del paso
			   ux, uy, uz son los cosenos de la trayectoria actual del fotón
			*****/
			while ((rnd = RandomNum) <= 0.0);   /* produce 0 < rnd <= 1 */
			s = -log(rnd)/(mua + mus);          /* Tamaño del paso.  Nota: log() es base e */
			/* Actualizar posición. */
			x += s * ux;                        
			y += s * uy;
			z += s * uz;

			/**** DROP
			   Quitar el peso del fotón (W) en el paquete local.
			*****/
			absorb = W*(1 - albedo);      /* Peso del fotón absorbido en este paso */
			W -= absorb;                  /* decrementar WEIGHT por la canitdad absorbida */
			   
			/* esférico */
			r = sqrt(x*x + y*y + z*z);    /* Posición esférica radial actual */
			ir = (short)(r/dr);           /* ir = índice del paquete espacial */
			if (ir >= NR) ir = NR;        /* el último paquete es para desbordamiento */
			Csph[ir] += absorb;           /* peso absorbido en el paquete */
			   
			/* cilindrico */
			r = sqrt(x*x + y*y);          /* Posición clindrica radial actual */
			ir = (short)(r/dr);           /* ir = índice del paquete espacial */
			if (ir >= NR) ir = NR;        /* el último paquete es para desbordamiento */
			Ccyl[ir] += absorb;           /* peso absorbido en el paquete */
			   
			/* plano */
			r = fabs(z);                  /* Posición plana radial actual */
			ir = (short)(r/dr);           /* ir = índice del paquete espacial */
			if (ir >= NR) ir = NR;        /* el último paquete es para desbordamiento */
			Cpla[ir] += absorb;           /* peso absorbido en el paquete */
			   

			/**** SPIN 
			   El fotón esparcido en la nueva trayectoria definido por theta y psi
			   Theta es especificado por cos(theta), el cual es determinado
			   basado en la función de esparcimiento de Henyey-Greenstein
			   Convertir theta y psi en cosenos ux, uy y uz
			*****/
			/* Muestreo para costheta 
				El ángulo de deflexión describe como el fotón es deflectado de su trayectoria actual. Un número
				aleatorio (rnd) selecciona un elemento para el valor cos(theta), llamado costheta. Se calcula tmb
				el valor sintheta.
			*/
			rnd = RandomNum;
		    if (g == 0.0)
				costheta = 2.0*rnd - 1.0;
			else 
			{
				double temp = (1.0 - g*g)/(1.0 - g + 2*g*rnd);
				costheta = (1.0 + g*g - temp*temp)/(2.0*g);					
			}
			sintheta = sqrt(1.0 - costheta*costheta); /* sqrt() es más rápido que sin() */

			/* Muestreo de psi 
				La deflexión se asume a ser dirigida con probabilidad igual en cualquier ángulo azimutal (psi). 
				Un número aleatorio (rnd) selecciona un valor para psi el cual es usado para generar valores para
				cos(psi) y sin(psi), los cuales son cospsi y sinpsi
			*/
			psi = 2.0*PI*RandomNum;
			cospsi = cos(psi);
			if (psi < PI)
				sinpsi = sqrt(1.0 - cospsi*cospsi);     /* sqrt() es más rápido que sin() */
			else
				sinpsi = -sqrt(1.0 - cospsi*cospsi);

			/* Nueva trayectoria */
			if (1 - fabs(uz) <= ONE_MINUS_COSZERO) /* Si es cercano a la perpendicular */
			{
				uxx = sintheta * cospsi;
				uyy = sintheta * sinpsi;
				uzz = costheta * SIGN(uz);   /* SIGN() es más rápido que la division */
			} 
			else 	/* Usualmente se usa esta opción */
			{
				temp = sqrt(1.0 - uz * uz);
				uxx = sintheta * (ux * uz * cospsi - uy * sinpsi) / temp + ux * costheta;
				uyy = sintheta * (uy * uz * cospsi + ux * sinpsi) / temp + uy * costheta;
				uzz = -sintheta * cospsi * temp + uz * costheta;
			}
				
			/* Actualizar la trayectoria */
			ux = uxx;
			uy = uyy;
			uz = uzz;


			/**** CHECAR ROULETTE 
			   Si el peso del fotón esta por debajo del THRESHOLD, entonces se termina el fotón usando la técnica de la ruleta
			   El fotón tiene la posibilidad (CHANCE) de tener su peso incrementado por el factor 1/CHANCE
			   y la probabilidad de 1-CHANCE de terminar.
			*****/
			if (W < THRESHOLD) 
			{
			    if (RandomNum <= CHANCE)
					W /= CHANCE;
				else photon_status = DEAD;
			}

			
		} /* finalizar la etapa de STEP_CHECK_HOP_SPIN */
		while (photon_status == ALIVE);

		//Cuanto lleva el proceso
		gotoxy(12,15); cprintf("%3.2f %", 100*(i_photon / Nphotons) );
	/* Si el fotón "muere", entonces lanzar uno nuevo */
	} /* terminar */
	while (i_photon < Nphotons);

	/**** SALVAR
	   Convertir los datos a ritmo de fluencia relativo [cm^-2] y salvarlo en un archivo que el usuario
	   colocará el nombre
	*****/
	/* Mostrar que se finalizó y que se introduzca el nombre del archivo */
	fflush(stdin);
	printf("\nPrograma terminado\n");
	printf("Nombre del archivo (< 8 caracteres): ");
	gets(namf);
	/* Concatenar el nombre con la extensión .out */
	strcat(namf, ".out");
	printf("\nArchivo %s ", namf);
	
	/* Abrir el archivo para escritura */
	target = fopen(namf, "w");

	/* Imprimir la cabecera */
	fprintf(target, "%s \n", nst);
	fprintf(target, "Número de fotones = %f\n", Nphotons);
	fprintf(target, "Tamaño del paquete = %5.5f [cm] \n", dr);
	fprintf(target, "***Parámetros del tejido*** \n");
	fprintf(target, "Coef. de Absorcion: %f\n",mua);
	fprintf(target, "Coef. de Esparcimiento: %f\n",mus);
	fprintf(target, "Coef. de Anisotropia: %f\n",g);
	fprintf(target, "Indice de Refracción: %f\n",nt);
	fprintf(target, "La última fila esta desbordada. Ignorarla\n");

	/* Imprimir los titulos de las columnas */
	fprintf(target, "r [cm] \t Fsph [1/cm2] \t Fcyl [1/cm2] \t Fpla [1/cm2]\n");
	/* Imprimir los datos: posición radial, ritmos de fluencia para las geometrías 3D (esférica), 2D(cilindrica), 1D(plana) */
	for (ir=0; ir<=NR; ir++) 
	{
		/* 
			Los arreglos Csph, Ccyl, Cpla tienen la cantidad de peso absorbido de los fotones
			Dividiendo ese peso por el númeor total de fotones y por el volumen del cascarón o
			el grid produce la concentración en un paquete relativo al número total de fotones lanzados [cm^-3]
			El volumen del cascarón es diferente para cada una de las geometrías.
			Diidiendo cada concentración en el paquete por el coeficiente de absorción produce el ritmo de 
			fluencia relativa F/P [cm^-2], donde F es el ritmo de Fluencia y P es la Potencia de la fuente
		*/
		/* r = sqrt(1.0/3 - (ir+1) + (ir+1)*(ir+1))*dr; */
		r = (ir + 0.5)*dr;
		shellvolume = 4.0*PI*r*r*dr; /* Por cascarón esférico */
		Fsph = Csph[ir]/Nphotons/shellvolume/mua;
		shellvolume = 2.0*PI*r*dr;   /* Por longitud [cm] del cilindro */
		Fcyl = Ccyl[ir]/Nphotons/shellvolume/mua;
		shellvolume = dr;            /* Por area [cm^2] del plano */
		Fpla =Cpla[ir]/Nphotons/shellvolume/mua;
		fprintf(target, "%5.5f \t %4.3e \t %4.3e \t %4.3e \n", r, Fsph, Fcyl, Fpla);
	}
	printf("... Listo!");
	
	//Cerrar target
	fclose(target);
	
	getch();
} /* fin del main */


/* SUBRUTINAS */

/**************************************************************************
 *	RandomGen
 *		Un generador aleatorio de números que genera uniformemente
 *		números aleatorios distribuidos entre 0 y 1 inclusive.
 *		El algortimo esta basado en:
 *      W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P.
 *      Flannery, "Numerical Recipes in C," Cambridge University
 *      Press, 2nd edition, (1992).
 *      &
 *      D.E. Knuth, "Seminumerical Algorithms," 2nd edition, vol. 2
 *      of "The Art of Computer Programming", Addison-Wesley, (1981).
 *
 *		Cuando el tipo es 0, colocar Seed como la semilla. Asegurar 0 < Seed < 320000.
 *		Cuando el tipo es 1, regresar el número aleatorio.
 *		Cuando el tipo es 2, obtener el estado del generador.
 *		Cuando el tipo es 3, restaurar el estado del generador.
 *
 *		El estado del generador es representado por Status[0...56].
 *
 *		Asegurar que se incializa la semilla antes de obtener el número aleatorio sino se
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
	static long i1, i2, ma[56];   /* ma[0] no es usado */
	long        mj, mk;
	short       i, ii;

	if (Type == 0) /* Colocar la semilla */
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
	else if (Type == 1) /* Obtener el número */
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
	else if (Type == 2) /* Obtener el estado */
	{
		for (i = 0; i < 55; i++)
			Status[i] = ma[i + 1];
		Status[55] = i1;
		Status[56] = i2;
	} 
	else if (Type == 3)  /* Restablecer el estado */
	{
		for (i = 0; i < 55; i++)
			ma[i + 1] = Status[i];
		i1 = Status[55];
		i2 = Status[56];
	} 
	else
		puts("Parametros erróneos para RandomGen().");
	return (0);
}

/*QUITAR LAS DEFINICIONES*/
#undef MBIG
#undef MSEED
#undef MZ
#undef FAC
