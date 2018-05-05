/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar el perfil del haz de fotones, y convulocionar sobre el haz
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Librerias a incluir */
#include "conv.h"

/* * * * * * * * DEFINICIONES * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Si IBMPC es 1, las extensiones de los archivos estan
 *	limitadas a maximo 3 letras.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define IBMPC 0

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Usado por la convolucion sobre el haz Gaussiano. Ignorar
 *	el valor mas alla de un radio GAUSSLIMIT.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define GAUSSLIMIT 4


/* * * * * * * * DECLARACIONES * * * * * * * */
FILE *GetWriteFile(char *);
FILE *GetWriteFile2(char *, char *);
void AllocConvData(InputStruct *, OutStruct *);
void FreeConvData(InputStruct *, OutStruct *);
float qtrap(float (*) (float), float, float, float);
double BessI0(double);

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Las estrcuturas de datos para el arbol binario utilizado para
 *	almacenar parte de la evaluacion del integrando.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct Node 
{
	float x, y;
	struct Node *left, *right;
};
typedef struct Node *LINK;
typedef LINK TREE;	//Definir el tipo arbol (TREE) como LINK

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Una estructura global para pasar la coordenada actual de
 * 	las cantidades fisicas siendo evaluadas y los punteros de
 *	los parametros de entrada & salida a la funcion de integracion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct 
{
	double r;
	short iz, ia;
	InputStruct *in_ptr;
	OutStruct *out_ptr;
	TREE tree;	//Un arbol para almacenar ITheta() & ExpBessI0().
} ConvVar;


