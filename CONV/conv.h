/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Program Name: conv.
 *
 *	Programa utilizado para procesar datos para mcml -
 *	Simulación Monte Carlo de la distribución de fotones en 
 *	medios turbios multi-capa en ANSI-C Standar
 ****
 *	Fecha de creacion:				11/1991.
 *	Fecha de ultima modificacion:	13/2012.
 *
 *	Lihong Wang, Ph. D.
 *	Steven L. Jacques, Ph. D.
 *	Laser Biology Research Laboratory - 17
 *	M.D. Anderson Cancer Center
 *	University of Texas
 *	1515 Holcombe Blvd.
 *	Houston, TX 77030
 *	USA
 *
 ****
 *	Convenciones generales en la nomenclatura:
 *	Nombres del preprocesador, todos en letras mayusculas
 *		e.g. #define PREPROCESSORS
 *	Globales: la primera letra de cada palabra es mayuscula, sin guiones bajos
 *		e.g. short GlobalVar;
 *	Variables Dummy:  la primera letra de cada palabra es mayuscula,
 *		y las palabras estan conectadas por guiones bajos.
 *		e.g. void NiceFunction(char Dummy_Var);
 *	Variables locales: todos en letras minusculas, las palabras esta conectadas
 *		por guiones bajos,
 *		e.g. short local_var;
 *	Nombres de funciones o tipos de datos:  de la misma forma que las globales.
 *
 ****
 *	Dimensiones de longitud: cm.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * LIBRERIAS INCLUIDAS * * * * * * * */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <conio.h>

/* * * * * * * * DEFINICIONES * * * * * * * */
#define PI 3.1415926	//Valor de pi utilizado en el programa
#define STRLEN 256		//Longitud de la cadena

#define Boolean char

//Definicion de operaciones para incrementar la velocidad de procesamiento
#define SIGN(x) ((x)>=0 ? 1:-1)			//Operación SIGNUM
#define MAX(x, y) ((x)>(y) ? (x):(y))	//Operación Max
#define MIN(x, y) ((x)<(y) ? (x):(y))	//Operacion min

#define NULLCONVSTRUCT {0, 0, 0, 0, 0}
#define NULLOUTSTRUCT {0.0,\
		NULL,NULL,NULL,0.0,\
		NULL,NULL,NULL,0.0,\
		NULL,NULL,NULL,0.0,\
		NULL,NULL,NULL,NULL,NULL,\
		0, NULLCONVSTRUCT}

