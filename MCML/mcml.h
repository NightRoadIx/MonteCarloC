/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Simulaciones Monte Carlo de la distribucion de fotones en
 *	un medio multi-capa en C ANSI Estandar
 ****
 *	Fecha de creacion:				10/1991.
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
 *	El programa esta basado en:
 *	(1) El codigo en Pascal escrito por Marleen Keijzer y
 *	Steven L. Jacques en su laboratorio en 1989, el cual
 *	trata con un medio turbio multi-capa
 *
 *	(2)Algortimo para un medio semi-infinito por
 *	S.A. Prahl, M. Keijzer, S.L. Jacques, A.J. Welch, 
 *	SPIE Institute Series Vol. IS 5 (1989), y por 
 *	A.N. Witt, The Astrophysical journal Supplement
 *	Series 35, 1-6 (1977).
 *	
 *	Las mayores modificaciones incluyen
 *		. Conforme al Standard ANSI C.
 *		. Remoción del límite en el número de elementos del arreglo
 *		  ya que los arreglos en este programa son dinámicamente
 *		  asignados. Esto significa que el programa puede aceptar
 *		  cualquier numero de capas o lineas de division mientras
 *		  la memoria lo permita
 *		. Evitar las variables globales siempre que sea posible
 *		  El programa no ha requerido de usar variables globales.
 *		. Variables agrupadas logicamente usando estructuras
 *		. Diseño Top-down, manteniendo cada subrutina limpia y corta
 *		. La Reflectancia y Transmitancia son resueltas angularmente
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

/* * * * * * * * DEFINICIONES * * * * * * * */
#define PI 3.1415926
#define WEIGHT 1E-4		// Peso crítico para la ruleta
#define CHANCE 0.1		// Oportunidad para la supervivencia de la ruleta
#define STRLEN 256		// Longitud de cadena

#define Boolean char

#define SIGN(x) ((x)>=0 ? 1:-1)

/* * * * * * * * ESTRUCTURAS * * * * * * * */
//Estrcutura usada para describir un paquete de foton
typedef struct {
  double x, y ,z;	// Coordenadas cartesianas [cm]
  double ux, uy, uz;// Cosenos direccionales de un fotón
  double w;			// Peso
  Boolean dead;		// 1 si el fotón termina
  short layer;		// indice a la capa donde el paquete de foton se encuentra
  double s;			// Tamaño del paso actual [cm]
  double sleft;		// Tamaño del paso dejado. [adimensional]
} PhotonStruct;

/*
 *	Estrucutra usada para describir la geometria y las
 *  propieades opticas de la capa
 *  z0 y z1 son las coordenadas en z para los limites superior
 *  e inferior de la capa respectivamente
 */
typedef struct {
  double z0, z1;	// coordeanadas en z para la capa [cm]
  double n;			// indice de refraccion de la capa
  double mua;	    // coeficiente de absorcion [1/cm] */
  double mus;	    // coeficiente de esparcimiento [1/cm] */
  double g;		    // anisotropia
  // cosenos del angulo critico de la reflexion total interna para los limites superior e inferior
  // respectivamente. Tienen un valor de cero si no existe reflexion total interna, son usados
  // para velocidad de computacion
  double cos_crit0,	cos_crit1;
} LayerStruct;

/*
 *	Parametros de entrada para cada ejecucion independiente
 *
 *	z y r son para el sistema coordenado cilindrico [cm]
 *	a es para el angulo alpha entre la direccion del foton saliente
 *  y la superficie normal [radian]
 *
 *	Las separaciones lineales de la rejilla en las direcciones z, r & alpha
 *	son dz, dr & da respectivamente. El numero de lineas de la rejilla
 *	en las direcciones x, r y alpha son nz, nr y na respectivamente
 *
 *	El miembro laterspecs apuntará a un arreglo de estructuras
 *	el cual almacenara los parametros de cada capa
 *	Este arreglo tiene (number_layers + 2) elementos.
 *	Un elemento es para una capa
 *	Las capas 0 & (num_layers + 1) son para el medio ambiente por encima
 *	y debajo del medio turbio respectivamente
 */
typedef struct {
  char	 out_fname[STRLEN];	// nombre del archivo de salida
  char	 out_fformat;		// Formato del archivo de salida
							// 'A' para ASCII
							// 'B' para binario.
  long	 num_photons; 		// # fotones a ser lanzados
  double Wth; 				// lanzar la ruleta si el peso del foton < Wth
  
  double dz;				// separacion en z de la rejilla [cm]
  double dr;				// separacion en r de la rejilla [cm]
  double da;				// separacion alpha de la rejilla [radian]

  short nz;					// intervalo del arreglo 0..nz-1
  short nr;					// intervalo del arreglo 0..nr-1
  short na;					// intervalo del arreglo 0..na-1
  
  short	num_layers;			// Numero de capas
  LayerStruct * layerspecs;	// Parametros de la capa
} InputStruct;

/*
 *	Estructuras para anotar las cantidades fisicas
 *	z & r representan las coordenadas z & r coordinates del
 *	sistema coordeando cilidndrico [cm]
 *	a es para el angulo alpha entre la direccion del foton saliente
 *  y la superficie normal [radian]
 */
typedef struct {
  double    Rsp;	// reflectancia especular
  double ** Rd_ra;	// distribucion en 2D de la reflectancia difusa [1/(cm2 sr)]
  double *  Rd_r;	// distribucion radial 1D de la reflectancia difusa [1/cm2] */
  double *  Rd_a;	// Distribucion angular 1D de la reflectancia difusa [1/sr]
  double    Rd;		// Reflectancia difusa total
  
  double ** A_rz;	// densidad de probabilidad 2D en un medio turbio sobre r & z [1/cm3]
  double *  A_z;	// densidad de probabilidad 1D sobre z [1/cm]
  double *  A_l;	// probabilidad de absorcion de cada capa
  double    A;		// probabilidad de absorcion total
  
  double ** Tt_ra;	// distribucion 2D de la transmitancia total [1/(cm2 sr)]
  double *  Tt_r;	// distribucion radial 1D de la transmitancia [1/cm2]
  double *  Tt_a;	// distribucion angular 1D de la transmitancia [1/sr]
  double    Tt;		// Transmitancia total
} OutStruct;

 /* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * FUNCIONES PROTOTIPO
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 //Prototipo de funciones para la asignacion dinamica de memoria
 //y liberacion de los arreglos y matrices
double  *AllocVector(short, short);
double **AllocMatrix(short, short,short, short);
void 	 FreeVector(double *, short, short);
void 	 FreeMatrix(double **, short, short, short, short);
void 	 nrerror(char *);