/* * * * * * * * CONVOLUCION * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar los parametros para un haz plano
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GetFlatBeam(BeamStruct * Beam_Ptr)
{
	Beam_Ptr->type = flat;

	printf("Energia total del haz plano [J]: ");
	Beam_Ptr->P = GetFloat(FLT_MIN, FLT_MAX);
	printf("Radio del haz plano [cm]: ");
	Beam_Ptr->R = GetFloat(FLT_MIN, FLT_MAX);
	printf("Potencia Total: %8.2lg J, & radio: %8.2lg cm.\n", Beam_Ptr->P, Beam_Ptr->R);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar los parametros para un haz Gaussiano
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GetGaussianBeam(BeamStruct * Beam_Ptr)
{
	Beam_Ptr->type = gaussian;

	printf("Energia total del haz Gaussiano [J]: ");
	Beam_Ptr->P = GetFloat(FLT_MIN, FLT_MAX);
	printf("Radio 1/e2 del haz Gaussiano [cm]: ");
	Beam_Ptr->R = GetFloat(FLT_MIN, FLT_MAX);
	printf("Potencia Total: %8.2lg J, & radio: %8.2lg cm.\n", Beam_Ptr->P, Beam_Ptr->R);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar los parametros para un haz Gaussiano con parámetros fijos
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GetGaussianBeam2(BeamStruct * Beam_Ptr)
{
	Beam_Ptr->type = gaussian;

	Beam_Ptr->P = 1.0;
	Beam_Ptr->R = 0.02;
	printf("Potencia Total: %8.2lg J, & radio: %8.2lg cm.\n", Beam_Ptr->P, Beam_Ptr->R);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar el perfil del haz
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LaserBeam(BeamStruct * Beam_Ptr, OutStruct * Out_Ptr)
{
	ConvStruct null_conved = NULLCONVSTRUCT;
	char cmd_str[STRLEN];

	printf("Pefil del haz :  p=plano, g=Gaussiano. q=salir: ");
	do
		gets(cmd_str);
	while (!strlen(cmd_str));

	switch (toupper(cmd_str[0])) 
	{
		case 'P':	//Haz plano
			Out_Ptr->conved = null_conved;
			GetFlatBeam(Beam_Ptr);
			break;
		case 'G':	//Gaussiano
			Out_Ptr->conved = null_conved;
			GetGaussianBeam(Beam_Ptr);
			break;
		default:;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificando el perifl del haz gaussiano con sus valores
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LaserBeam2(BeamStruct * Beam_Ptr, OutStruct * Out_Ptr)
{
	ConvStruct null_conved = NULLCONVSTRUCT;
	char cmd_str[STRLEN];
	
	printf("LaserB ");
	//Mandar a llamar el haz gaussiano con valores predefinidos
	GetGaussianBeam2(Beam_Ptr);
}

/* * * * * * * * CONVOLUCIONES * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar la resolucion & el numero de puntos en 
 *	la direccion r.
 *	Colocar el Out_Ptr->conved a NULL.
 *	Reasignar los arreglos para la convolucion de arreglos.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvResolution(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char in_str[STRLEN];
	ConvStruct null_conved = NULLCONVSTRUCT;

	if (!Out_Ptr->allocated) 
	{
		puts("...No hay datos de salida mcml para trabajar con");
		return;
	}
	printf("Resolucion actual: %8.2lg cm & numeros de puntos: %hd\n", In_Ptr->drc, In_Ptr->nrc);
	printf("Resolucion de entrada en la direccion r [cm]: ");
	In_Ptr->drc = GetFloat(FLT_MIN, FLT_MAX);
	printf("Ingresar el numero de puntos en la direccion r: ");
	In_Ptr->nrc = GetShort(1, SHRT_MAX);

	printf("Resolucion: %8.2lg cm & numero de puntos: %hd\n", In_Ptr->drc, In_Ptr->nrc);

	Out_Ptr->conved = null_conved;
	FreeConvData(In_Ptr, Out_Ptr);
	AllocConvData(In_Ptr, Out_Ptr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Especificar el error de convolucion relativo. Se recomienda 
 *  utilizar un intervalo 0.001-0.1.
 *	Colocar el Out_Ptr->conved a NULL.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvError(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char in_str[STRLEN];
	ConvStruct null_conved = NULLCONVSTRUCT;
	float eps;

	printf("Error de convolucion relativo\n");
	printf("Valor actual es %8.2g (0.001-0.1 es recomendado): ", In_Ptr->eps);
	In_Ptr->eps = GetFloat(FLT_MIN, 1);
	Out_Ptr->conved = null_conved;
}

/* * * * * * * * ARBOL BINARIO * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Funcion para llenar un nodo del arbol
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
LINK FillNode(float x, float y)
{
	LINK l;

	l = (LINK) malloc(sizeof(struct Node));
	if (l) 
	{
		l->x = x;
		l->y = y;
		l->left = l->right = NULL;
	}
	return (l);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para insertar un nodo en el arbol.
 *	Asumir que (x, y) no esta en el arbol.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void InsertNode(TREE * TreePtr, float x, float y)
{
	LINK l1, l2;	//11 buferea 12

	l1 = NULL;
	l2 = *TreePtr;
	while (l2 != NULL) 
	{
		l1 = l2;
		if (x < l2->x)
			l2 = l2->left;
		else
			l2 = l2->right;
	}

	if (l1 == NULL)		//En el caso de que el arbol este vacio
		*TreePtr = FillNode(x, y);
	else if (x < l1->x)
		l1->left = FillNode(x, y);
	else
		l1->right = FillNode(x, y);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para buscar un nodo en el arbol.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
LINK SearchNode(TREE Tree, float x)
{
	LINK l = Tree;
	Boolean found = 0;

	while (l != NULL && !found)
		if (x < l->x)
			l = l->left;
		else if (x > l->x)
			l = l->right;
		else
			found = 1;
			
  return (l);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para insertar un nodo en el arbol.
 *	Asumir que (x, y) no esta en el arbol.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeTree(TREE Tree)
{
	if (Tree) 
	{
		FreeTree(Tree->left);
		FreeTree(Tree->right);
		free(Tree);
	}
}

/* * * * * * * * INTEGRACION * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Calcular Itheta
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double ITheta(double r, double r2, double R)
{
	double temp;

	if (R >= r + r2)
		temp = 1;
	else if (fabs(r - r2) <= R)
	{
		temp = (r * r + r2 * r2 - R * R) / (2 * r * r2);
		if (fabs(temp) > 1)
			temp = SIGN(temp);
		temp = acos(temp) / PI;
	} 
	else	/* R < fabs(r-r2) */
		temp = 0;

  return (temp);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Calcular el exponenteBesselIO
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double ExpBessI0(double r, double r2, double R)
{
	double expbess;
	double _RR = 1 / (R * R);
	double x = 4 * r * r2 * _RR;
	double y = 2 * (r2 * r2 + r * r) * _RR;

	expbess = exp(-y + x) * BessI0(x);
	return (expbess);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Interpolar para los arreglos de A_rz[]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double A_rzInterp(double **A_rz, double r2)
{
	double ir2, A_lo, A_hi, A_at_r2;
	short iz, ir2lo, nr = ConvVar.in_ptr->nr;

	ir2 = r2 / ConvVar.in_ptr->dr;
	iz = ConvVar.iz;
	if (nr < 3)
		A_at_r2 = A_rz[0][iz];
	else if (ir2 < nr - 1.5) // interpolation.
	{	
		ir2lo = MAX(0, (short) (ir2 - 0.5));	//Truncamiento
		A_lo = A_rz[ir2lo][iz];
		A_hi = A_rz[ir2lo + 1][iz];
		A_at_r2 = A_lo + (A_hi - A_lo) * (ir2 - ir2lo - 0.5);
	} 
	else	//Extrapolacion 
	{
		ir2lo = nr - 3;
		A_lo = A_rz[ir2lo][iz];
		A_hi = A_rz[ir2lo + 1][iz];
		if (A_lo >= A_hi)	//Ruido de prueba o prueba de ruido
			A_at_r2 = A_lo + (A_hi - A_lo) * (ir2 - ir2lo - 0.5);
		else
			A_at_r2 = 0.0;
	}

	return (MAX(0, A_at_r2));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Interpolar para los arreglos de Rd_ra[] o Tt_ra[]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double RT_raInterp(double **RT_ra, double r2)
{
	double ir2, RT_lo, RT_hi, RT_at_r2;
	short ia, ir2lo, nr = ConvVar.in_ptr->nr;

	ir2 = r2 / ConvVar.in_ptr->dr;
	ia = ConvVar.ia;
	if (nr < 3)
		RT_at_r2 = RT_ra[0][ia];
	else if (ir2 < nr - 1.5)	//Interpolacion
	{
		ir2lo = MAX(0, (short) (ir2 - 0.5));	//Truncamiento
		RT_lo = RT_ra[ir2lo][ia];
		RT_hi = RT_ra[ir2lo + 1][ia];
		RT_at_r2 = RT_lo + (RT_hi - RT_lo) * (ir2 - ir2lo - 0.5);
	} 
	else	//extrapolacion
	{
		ir2lo = nr - 3;
		RT_lo = RT_ra[ir2lo][ia];
		RT_hi = RT_ra[ir2lo + 1][ia];
		if (RT_lo >= RT_hi)		//Ruido de prueba o prueba de ruido
			RT_at_r2 = RT_lo + (RT_hi - RT_lo) * (ir2 - ir2lo - 0.5);
		else
			RT_at_r2 = 0.0;
	}

	return (MAX(0, RT_at_r2));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Interpolar para los arreglos de Rd_r[] o Tt_r[]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

double RT_rInterp(double *RT_r, double r2)
{
	double ir2, RT_lo, RT_hi, RT_at_r2;
	short ir2lo, nr = ConvVar.in_ptr->nr;

	ir2 = r2 / ConvVar.in_ptr->dr;
	if (nr < 3)
		RT_at_r2 = RT_r[0];
	else if (ir2 < nr - 1.5) //Interpolacion
	{
		ir2lo = MAX(0, (short) (ir2 - 0.5));	//Truncamiento
		RT_lo = RT_r[ir2lo];
		RT_hi = RT_r[ir2lo + 1];
		RT_at_r2 = RT_lo + (RT_hi - RT_lo) * (ir2 - ir2lo - 0.5);
	} 
	else 
	{			/* extrapolation. */
		ir2lo = nr - 3;
		RT_lo = RT_r[ir2lo];
		RT_hi = RT_r[ir2lo + 1];
		if (RT_lo >= RT_hi)		//Ruido de prueba o prueba de ruido
			RT_at_r2 = RT_lo + (RT_hi - RT_lo) * (ir2 - ir2lo - 0.5);
		else
			RT_at_r2 = 0.0;
	}

	return (MAX(0, RT_at_r2));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Integrando de la convolucion para lso haces plano & gaussiano.
 *	Regresar el integrando para la convolucion integral.
 *	r2 es el r" en la ecuacion mostrada en el manual.
 *	Cuando r2 esta en el intervalo del arreglo guardado, la interpolacion
 *	es utilizada para evaluar la reflectancia difusa en r2.
 *	Hay que tener en cuenta que dado que los ultimos elementos del arreglo
 *	reunen todo el peso del foton que cae mas alla del sistema de rejilla
 *	se debe de evitar su uso en la convolucion.
 *
 *  Integrando para la convolucion de A_rz
 *  r2 -> es r en la integracion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float A_rzFGIntegrand(float r2)
{
	float f;
	short nr = ConvVar.in_ptr->nr;
	double R, r, A_at_r2;
	LINK link;

	A_at_r2 = A_rzInterp(ConvVar.out_ptr->A_rz, r2);

	R = ConvVar.in_ptr->beam.R;
	r = ConvVar.r;
	if ((link = SearchNode(ConvVar.tree, r2)))	//f en el arbol
		f = link->y;
	else 
	{
		if (ConvVar.in_ptr->beam.type == flat)	//Haz tipo plano
			f = ITheta(r, r2, R);
		else									//Haz tipo Gaussiano
			f = ExpBessI0(r, r2, R);
		InsertNode(&ConvVar.tree, r2, f);
	}

	f *= A_at_r2 * r2;
	return (f);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Integrando de la convolucion para los haces plano & gaussiano.
 *	Ver los comentarios para A_rzFGIntegrand().
 *
 *  Integrando para la convolucion de Rd_ra 
 *  r2 -> es r" en la integracion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float Rd_raFGIntegrand(float r2)
{
	float f;
	short nr = ConvVar.in_ptr->nr;
	double R, r, Rd_at_r2;
	LINK link;

	Rd_at_r2 = RT_raInterp(ConvVar.out_ptr->Rd_ra, r2);

	R = ConvVar.in_ptr->beam.R;
	r = ConvVar.r;
	if ((link = SearchNode(ConvVar.tree, r2)))	//f en el arbol
		f = link->y;
	else 
	{
		if (ConvVar.in_ptr->beam.type == flat)	//haz plano
			f = ITheta(r, r2, R);
		else									//haz Gaussiano
			f = ExpBessI0(r, r2, R);
		InsertNode(&ConvVar.tree, r2, f);
	}

	f *= Rd_at_r2 * r2;
	return (f);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Integrando para la convolucion para los haces plano y gaussiano.
 *	Ver los comentarios para A_rzFGIntegrand().
 *
 *  Integrando para la convolucion de Rd_r 
 *  r2 -> r en la integracion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float Rd_rFGIntegrand(float r2)
{
	float f;
	short nr = ConvVar.in_ptr->nr;
	double R, r, Rd_at_r2;

	Rd_at_r2 = RT_rInterp(ConvVar.out_ptr->Rd_r, r2);

	R = ConvVar.in_ptr->beam.R;
	r = ConvVar.r;
	if (ConvVar.in_ptr->beam.type == flat)		//haz plano
		f = Rd_at_r2 * ITheta(r, r2, R) * r2;
	else										//haz gaussiano
		f = Rd_at_r2 * ExpBessI0(r, r2, R) * r2;

	return (f);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Integrando para la convolucion para los haces plano y gaussiano.
 *	Ver los comentarios para A_rzFGIntegrand().
 *
 *  Integrando para la convolucion de Tt_ra 
 *  r2 -> r en la integracion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float Tt_raFGIntegrand(float r2)
{
	float f;
	short nr = ConvVar.in_ptr->nr;
	double R, r, Tt_at_r2;
	LINK link;

	Tt_at_r2 = RT_raInterp(ConvVar.out_ptr->Tt_ra, r2);

	R = ConvVar.in_ptr->beam.R;
	r = ConvVar.r;
	if ((link = SearchNode(ConvVar.tree, r2)))	/* f in tree. */
		f = link->y;
	else 
	{
		if (ConvVar.in_ptr->beam.type == flat)	//haz plano
			f = ITheta(r, r2, R);
		else									//haz gaussiano
			f = ExpBessI0(r, r2, R);
		InsertNode(&ConvVar.tree, r2, f);
	}

	f *= Tt_at_r2 * r2;
	return (f);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Integrando para la convolucion para los haces plano y gaussiano.
 *	Ver los comentarios para A_rzFGIntegrand().
 *
 *  Integrando para la convolucion de Tt_r 
 *  r2 -> r en la integracion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
float Tt_rFGIntegrand(float r2)
{
	float f;
	short nr = ConvVar.in_ptr->nr;
	double R, r, Tt_at_r2;

	Tt_at_r2 = RT_rInterp(ConvVar.out_ptr->Tt_r, r2);

	R = ConvVar.in_ptr->beam.R;
	r = ConvVar.r;
	if (ConvVar.in_ptr->beam.type == flat)		//haz plano
		f = Tt_at_r2 * ITheta(r, r2, R) * r2;
	else										//haz gaussiano
		f = Tt_at_r2 * ExpBessI0(r, r2, R) * r2;

	return (f);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Integracion del haz plano.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double FlatIntegration(float (*Func) (float))
{
	double rc = ConvVar.r;
	double R = ConvVar.in_ptr->beam.R;
	double b_max = (ConvVar.in_ptr->nr - 0.5) * ConvVar.in_ptr->dr;
	double a = MAX(0, rc - R);
	double b = MIN(b_max, rc + R);

	if (a >= b)
		return (0);
	else
		return (qtrap(Func, a, b, ConvVar.in_ptr->eps));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Integracion del haz Gaussiano.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double GaussIntegration(float (*Func) (float))
{
	double rc = ConvVar.r;
	double R = ConvVar.in_ptr->beam.R;
	double b_max = (ConvVar.in_ptr->nr - 0.5) * ConvVar.in_ptr->dr;
	double a = MAX(0, rc - GAUSSLIMIT * R);
	double b = MIN(b_max, rc + GAUSSLIMIT * R);

	if (a >= b)
		return (0);
	else
		return (qtrap(Func, a, b, ConvVar.in_ptr->eps));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Convolucion de A_rz.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvA_rz(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short irc, iz;
	double rc, P = In_Ptr->beam.P, R = In_Ptr->beam.R;

	puts("La convolucion puede tarda un poco de tiempo. Espere...");
	for (irc = 0; irc < In_Ptr->nrc; irc++) 
	{
		rc = (irc + 0.5) * In_Ptr->drc;
		ConvVar.r = rc;
		ConvVar.tree = NULL;					//Iniciar el arbol
		for (iz = 0; iz < In_Ptr->nz; iz++) 
		{
			ConvVar.iz = iz;
			if (In_Ptr->beam.type == flat)		//Para el haz plano
				Out_Ptr->A_rzc[irc][iz] = 2 * P / (R * R) * FlatIntegration(A_rzFGIntegrand);
			else								//Para el haz gaussiano
				Out_Ptr->A_rzc[irc][iz] = 4 * P / (R * R) * GaussIntegration(A_rzFGIntegrand);
		}
		FreeTree(ConvVar.tree);					//Liberar el arbol para los sigueintes calculos
		
		// Mostrar algo para saber en que punto va la convolución
		// gotoxy();
		// printf("");
	}

	Out_Ptr->conved.A_rz = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Convolucion de Rd_ra.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvRd_ra(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short irc, ia;
	double rc, P = In_Ptr->beam.P, R = In_Ptr->beam.R;
	double b_max = (In_Ptr->nr - 1) * In_Ptr->dr;

	puts("La convolucion puede tarda un poco de tiempo. Espere...");
	for (irc = 0; irc < In_Ptr->nrc; irc++) 
	{
		rc = (irc + 0.5) * In_Ptr->drc;
		ConvVar.r = rc;
		ConvVar.tree = NULL;				//Iniciar el arbol
		for (ia = 0; ia < In_Ptr->na; ia++) 
		{
			ConvVar.ia = ia;
			if (In_Ptr->beam.type == flat)	//Para el haz plano
				Out_Ptr->Rd_rac[irc][ia] = 2 * P / (R * R) * FlatIntegration(Rd_raFGIntegrand);
			else							//Para el haz gaussiano
				Out_Ptr->Rd_rac[irc][ia] = 4 * P / (R * R) * GaussIntegration(Rd_raFGIntegrand);
		}
		FreeTree(ConvVar.tree);				//Liberar el arbol para los sigueintes calculos
	}

	Out_Ptr->conved.Rd_ra = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Convolucion de Rd_r.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvRd_r(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short irc;
	double rc, P = In_Ptr->beam.P, R = In_Ptr->beam.R;
	double b_max = (In_Ptr->nr - 1) * In_Ptr->dr;

	puts("La convolucion puede tarda un poco de tiempo. Espere...");
	printf("%d", In_Ptr->nrc);
	for (irc = 0; irc < In_Ptr->nrc; irc++) 
	{
		rc = (irc + 0.5) * In_Ptr->drc;
		ConvVar.r = rc;
		if (In_Ptr->beam.type == flat)		//Para el haz plano
			Out_Ptr->Rd_rc[irc] = 2 * P / (R * R) * FlatIntegration(Rd_rFGIntegrand);
		else								//Para el haz gaussiano
			Out_Ptr->Rd_rc[irc] = 4 * P / (R * R) * GaussIntegration(Rd_rFGIntegrand);
		
		/*Mostrar unos asteriscos para indicar que está trabajando*/
		if( ((irc/In_Ptr->nrc)*100)%10 == 0 )
		{
			printf("*");
		}
	}

	Out_Ptr->conved.Rd_r = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Convolucion de Tt_ra.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvTt_ra(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short irc, ia;
	double rc, P = In_Ptr->beam.P, R = In_Ptr->beam.R;
	double b_max = (In_Ptr->nr - 1) * In_Ptr->dr;

	puts("La convolucion puede tarda un poco de tiempo. Espere...");
	for (irc = 0; irc < In_Ptr->nrc; irc++) 
	{
		rc = (irc + 0.5) * In_Ptr->drc;
		ConvVar.r = rc;
		ConvVar.tree = NULL;				//Iniciar el arbol
		for (ia = 0; ia < In_Ptr->na; ia++) 
		{
			ConvVar.ia = ia;
			if (In_Ptr->beam.type == flat)	//Para el haz plano
				Out_Ptr->Tt_rac[irc][ia] = 2 * P / (R * R) * FlatIntegration(Tt_raFGIntegrand);
			else							//Para el haz Gaussiano
				Out_Ptr->Tt_rac[irc][ia] = 4 * P / (R * R) * GaussIntegration(Tt_raFGIntegrand);
		}
		FreeTree(ConvVar.tree);
	}

	Out_Ptr->conved.Tt_ra = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Convolucion Tt_r.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ConvTt_r(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short irc;
	double rc, P = In_Ptr->beam.P, R = In_Ptr->beam.R;
	double b_max = (In_Ptr->nr - 1) * In_Ptr->dr;

	puts("La convolucion puede tarda un poco de tiempo. Espere...");
	for (irc = 0; irc < In_Ptr->nrc; irc++) 
	{
		rc = (irc + 0.5) * In_Ptr->drc;
		ConvVar.r = rc;
		if (In_Ptr->beam.type == flat)		//Para el haz plano
			Out_Ptr->Tt_rc[irc] = 2 * P / (R * R) * FlatIntegration(Tt_rFGIntegrand);
		else								//Para el haz Gaussiano
			Out_Ptr->Tt_rc[irc] = 4 * P / (R * R) * GaussIntegration(Tt_rFGIntegrand);
	}

  Out_Ptr->conved.Tt_r = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Mostrar el menu de convolcuion al pedir la ayuda.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowOutConvMenu(char *in_fname)
{
	printf("Arz = Absorcion vs r & z [J/cm3]\n");
	printf("Frz = Fluencia vs r & z [J/cm2]\n");

	printf("Rr  = Reflectancia difusa vs radio r [J/cm2]\n");
	printf("Rra = Reflectancia difusa vs radio & angulo [J/(cm2 sr)]\n");

	printf("Tr  = Transmitancia vs radio r [J/cm2]\n");
	printf("Tra = Transmitancia vs radio & angulo [J/(cm2 sr)]\n");

	printf("Q   = Salir al menu principal\n");
	printf("* ingresar el nombre del archivo: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de A_rz
 *  3 numeros por cada linea r, z, A[r][z].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_rzc(InputStruct * In_Ptr, double **A_rzc)
{
	FILE *file;
	short ir, iz, nr = In_Ptr->nr, nz = In_Ptr->nz;
	double r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Arz");
	#else
		strcpy(fname, "Arzc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-12s\t%-s[J/cm3]\n", "r[cm]", "z[cm]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (iz = 0; iz < nz; iz++) 
		{
			z = (iz + 0.5) * dz;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, A_rzc[ir][iz]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de F_rz
 *  3 numeros por cada linea r, z, F[r][z].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteF_rzc(InputStruct * In_Ptr, double **A_rzc)
{
	FILE *file;
	short ir, iz, nr = In_Ptr->nr, nz = In_Ptr->nz;
	double mua, r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Frz");
	#else
		strcpy(fname, "Frzc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-12s\t%-s[J/cm3]\n", "r[cm]", "z[cm]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (iz = 0; iz < nz; iz++) 
		{
			z = (iz + 0.5) * dz;
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, A_rzc[ir][iz] / mua);
			else
				fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, 0.0);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de Rd_ra
 *  3 numeros por cada linea r, a, Rd[r][z].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_rac(InputStruct * In_Ptr, double **Rd_rac)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Rra");
	#else
		strcpy(fname, "Rrac");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-12s\t%-s[J/(cm2sr)]\n", "r[cm]", "a[rad]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (ia = 0; ia < na; ia++) 
		{
			a = (ia + 0.5) * da;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, a, Rd_rac[ir][ia]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de Rd_r
 *  2 numeros por cada linea r, Rd[r].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_rc(InputStruct * In_Ptr, double *Rd_rc)
{
	short ir, nr = In_Ptr->nr;
	double dr = In_Ptr->dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Rrc");
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-s[J/cm2]\n", "r[cm]", fname);
	for (ir = 0; ir < nr; ir++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ir + 0.5) * dr, Rd_rc[ir]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de Rd_r
 *  2 numeros por cada linea r, Rd[r].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_rc2(InputStruct * In_Ptr, double *Rd_rc, char *archon)
{
	short ir, nr = In_Ptr->nr;
	double dr = In_Ptr->dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Rrc");
	file = GetWriteFile2(fname, archon);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-s[J/cm2]\n", "r[cm]", fname);
	for (ir = 0; ir < nr; ir++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ir + 0.5) * dr, Rd_rc[ir]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de Tt_ra
 *  3 numeros por cada linea r, a, Tt[r][a].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_rac(InputStruct * In_Ptr, double **Tt_rac)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Tra");
	#else
		strcpy(fname, "Trac");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-12s\t%-s[J/(cm2sr)]\n", "r[cm]", "a[rad]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (ia = 0; ia < na; ia++) 
		{
			a = (ia + 0.5) * da;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, a, Tt_rac[ir][ia]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir en el archivo la convolucion de Tt_r
 *  2 numeros por cada linea r, Tt[r].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_rc(InputStruct * In_Ptr, double *Tt_rc)
{
	short ir, nr = In_Ptr->nr;
	double dr = In_Ptr->dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Trc");
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	fprintf(file, "%-12s\t%-s[J/cm2]\n", "r[cm]", fname);
	for (ir = 0; ir < nr; ir++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ir + 0.5) * dr, Tt_rc[ir]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion de Absorcion del menu.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvA(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':
			if (toupper(Cmd_Str[2]) == 'Z') 	//Absorcion vs r & z
			{
				if (!Out_Ptr->conved.A_rz)
					ConvA_rz(In_Ptr, Out_Ptr);
				WriteA_rzc(In_Ptr, Out_Ptr->A_rzc);
			} 
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion de Fluencia del menu.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvF(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':
			if (toupper(Cmd_Str[2]) == 'Z')		//Fluencia vs r & z
			{
				if (!Out_Ptr->conved.A_rz)
					ConvA_rz(In_Ptr, Out_Ptr);
				WriteF_rzc(In_Ptr, Out_Ptr->A_rzc);
			}
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion de Reflectancia difusa del menu.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvR(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':
			ch = toupper(Cmd_Str[2]);
			if (ch == '\0') 					//Reflectancia difusa vs radio r
			{
				if (!Out_Ptr->conved.Rd_r)
					ConvRd_r(In_Ptr, Out_Ptr);
				WriteRd_rc(In_Ptr, Out_Ptr->Rd_rc);
			} 
			else if (ch == 'A') 				//Reflectancia difusa vs radio & angulo
			{
				if (!Out_Ptr->conved.Rd_ra)
					ConvRd_ra(In_Ptr, Out_Ptr);
				WriteRd_rac(In_Ptr, Out_Ptr->Rd_rac);
			} 
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion de Reflectancia difusa del menu.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvR2(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr, char *archon)
{
	char ch;

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':
			ch = toupper(Cmd_Str[2]);
			if (ch == '\0') 					//Reflectancia difusa vs radio r
			{
				if (!Out_Ptr->conved.Rd_r)
					ConvRd_r(In_Ptr, Out_Ptr);
				WriteRd_rc2(In_Ptr, Out_Ptr->Rd_rc, archon);
			} 
			else if (ch == 'A') 				//Reflectancia difusa vs radio & angulo
			{
				if (!Out_Ptr->conved.Rd_ra)
					ConvRd_ra(In_Ptr, Out_Ptr);
				WriteRd_rac(In_Ptr, Out_Ptr->Rd_rac);
			} 
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion de Transmitancia del menu.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvT(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':
			ch = toupper(Cmd_Str[2]);
			if (ch == '\0') 					//Transmitancia vs radio r
			{
				if (!Out_Ptr->conved.Tt_r)
					ConvTt_r(In_Ptr, Out_Ptr);
				WriteTt_rc(In_Ptr, Out_Ptr->Tt_rc);
			} 
			else if (ch == 'A') 				//Transmitancia vs radio & angulo
			{
				if (!Out_Ptr->conved.Tt_ra)
					ConvTt_ra(In_Ptr, Out_Ptr);
				WriteTt_rac(In_Ptr, Out_Ptr->Tt_rac);
			} 
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Se utiliza la variable global ConvVar
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	ConvVar.in_ptr = In_Ptr;
	ConvVar.out_ptr = Out_Ptr;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':	//Seccion de Absorcion
			BranchOutConvA(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'F':	//Seccion de Fluencia
			BranchOutConvF(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'R':	//Seccion de Reflectancia difusa
			BranchOutConvR(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'T':	//Seccion de Transmitancia
			BranchOutConvT(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'H':	//Seccion de ayuda
			ShowOutConvMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir del menu de convolucion
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Se utiliza la variable global ConvVar
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutConvCmd2(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr, char *archon)
{
	char ch;

	ConvVar.in_ptr = In_Ptr;
	ConvVar.out_ptr = Out_Ptr;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':	//Seccion de Absorcion
			BranchOutConvA(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'F':	//Seccion de Fluencia
			BranchOutConvF(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'R':	//Seccion de Reflectancia difusa
			BranchOutConvR2(Cmd_Str, In_Ptr, Out_Ptr, archon);
			break;
		case 'T':	//Seccion de Transmitancia
			BranchOutConvT(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'H':	//Seccion de ayuda
			ShowOutConvMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir del menu de convolucion
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  .
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OutputConvData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else if (In_Ptr->beam.type == pencil)
		puts("...No se ha especificado un haz incidente");
	else
		do 
		{
			printf("\n> Datos de salida convulocionados (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str));	//Evitar la cadena NULL
			BranchOutConvCmd(cmd_str, In_Ptr, Out_Ptr);
		} 
		while (toupper(cmd_str[0]) != 'Q');
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  .
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OutputConvData2(InputStruct * In_Ptr, OutStruct * Out_Ptr, char *archon)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else if (In_Ptr->beam.type == pencil)
		puts("...No se ha especificado un haz incidente");
	else
	{
		//printf("\n> Datos de salida convulocionados (h para ayuda) => ");
		do
			strcpy(cmd_str,"Rr");
		while (!strlen(cmd_str));	//Evitar la cadena NULL
		BranchOutConvCmd2(cmd_str, In_Ptr, Out_Ptr, archon);
	}
}

/* * * * * * * * CONTORNOS * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Densidad de absorcion a fluencia.  A = F /mua;
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void A2Fconv(InputStruct * In_Ptr, double **A_rz)
{
	short nz = In_Ptr->nz, nrc = In_Ptr->nrc;
	short ir, iz;
	double mua;

	for (ir = 0; ir < nrc; ir++)
		for (iz = 0; iz < nz; iz++) 
		{
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				A_rz[ir][iz] /= mua;
		}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Fluencia a densidad de absorcion.  F = A * mua;
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void F2Aconv(InputStruct * In_Ptr, double **A_rz)
{
	short nz = In_Ptr->nz, nrc = In_Ptr->nrc;
	short ir, iz;
	double mua;

	for (ir = 0; ir < nrc; ir++)
		for (iz = 0; iz < nz; iz++) 
		{
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				A_rz[ir][iz] *= mua;
		}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Mostrar el menu de contornos de convolucion (mostrado con ayuda).
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowContConvMenu(char *in_fname)
{
	printf("A = Absorcion vs r & z [J/cm3]\n");
	printf("F = Fluencia vs r & z [J/cm2]\n");
	printf("R = Reflectance difusa vs radio and angulo [J/(cm2 sr)]\n");
	printf("T = Transmitancia vs radio and angulo [J/(cm2 sr)]\n");
	printf("Q   = Salir al menu principal\n");
	printf("* Ingresar nombre de archivo: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion del menu de contornos de convolucion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchContConvCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	ConvVar.in_ptr = In_Ptr;
	ConvVar.out_ptr = Out_Ptr;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':	//Absorcion vs r & z
			if (!Out_Ptr->conved.A_rz)
				ConvA_rz(In_Ptr, Out_Ptr);
			IsoPlot(Out_Ptr->A_rzc, In_Ptr->nrc - 1, In_Ptr->nz - 1, In_Ptr->drc, In_Ptr->dz);
			break;
		case 'F':	//Fluencia vs r & z
			if (!Out_Ptr->conved.A_rz)
				ConvA_rz(In_Ptr, Out_Ptr);
			A2Fconv(In_Ptr, Out_Ptr->A_rzc);
			IsoPlot(Out_Ptr->A_rzc, In_Ptr->nrc - 1, In_Ptr->nz - 1, In_Ptr->drc, In_Ptr->dz);
			F2Aconv(In_Ptr, Out_Ptr->A_rzc);
			break;
		case 'R':	//Reflectance difusa vs radio and angulo
			if (!Out_Ptr->conved.Rd_ra)
				ConvRd_ra(In_Ptr, Out_Ptr);
			IsoPlot(Out_Ptr->Rd_rac, In_Ptr->nrc - 1, In_Ptr->na - 1, In_Ptr->drc, In_Ptr->da);
			break;
		case 'T':	//Transmitancia vs radio and angulo
			if (!Out_Ptr->conved.Tt_ra)
				ConvTt_ra(In_Ptr, Out_Ptr);
			IsoPlot(Out_Ptr->Tt_rac, In_Ptr->nrc - 1, In_Ptr->na - 1, In_Ptr->drc, In_Ptr->da);
			break;
		case 'H':	//Ayuda
			ShowContConvMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir del menu
			break;
		default:
			puts("...Comando erroneo");
	}
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  .
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ContourConvData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else if (In_Ptr->beam.type == pencil)
		puts("...No se ha especificado el haz incidente");
	else
		do 
		{
			printf("\n> Contornos de salida de los datos convulocionados (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str));	//Evitar la cadena NULL
			BranchContConvCmd(cmd_str, In_Ptr, Out_Ptr);
		} 
		while (toupper(cmd_str[0]) != 'Q');
}

/* * * * * * * * ESCANEO/ANALISIS * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  .
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowScanConvMenu(char *in_fname)
{
	printf("Ar = Absorcion vs r @ z fija [J/cm3]\n");
	printf("Az = Absorcion vs z @ r fija [J/cm3]\n");
	printf("Fr = Fluencia vs r @ z fija [J/cm2]\n");
	printf("Fz = Fluencia vs z @ r fija [J/cm2]\n");
	printf("Rr = Reflectancia difusa vs r @ angulo fijo [J/(cm2 sr)]\n");
	printf("Ra = Reflectancia difusa vs angulo @ r fijo [J/(cm2 sr)]\n");
	printf("Tr = Transmitancia vs r @ angulo fijo [J/(cm2 sr)]\n");
	printf("Ta = Transmitancia vs angulo @ r fijo [J/(cm2 sr)]\n");
	printf("Q  = Salir\n");
	printf("* input filename: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de A_r
 *  Ext -> Es tanto "Ars" o "Frs".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvA_r(char *Ext, InputStruct * In_Ptr, double **A_rzc)
{
	short irc, iz, nrc = In_Ptr->nrc, nz = In_Ptr->nz;
	double r, z, drc = In_Ptr->drc, dz = In_Ptr->dz;
	FILE *file;

	file = GetWriteFile(Ext);
	printf("\nLlego archivo %s", Ext);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", Ext);
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(Ext,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",Ext);
				break;
			}
		}
		//return;
	}

	printf("Separacion de la rejilla en z es %-10.4lg cm.\n", dz);
	printf("Ingresar el indice de z fijo (0 - %2hd): ", nz - 1);
	iz = GetShort(0, nz - 1);
	fprintf(file, "%-12s\t%-s@z=%-9.3lg\n", "r[cm]", Ext, dz * (iz + 0.5));
	for (irc = 0; irc < nrc; irc++) 
	{
		r = (irc + 0.5) * drc;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, A_rzc[irc][iz]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de A_z
 *  Ext -> Es tanto "Azs" o "Fzs".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvA_z(char *Ext, InputStruct * In_Ptr, double **A_rzc)
{
	short irc, iz, nrc = In_Ptr->nrc, nz = In_Ptr->nz;
	double r, z, drc = In_Ptr->drc, dz = In_Ptr->dz;
	FILE *file;

	file = GetWriteFile(Ext);
	printf("\nLlego archivo %s", Ext);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", Ext);
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(Ext,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",Ext);
				break;
			}
		}
		//return;
	}

	printf("Separacion de la rejilla en z es %-10.4lg cm.\n", drc);
	printf("Ingresar el indice de r fijo (0 - %2hd): ", nrc - 1);
	irc = GetShort(0, nrc - 1);
	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "z[cm]", Ext, drc * (irc + 0.5));
	for (iz = 0; iz < nz; iz++) 
	{
		z = (iz + 0.5) * dz;
		fprintf(file, "%-12.4E\t%-12.4E\n", z, A_rzc[irc][iz]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de Rd_z
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvRd_r(InputStruct * In_Ptr, double **Rd_rac)
{
	short irc, ia, nrc = In_Ptr->nrc, na = In_Ptr->na;
	double r, a, drc = In_Ptr->drc, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Rrs");
	#else
		strcpy(fname, "Rrsc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	printf("Separacion angular de la rejilla es %-10.4lg rad.\n", da);
	printf("Ingresar el indice de angulo fijo (0 - %2hd): ", na - 1);
	ia = GetShort(0, na - 1);

	fprintf(file, "%-12s\t%-s@a=%-9.3lg\n", "r[cm]", fname, da * (ia + 0.5));
	for (irc = 0; irc < nrc; irc++) 
	{
		r = (irc + 0.5) * drc;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, Rd_rac[irc][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de Rd_a
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvRd_a(InputStruct * In_Ptr, double **Rd_rac)
{
	short irc, ia, nrc = In_Ptr->nrc, na = In_Ptr->na;
	double r, a, drc = In_Ptr->drc, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Ras");
	#else
		strcpy(fname, "Rasc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	printf("Separacion de la rejilla en r es %-10.4lg cm.\n", drc);
	printf("Ingresar el indice de r fijo (0 - %2hd): ", nrc - 1);
	irc = GetShort(0, nrc - 1);

	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "a[rad]", fname, drc * (irc + 0.5));
	for (ia = 0; ia < na; ia++) 
	{
		a = (ia + 0.5) * da;
		fprintf(file, "%-12.4E\t%-12.4E\n", a, Rd_rac[irc][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de Tt_r
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvTt_r(InputStruct * In_Ptr, double **Tt_rac)
{
	short irc, ia, nrc = In_Ptr->nrc, na = In_Ptr->na;
	double r, a, drc = In_Ptr->drc, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Trs");
	#else
		strcpy(fname, "Trsc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	printf("Separacion angular de la rejilla es %-10.4lg rad.\n", da);
	printf("Ingresar el indice de angulo fijo (0 - %2hd): ", na - 1);
	ia = GetShort(0, na - 1);

	fprintf(file, "%-12s\t%-s@a=%-9.3lg\n", "r[cm]", fname, da * (ia + 0.5));
	for (irc = 0; irc < nrc; irc++) 
	{
		r = (irc + 0.5) * drc;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, Tt_rac[irc][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Analisis de la convolucion de Tt_a
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvTt_a(InputStruct * In_Ptr, double **Tt_rac)
{
	short irc, ia, nrc = In_Ptr->nrc, na = In_Ptr->na;
	double r, a, drc = In_Ptr->drc, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	#if IBMPC
		strcpy(fname, "Tas");
	#else
		strcpy(fname, "Tasc");
	#endif
	file = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (file == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(file); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			file=fopen(fname,"w");
			if(file != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	printf("Separacion de la rejilla en r es %-10.4lg cm.\n", drc);
	printf("Ingresar el indice de r fijo (0 - %2hd): ", nrc - 1);
	irc = GetShort(0, nrc - 1);

	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "a[rad]", fname, drc * (irc + 0.5));
	for (ia = 0; ia < na; ia++) 
	{
		a = (ia + 0.5) * da;
		fprintf(file, "%-12.4E\t%-12.4E\n", a, Tt_rac[irc][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion del menu para el analisis de la convolucion de la Absorcion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanConvA(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char fname[STRLEN];

	if (!Out_Ptr->conved.A_rz)
		ConvA_rz(In_Ptr, Out_Ptr);

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':			//Absorcion vs r @ z fija
			#if IBMPC		//En caso de que sea una PC (extension de 3 chars)
				strcpy(fname, "Ars");
				ScanConvA_r(fname, In_Ptr, Out_Ptr->A_rzc);
			#else			//Otras computadoras (Linux, Mac)
				strcpy(fname, "Arsc");
				ScanConvA_r(fname, In_Ptr, Out_Ptr->A_rzc);
			#endif
			break;
		case 'Z':			//Absorcion vs z @ r fija
			#if IBMPC
				strcpy(fname, "Azs");
				ScanConvA_z(fname, In_Ptr, Out_Ptr->A_rzc);
			#else
				strcpy(fname, "Azsc");
				ScanConvA_z(fname, In_Ptr, Out_Ptr->A_rzc);
			#endif
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion del menu para el analisis de la convolucion de la Fluencia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanConvF(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char fname[STRLEN];

	if (!Out_Ptr->conved.A_rz)
		ConvA_rz(In_Ptr, Out_Ptr);
	A2Fconv(In_Ptr, Out_Ptr->A_rzc);

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':			//Fluencia vs r @ z fija
			#if IBMPC
				strcpy(fname, "Frs");
				ScanConvA_r(fname, In_Ptr, Out_Ptr->A_rzc);
			#else
				strcpy(fname, "Frsc");
				ScanConvA_r(fname, In_Ptr, Out_Ptr->A_rzc);
			#endif
			break;
		case 'Z':			//Fluencia vs z @ r fija
			#if IBMPC
				strcpy(fname, "Fzs");
				ScanConvA_z(fname, In_Ptr, Out_Ptr->A_rzc);
			#else
				strcpy(fname, "Fzsc");
				ScanConvA_z(fname, In_Ptr, Out_Ptr->A_rzc);
			#endif
			break;
		default:
			puts("...Comando erroneo");
  }

  F2Aconv(In_Ptr, Out_Ptr->A_rzc);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion del menu para el analisis de la convolucion de la Reflectancia difusa.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanConvR(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	if (!Out_Ptr->conved.Rd_ra)
		ConvRd_ra(In_Ptr, Out_Ptr);
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':			//Reflectancia difusa vs r @ angulo fijo
			ScanConvRd_r(In_Ptr, Out_Ptr->Rd_rac);
			break;
		case 'A':			//Reflectancia difusa vs angulo @ r fijo
			ScanConvRd_a(In_Ptr, Out_Ptr->Rd_rac);
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seccion del menu para el analisis de la convolucion de la Transmitancia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanConvT(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	if (!Out_Ptr->conved.Tt_ra)
		ConvTt_ra(In_Ptr, Out_Ptr);
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':			//Transmitancia vs r @ angulo fijo
			ScanConvTt_r(In_Ptr, Out_Ptr->Tt_rac);
			break;
		case 'A':			//Transmitancia vs angulo @ r fijo
			ScanConvTt_a(In_Ptr, Out_Ptr->Tt_rac);
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para el analisis de la convolucion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanConvCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	ConvVar.in_ptr = In_Ptr;
	ConvVar.out_ptr = Out_Ptr;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':			//Seccion de Absorcion
			BranchScanConvA(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'F':			//Seccion de la Fluencia
			BranchScanConvF(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'R':			//Seccion de la Reflectancia Difusa
			BranchScanConvR(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'T':			//Seccion de la Transmitancia
			BranchScanConvT(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'H':			//Ayuda
			ShowScanConvMenu(In_Ptr->in_fname);
			break;
		case 'Q':			//Salir del menu
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  .
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanConvData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else if (In_Ptr->beam.type == pencil)
		puts("...No se ha especificado el haz incidente");
	else
		do 
		{
			printf("\n> Contornos de salida de los datos convulocionados (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str));	/* avoid null string. */
			BranchScanConvCmd(cmd_str, In_Ptr, Out_Ptr);
		} 
		while (toupper(cmd_str[0]) != 'Q');
}