/* * * * * * * * ESTRUCTURAS * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Estructura utilizada para describir la geometría y las
 *	propiedades ópticas de la capa.
 *	z0 & z1 son las coordenadas z para la  frontera superior
 *	e inferior respectivamente.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct 
{
	double z0, z1;	//Coordenadas z de la capa [cm]
	double n;		//Indice de refracción de la capa
	double mua;		//Coeficiente de absorción [1/cm]
	double mus;		//Coeficiente de esparcimiento [1/cm]
	double g;		//Anisotropía
} LayerStruct;

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Parametros para describir el haz de fotones.
 *  Tipo lapiz: Haz infinitamente delgado. Este es el haz default
 *			    para la salida de la simulación mcml.
 *	Plano:		Haz plano colimado de radio R.
 *	Gaussiano:	Gaussiano con radio 1/e2 R.
 *	Otros: 		Haz general descrito por puntos con interpolacion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct 
{
	//Enumeracion del tipo de haz
	enum 
	{
		pencil, flat, gaussian, others
	} type;
	double P;		//Potencia total [J]
	double R;		//Radio [cm]
} BeamStruct;

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Los parametros de entrada para cada ejecucion independiente.
 *
 *	z & r son para el sistema de coordenadas cilindricas [cm].
 *	a es para el angulo alpha entre la direccion de salida 
 *	y la superficie normal [radian].
 *
 *	Las separaciones entre las lineas de la rejilla en las 
 *	direcciones x, r & alpha son dz, dr & da respectivamente. 
 *	Los numeros de las lineas de la rejilla en las direcciones z, r & alpha
 *	son nz, nr, & na respectivamente.
 *
 *	El layerspecs apuntara a un arreglo de estructuras
 *  en el cual se almacenaran paramateros de cada capa.
 *	Este arreglo tiene (numero_capas + 2) elementos.
 *	Un elemento es para la capa.
 *	Las capas 0 & (num_layers + 1) son para la capa superior del
 *	ambiente y la inferior respectivamente.
 *
 *	Para la convolucion, las separaciones de las lineas de la rejilla en
 *	la direccion x & alpha siguen siendo dz & da respectivamente. Los numeros
 *	de las lineas de la rejilla en las direcciones z & alpha siguen siendo
 *	nz, and na respectivamente. Sin embargo, las separaciones de las lineas de la rejilla
 *	& el numero de linas en la direccion r son drc & nrc respectivamente
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct 
{
	char in_fname[STRLEN];	//Nombre del archivo de salida de la simulacion mcml
	char in_fformat;		//Formato del archivo de salida mcml
							// 'A' para ASCII
							// 'B' para binario
	long num_photons;		//Numero de fotones a simular
	double Wth;				//Para la ruleta, el peso umbral, peso < Wth
	
	double dz;				//Separacion de la rejilla en z [cm]
	double dr;				//Separacion de la rejilla en r [cm]
	double da;				//Separacion de la rejilla en alpha [rad]
	
	short nz;				//Intervalo del arreglo 0..nz-1.
	short nr;				//Intervalo del arreglo 0..nr-1.
	short na;				//Intervalo del arreglo 0..na-1.

	short num_layers;		//Numero de capas
	LayerStruct *layerspecs;//Parametros de la capa

	BeamStruct beam;		//Haz incidente de tamaño finito
	double drc;				//Separacion de la rejilla r de la convolucion [cm]
	short nrc;				//Intervalo del arreglo de la convolucion 0..nrc-1.
	float eps;				//Error relativo en la convolucion
} InputStruct;

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Estructura para llevar un registro de que cantidades son convulocionadas
 *	Son inicializadas a 0, & son solocadas a 1 cuando la cantidad 
 *	representeda esta convulocionada.
 *
 *	Cuando el perfil del haz es cambiado, todos ellos son reiniciados a 0
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct 
{
	Boolean Rd_ra;
	Boolean Rd_r;
	Boolean A_rz;
	Boolean Tt_ra;
	Boolean Tt_r;
} ConvStruct;

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Estrcuturas para anotar las cantidades fisicas de
 *	mcml y para ser convolucionadas con los haces de fotones
 *	de tamaño finito. Entonces, aquí "fuera/out" significa que
 *	es la salida tanto de mcml y conv.
 *
 *	El elemento asigando es utilizado para mantener el status
 *	de los arreglos. Es colocado a 1 si todos los arreglos
 *	se les asignan y almacenan valores. En otro caso se inicializa a 0.
 *
 *	z & r representan las coordenadas z & r del sistema de
 *	coordenadas cilindricas [cm].
 *	a es el angulo alpha entre la direccion de salida del foton
 *	y la normal a la superficie [radian].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct 
{
	double Rsp;			//Reflectancia especular
	double **Rd_ra;		//Distribucion 2D de la reflectancia difusa [1/(cm2 sr)
	double *Rd_r;		//Distribucion radial de la reflectancia difusa [1/cm2]
	double *Rd_a;		//Distribucion angular de la reflectancia difusa [1/sr]
	double Rd;			//Reflectancia difusa total

	double **A_rz;		//Densidad de probabilidad 2D en el medio turbio r & z [1/cm3]
	double *A_z;		//Densidad de probabilidad 1D sobre z [1/cm]
	double *A_l;		//Probabilidad de absorcion de cada capa
	double A;			//Probabilida de absorcion total

	double **Tt_ra;		//Distribucion total 2D de la transmitancia [1/(cm2 sr)
	double *Tt_r;		//Distribucion radial 1D de la transmitancia [1/(cm2)
	double *Tt_a;		//Distribucion angular 1D de la transmitancia [1/(sr)
	double Tt;			//Transmitancia total

	double **Rd_rac;	//Datos convulocionados [J/(cm2 sr)]
	double *Rd_rc;		//[J/cm2]
	double **A_rzc;		//[J/cm3]
	double **Tt_rac;	//[J/(cm2 sr)]
	double *Tt_rc;		//[J/cm2]

	char allocated;		//Colocar a 1 donde los arreglos son almacenados
	ConvStruct conved;
} OutStruct;

/* * * * * * * * FUNCIONES PROTOTIPO * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Rutinas prototipo para la asignacion dinamica de memoria &
 *	liberar los arreglos y matrices.
 *	Modificado de "Numerical Recipes in C".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double *AllocVector(short, short);
double **AllocMatrix(short, short, short, short);
void FreeVector(double *, short, short);
void FreeMatrix(double **, short, short, short, short);
void nrerror(char *);

//Otros prototipos
/*
 *  Z[i][j] el arreglo 2D
 *  IXmax, IYmax, el 0<=i<=IXmax, 0<=j<=IYmax.
 *  Dx, Dy, las separaciones de la rejilla
 */
void IsoPlot(double **Z, long int IXmax, long int IYmax, double Dx, double Dy);
short GetShort(short, short);
float GetFloat(float, float);
