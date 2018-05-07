/********************************************
 *  trmc.c    , en lenguaje ANSI Estándar C
 *  Monte Carlo resuelto en tiempo.
 *
 *  La simulación de Monte Carlo del ritmo de fluencia de fotones resuelta en tiempo
 *  en respuesta a un impulso de energía en el tiempo cero, irradiada como una fuente
 *  puntual isotrópica. La propagación de fotones es 3D en coordenadas esféricas.
 *  El ritmo de fluencia instáneo,
 *  F(r,t)/Uo [cm^-2 s^-1]  donde F(r,t) = [J cm^-2 s^-1] & Uo = [J].
 *  Con fines de cálculo, el valor de U0 se asume como la unidad [1 J].
 *  La propagación en un medio semiinfinito homogéneo
 *  sin fronteras.
 *
 *  Los resultados son almacenados como F[ir][it]/Uo para cada uno de los puntos en el tiempo
 *    t = T[it], it = 0-3, & 100 posiciones radiales , r = (ir + 0.5)*dr.
 *    Results are placed in output file "trmc.out" as columns:
 *    r [cm]   F/Uo @ time#1   F/Uo @ time#2   F/Uo @ time#3   F/Uo @ time#4
 *
 *  by Steven L. Jacques based on prior collaborative work 
 *    with Lihong Wang, Scott Prahl, and Marleen Keijzer.
 *    partially funded by the NIH (R29-HL45045, 1991-1997) and  
 *    the DOE (DE-FG05-91ER617226, DE-FG03-95ER61971, 1991-1999).
 *
 **********/

#include <math.h>
#include <stdio.h>

#define	PI          3.1415926
#define	LIGHTSPEED	2.997925E10 /* Velocidad de la luz en el vacío [cm/s] */
#define ALIVE       1   		/* Si el fotón aún no termina */
#define DEAD        0    		/* Si el fotón va a ser terminado */
#define THRESHOLD   0.01		/* usado en la ruleta */
#define CHANCE      0.1  		/* usado en la ruleta */
#define COS90D      1.0E-6
     /* Si cos(theta) <= COS90D, theta >= PI/2 - 1e-6 rad. */
#define ONE_MINUS_COSZERO 1.0E-12
     /* Si 1-cos(theta) <= ONE_MINUS_COSZERO, fabs(theta) <= 1e-6 rad. */
     /* Si 1+cos(theta) <= ONE_MINUS_COSZERO, fabs(PI-theta) <= 1e-6 rad. */
#define SIGN(x)           ((x)>=0 ? 1:-1)
#define InitRandomGen    (double) RandomGen(0, 1, NULL)
     /* Inicializa la semilla para el generador de números aleatorios. */     
#define RandomNum        (double) RandomGen(1, 0, NULL)
     /* Llama al número aleatorio del generador de números aleatorios. */

/* DECLARACIÓN DE FUNCIONES */
/* Generador de números aleatorios */
double RandomGen(char Type, long Seed, long *Status);  

void main() 
{
	/* Parámetros de propagación */
	double	x, y, z;    /* Posición del fotón */
	double	ux, uy, uz; /* Trayectoria del fotón como cosenos */
	double  uxx, uyy, uzz;	/* Valores temporales usados durante el SPIN */
	double	s;          /* Tamaño de los pasos. s = -log(RND)/mus [cm] */
	double	costheta;   /* cos(theta) */
	double  sintheta;   /* sen(theta) */
	double	cospsi;     /* cos(psi) */
	double  sinpsi;     /* sen(psi) */
	double	psi;        /* ángulo acimutal */
	double	i_photon;   /* fotón actual */
	short   photon_status;  /* bandera = VIVO=1 or MUERTO=0 */

	/* Variables de tiempo y de longitud del paso */
	short	NTpts;      /* número de puntos de tiempo */
	double  T[4];       /* Puntos de tiempo en los cuales las concentraciones de fotones se mapean */
	double  LT[4];      /* Longitudes del paso totales como punto de tiempo */
	double  Lmax;       /* Puntos de tiempo y longitudes de paso máximo considerados */
	double  L;          /* Longitudes de paso del fotón totales */
	short	it;         /* índice al punto del tiempo */

	/* Otras variables */
	double  c;          /* Velocidad de la luz en el medio */
	double	Csph[101][4];  /* Concentración de fotones C[ir=0..100][it=0-3] */
	double	F[101][4];  /* Ritmo de fluencia */
	double	mus;        /* Coeficiente de esparcimiento [cm^-1] */
	double	g;          /* anisotropía [-] */
	double	nt;         /* índice de refracción del tejido */
	double	Nphotons;   /* número de fotones en la simulación */
	short	NR;         /* número de posiciones radiales */
	double	radial_size;  /* tamaño radial máximo */
	double	r;          /* posición radial */
	double  dr;         /* tamaño del depósito o "bin" radial */
	short	ir;         /* índice a la posición */
	double  shellvolume;  /* volumen del cascarón en la posición radial r */

	/* Variables extras */
	double  rnd;            /* valor aleatorio asignado 0-1 */
	double  s1, x1, y1, z1; /* valores temporales */
	short	i, j;           /* índices */
	double	u, temp, sum;   /* variables */
	FILE*	target;         /* apuntador al archivo de salida */

	printf("Inicio\n");


	/**** ENTRADA
	   Entrada de propiedades opticas
	   Entrada del tamaño del paquete y el arreglo
	   Entrada del número de fotones
	*****/

	Nphotons    = 1E3; /* Colocar el número de fotones en la simulación */
	/* Propieades ópticas: */
	mus         = 100;  /* cm^-1 */
	g           = 0.90;  
	nt          = 1.33;
	/* Parámetros de los depósitos o "bins" */
	NR          = 100;	 /* Colocar el número de depósitos.  */
	radial_size = 6.0;   /* Intervalo total en donde los depósitos se extienden [cm] */
	dr          = radial_size/NR;  /* cm */
	/* Parámetros de los puntos en el tiempo: */
	NTpts    = 4;        /* Número de puntos en el tiempo */
	T[0]        = 0.050e-9; /* 1° punto en el tiempo */
	T[1]        = 0.100e-9; /* 2° punto en el tiempo */
	T[2]        = 0.500e-9; /* 3° punto en el tiempo */
	T[3]        = 1e-9;     /* 4° punto en el tiempo */

	/**** INICIALIZACIÓN
	*****/
	dr = radial_size/NR;      /* cm,  tamaño del depósito incremental  */
	c = LIGHTSPEED/nt;        /* cálculo de la velocidad de la luz en el tiempo */
	i_photon = 0;             /* contador de fotones a cero */
	InitRandomGen;            /* inicia la semilla y el generador de números aleatorios */

	for (it=0; it<NTpts; it++) 
		LT[it] = T[it]*c;     /* tamaños de paso LT[it] asociados con T[it] */
	Lmax  = LT[NTpts-1];      /* última longitud de paso considerada */

	for (ir=0; ir<=NR; ir++)  /* limpiar el arreglo de datos */
		for (it=0; it<NTpts; it++)
			Csph[ir][it] = 0;
			
	   
	/**** INICIAR
	   Lanzar N fotones, iniciando cada uno antes de la propagación.
	   El programa que lanza y propaga cada  fotón en el tejido esta dentro de este do-while,
	   el cual lanza N fotones
	*****/
	do 
	{

		/**** LANZAMIENTO
		   Iniciar la posición y trayectoria del fotón
		   Implementar la fuente isotrópica
		*****/
		i_photon += 1;	            /* incrementar el contador de fotones */

		photon_status = ALIVE;      /* Lanzar el fotón como VIVO */
		L = 0;                      /* colocar el paso del fotón acumulado como cero */
		it = 0;                     /* colocar el apuntador al punto de tiempo T[1] */

		x = 0;                      /* Colocar la posición del fotón en el origen. */
		y = 0;
		z = 0;

		/* Aleatoriamente colocar la trayectoria del fotón para producir una fuente isotrópica. */
		costheta = 2.0*RandomNum - 1.0;   
		sintheta = sqrt(1.0 - costheta*costheta);	/* sintheta es siempre positiva */
		psi = 2.0*PI*RandomNum;
		ux = sintheta*cos(psi);
		uy = sintheta*sin(psi);
		uz = costheta;


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
			  while ((rnd = RandomNum) <= 0.0);   /* produce 0 < rnd <= 1. 
													 Si rnd == 0, reintentar. */
			  s = -log(rnd)/mus;                  /* Tamaño del paso.  Nota: log() está en base e */

			  if (L + s >= LT[it]) {
					s1 = LT[it] - L;              /* partial step to   position at T[it] */
					x1 = x + s1*ux;               /* move to temporary position at T[it] */
					y1 = y + s1*uy;
					z1 = z + s1*uz;
				
					/******************* DROP *********
					*	Registrar ña posición del fotón en el bin local C[ir][it].
					*	Se graban la concentración de fotones en el tiempo T[it].
					*   Cualquier perdida de energía del fotón debido a la abosrción puede después
					*	ser tomada en cuenta por el término (-mua*c*T[it]).
					*****/
					r = sqrt(x1*x1 + y1*y1 + z1*z1); /* Posición esférica radial actual */
					ir = (short)(r/dr);           /* ir = índice al "bin" espacial */
					if (ir >= NR) ir = NR;        /* último "bin" está desbordado */
					Csph[ir][it] += 1;            /* DROP el fotón en el "bin" */
					
					it += 1;                      /* incrementar el apuntador a la siguiente posición */
					}

			  x += s*ux;	/* Actualizar las posiciones al tomar un paso completo s. */
			  y += s*uy;
			  z += s*uz;
			  L += s;       /* Actualizar la longitud de paso del fotón total */
			  

			/**** SPIN 
			   El fotón esparcido en la nueva trayectoria definido por theta y psi
			   Theta es especificado por cos(theta), el cual es determinado
			   basado en la función de esparcimiento de Henyey-Greenstein
			   Convertir theta y psi en cosenos ux, uy y uz
			*****/
			  /* Muestrear para costheta */
			  rnd = RandomNum;
				 if (g == 0.0)
					costheta = 2.0*rnd - 1.0;
				 else {
					double temp = (1.0 - g*g)/(1.0 - g + 2*g*rnd);
					costheta = (1.0 + g*g - temp*temp)/(2.0*g);
					}
			  sintheta = sqrt(1.0 - costheta*costheta); /* sqrt() es más rápido que sin(). */

			  /* Muestrear psi. */
			  psi = 2.0*PI*RandomNum;
			  cospsi = cos(psi);
			  if (psi < PI)
				sinpsi = sqrt(1.0 - cospsi*cospsi);     /* sqrt() es más rápido que sin(). */
			  else
				sinpsi = -sqrt(1.0 - cospsi*cospsi);

			  /* Nueva trayectoria. */
			  if (1 - fabs(uz) <= ONE_MINUS_COSZERO) {      /* Cerca de la perpendicular. */
				uxx = sintheta * cospsi;
				uyy = sintheta * sinpsi;
				uzz = costheta * SIGN(uz);   /* SIGN() es más rápida que la división. */
				} 
			  else {					/* Usualmente se utiliza esta opción */
				temp = sqrt(1.0 - uz * uz);
				uxx = sintheta * (ux * uz * cospsi - uy * sinpsi) / temp + ux * costheta;
				uyy = sintheta * (uy * uz * cospsi + ux * sinpsi) / temp + uy * costheta;
				uzz = -sintheta * cospsi * temp + uz * costheta;
				}
				
			  /* Actualizar la trayectoria */
			  ux = uxx;
			  uy = uyy;
			  uz = uzz;


			/**** CHECK si el fotón está VIVO
			*****/
			if (L >= Lmax)
			   photon_status = DEAD;


		} /* fin del ciclo HOP/DROP_SPIN_CHECK */
		while (photon_status == ALIVE);

		  /* Si el fotón está muerto entonces se lanza otro. */
	}
	while (i_photon < Nphotons);


	/**** SALVAR
	   Convertir los datos a ritmo de fluencia relativo [cm^-2] y salvarlo en un archivo que el usuario
	   colocará el nombre
	*****/
	target = fopen("trmc.out", "w");

	/* Imprimir la cabecera del archivo */
	fprintf(target, "Número de fotones = %f\n", Nphotons);
	fprintf(target, "dr = %5.5f [cm] \n", dr);
	fprintf(target, "La última fila esta desbordada. Ignorarla.\n");
	fprintf(target, "La salida es el ritmo de fluencia F [W/(cm2 s)].\n");

	/* imprimir los títulos de las columnas */
	fprintf(target, "r [cm] \t %5.3f ns \t %5.3f ns \t %5.3f ns \t %5.3f ns\n", 
					 T[0]*1e9, T[1]*1e9, T[2]*1e9, T[3]*1e9);

	/* imprimir los datos: posición radial, Ritmo de fluencia de los fotones @ T[it]. */
	for (ir=0; ir<NR; ir++) 
	{        /* No imprimir el "bin" desbordado. */
		r = (ir + 0.5)*dr;
		fprintf(target, "%5.4f", r);
		shellvolume = 4.0*PI*r*r*dr; /* por cascarón esférico */
		for (it=0; it<4; it++) 
		{
			F[ir][it] = c*Csph[ir][it]/Nphotons/shellvolume;
			fprintf(target, "\t%5.3e", F[ir][it]);
		}
		fprintf(target, "\n");
	}

	fclose(target);


	printf("FIN DE SIMULACIÓN\n");

}

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
