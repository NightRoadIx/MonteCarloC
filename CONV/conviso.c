/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  En este programa se grafica una serie de lineas de contorno
 *  para un arreglo double 2D.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * LIBRERIAS INCLUIDAS * * * * * * * */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

//Valor de pi utilizado en el programa
#ifndef PI
#define PI 3.1415926
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresar true si val esta entre Z[i][j] & Z[i+1][j].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define INTERSECTI(val, Z, i, j) \
  (Z[i][j] <= val && val <= Z[i+1][j] || \
   Z[i][j] >= val && val >= Z[i+1][j])

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresar true si val esta entre Z[i][j] & Z[i][j+1].
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define INTERSECTJ(val, Z, i, j) \
  (Z[i][j] <= val && val <= Z[i][j+1] || \
   Z[i][j] >= val && val >= Z[i][j+1])

FILE *GetWriteFile(char *Ext);	//Funcion definida en convo.c

/* * * * * * * * ESTRUCTURAS UTILIZADAS * * * * * * * */
struct XY 
{
	double x, y;
};

struct XYpairs 
{
	double x, y;
	struct XYpairs *next;
};

/* Definicion de estructuras */
typedef struct XYpairs *PairList;

//Configura una cola para la impresion de los pares
typedef PairList QDATA;

struct Qlinked_list 
{
	QDATA d;
	struct Qlinked_list *next;
};

typedef struct Qlinked_list QELEMENT; //definir el nombre de la estructura
typedef QELEMENT *QLINK;

//Una cola de pares
typedef struct 
{
	QLINK front, rear;
} QUE;

struct Isolines 
{
	double iso_val;	//Valores z para la linea de contorno
	PairList pairs;
	struct Isolines *next;
};
typedef struct Isolines *IsoList; //definir el nombre de la estructura

/* fin de las definiciones */

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener el valor minimo de z.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double ZMin(double **Z, long IXmax, long IYmax, double Dx, double Dy, struct XY * Pmin_Ptr)
{
	double zmin;
	long i, j;

	zmin = Z[0][0];
	for (i = 0; i < IXmax; i++)
		for (j = 0; j < IYmax; j++)
			if (Z[i][j] < zmin) 
			{
				zmin = Z[i][j];
				Pmin_Ptr->x = i * Dx;
				Pmin_Ptr->y = j * Dy;
			}
	return (zmin);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener el valor maximo de z.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double ZMax(double **Z, long IXmax, long IYmax, double Dx, double Dy, struct XY * Pmax_Ptr)
{
	double zmax;
	long i, j;

	zmax = Z[0][0];
	for (i = 0; i < IXmax; i++)
		for (j = 0; j < IYmax; j++)
			if (Z[i][j] > zmax) 
			{
				zmax = Z[i][j];
				Pmax_Ptr->x = i * Dx;
				Pmax_Ptr->y = j * Dy;
			}
	return (zmax);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener los isovalores del usuario.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
IsoList GetIsoValues(double Z_Min, double Z_Max)
{
	IsoList head;
	char in_str[256];

	printf("Ingresar un isovalor o presionar . para terminar: ");
	scanf("%s", in_str);
	//Si se ingresa o un 1 o '.' entonces sale
	if (strlen(in_str) == 1 && in_str[0] == '.')
		return (NULL);

	//Asignar memoria
	head = (IsoList) malloc(sizeof(struct Isolines));
	if (head == NULL)	//Si la asignacion fallo
		return (NULL);

	//Obtener los elementos para el nodo
	sscanf(in_str, "%lf", &head->iso_val);
	if (head->iso_val < Z_Min)
		head->iso_val = Z_Min;
	else if (head->iso_val > Z_Max)
		head->iso_val = Z_Max;
	head->pairs = NULL;
	head->next = GetIsoValues(Z_Min, Z_Max);
	return (head);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  La isoposicion esta entre [i][j] & [i+1][j]
 *
 *  Interpolacion lineal es utilizada para ubicar el componente x
 *  de la iso-posicion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void IsoPositionI(PairList pair, double iso_val, double **Z, long i, long j, double Dx, double Dy)
{
	pair->y = (j + 0.5) * Dy;
	if (Z[i][j] != Z[i + 1][j])
		pair->x = (i + 0.5) * Dx + Dx * (iso_val - Z[i][j]) / (Z[i + 1][j] - Z[i][j]);
	else
		pair->x = (i + 1) * Dx;	//Tomar el punto medio
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  La isoposicion esta entre [i][j] & [i][j+1]
 *
 *  Interpolacion lineal es utilizada para ubicar el componente y
 *  de la iso-posicion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void IsoPositionJ(PairList pair, double iso_val, double **Z, long i, long j, double Dx, double Dy)
{
	pair->x = (i + 0.5) * Dx;
	if (Z[i][j + 1] != Z[i][j])
		pair->y = (j + 0.5) * Dy + Dy * (iso_val - Z[i][j]) / (Z[i][j + 1] - Z[i][j]);
	else
		pair->y = (j + 1) * Dy;	//Tomar el punto medio
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Localizar la isoposicion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void IsoPosition(PairList pair, double iso_val, double **Z, long i, long j, double Dx, double Dy)
{
	if (INTERSECTI(iso_val, Z, i, j))
		IsoPositionI(pair, iso_val, Z, i, j, Dx, Dy);
	else if (INTERSECTJ(iso_val, Z, i, j))
		IsoPositionJ(pair, iso_val, Z, i, j, Dx, Dy);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Asigancion de los pares XY
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
PairList AllocPair(void)
{
	PairList pair;

	pair = (PairList) malloc(sizeof(struct XYpairs));
	if (pair == NULL)
		puts("...malloc error. Las lineas de contorno no estan completas");
	return (pair);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Obtencion de cada una de las iso lineas
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GetAnIsoLine(IsoList IsoNode, double **Z, long IXmax, long IYmax, double Dx, double Dy)
{
	long i, j;
	double ival = IsoNode->iso_val;
	PairList    pair_tail;

	for (j = 0; j < IYmax - 1; j++)
		for (i = 0; i < IXmax - 2; i++)
			if (INTERSECTI(ival, Z, i, j) || INTERSECTJ(ival, Z, i, j)) 
			{	//Encontrar el par
				if (IsoNode->pairs == NULL) 
				{	//El primer par
					if (!(IsoNode->pairs = AllocPair()))
						return;
					pair_tail = IsoNode->pairs;
				} 
				else 
				{	//Los pares subsecuentes
					if (!(pair_tail->next = AllocPair()))
						return;
					pair_tail = pair_tail->next;
				}
				IsoPosition(pair_tail, ival, Z, i, j, Dx, Dy);
			}
	pair_tail->next = NULL;	//Fin de la lista de pares
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresar el cuadrante.
 *	Para Frz o Arz, el punto maximo esta usualmente en el eje z
 *	el cual es el y aqui. Se desea iniciar la isolinea del 4to cuadrante.
 *	Por lo tanto, se usa -1 para el 4to cuadrante en lugar de 4.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
short GetQuadrant(double x, double y)
{
	if (x > 0 && y >= 0)
		return (1);			//Incluir el eje +x
	else if (x <= 0 && y > 0)
		return (2);			//Incluir el eje +y
	else if (x < 0 && y <= 0)
		return (3);			//Incluir el eje -x
	else if (x >= 0 && y < 0)
		return (-1);		//Incluir el eje -y
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Comparar el angulo wrt Pmax. Si el angulo de "This" es mayor
 *  que el de "Next", regesar 1. En otro caso, regresar 0.
 *
 *  A pesar de que atan2 regresa un valor en el intervalo -pi a pi
 *	se desea evitarlo por que produce una ejecucion lenta.
 *	Se compara los cuadrantes de los dos primeros puntos- Si estan en el
 *	mismo cuadrante se comparan las posiciones relativas.
 *
 *  PairList This -> El par actual
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
char OutOfOrder(PairList This, PairList Next, struct XY Pmax)
{
	double x0, y0, x1, y1;
	short q0, q1; //Cuadrantes
	char out_order;

	if (This == NULL || Next == NULL)
		return (0); //Esto se supone que nunca debe ocurrir, pero para evitar errores
	x0 = This->x - Pmax.x;
	y0 = This->y - Pmax.y;
	q0 = GetQuadrant(x0, y0);
	x1 = Next->x - Pmax.x;
	y1 = Next->y - Pmax.y;
	q1 = GetQuadrant(x1, y1);

	if (q0 < q1)
		out_order = 0;
	else if (q0 > q1)
		out_order = 1;
	else 
	{	//En el mismo cuadrante
		if (y0 * x1 < y1 * x0)
			out_order = 0;
		else
			out_order = 1;
	}

	return (out_order);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Ordenar las isoposiciones de acuerdo al angulo con
 *  respecto a la posicion del valor maximo.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SortAnIsoLine(PairList * PairHeadPtr, struct XY Pmax)
{
	char sorted = 0;
	PairList this, last; //this=el par que esta siendo comparado con el siguiente
	PairList sorted_head = NULL;	// La cabeza de la sublista de áres ordenados

	//Realizar el movimiento de las isoposiciones mientras que sorted & sorted_head sean
	//diferentes al valor del apuntador PairHeadPtr
	while (!sorted && sorted_head != *PairHeadPtr) 
	{
		sorted = 1;	//Asumir que la sublista esta ordenada
		last = NULL; //Al ultimo se le asigan NULL
		this = *PairHeadPtr; //La sublista comienza en *PairHeadPtr, termina en *sorted_head

		while (this->next != sorted_head) 
		{	//Cruzar la sublista
			if (OutOfOrder(this, this->next, Pmax)) 
			{	//Intercambiar y mover al siguiente
				sorted = 0;
				if (last == NULL)	//Si el ultimo es NULL
					*PairHeadPtr = this->next;
				else
					last->next = this->next;

				//Mover los elementos
				last = this->next;
				this->next = last->next;
				last->next = this;
			} 
			else 
			{	//Mover al siguiente
				last = this;
				this = this->next;
			}
		}	/*Finaliza el recorrido de la sublista*/

		sorted_head = this; //Se asigna el par comparado con el primer elemento
	}
}

#if 0
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Repetir el primer par de la isolinea en el fin de la lista
 *	de modo que la isolinea sea un lazo.
 *	Algunas veces no se desea que suceda esto, por ello el #if 0.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LoopAnIsoLine(PairList * PairHeadPtr)
{
	PairList tail;

	tail = *PairHeadPtr; //Se asigna el primer elemento de la lista
	//Mientras que el siguiente elemento de tail sea diferente a NULL
	//se va generando la lista
	while (tail->next != NULL)
		tail = tail->next;

	//Asignacion de memoria para los pares de la lista
	tail->next = (PairList) malloc(sizeof(struct XYpairs));
	//Si hay algun fallo en la asignacion de memoria
	if (tail->next == NULL)
		return;
	//Movilizar los elementos
	tail = tail->next;
	tail->x = (*PairHeadPtr)->x;
	tail->y = (*PairHeadPtr)->y;
	tail->next = NULL; //Hasta el ultimo de la lista
}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para la obtencion de las isolineas usando las funciones
 *  declaradas
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GetIsoLines(IsoList isos, double **Z, long IXmax, long IYmax, double Dx, double Dy, struct XY Pmax)
{
	IsoList node = isos;

	//Mientras en node exista un elemento
	while (node != NULL) 
	{
		//Obtener cada una de las isolineas
		GetAnIsoLine(node, Z, IXmax, IYmax, Dx, Dy);
		//Ordenar las isolineas
		SortAnIsoLine(&node->pairs, Pmax);
		/* LoopAnIsoLine(&node->pairs); */
		node = node->next; //Mover al siguiente elemento
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Verificar si la cola esta vacia
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
char IsEmpty(QUE q)
{
	return (q.front == NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Eliminar cada uno de los elementos de la lista
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Deque(QUE * q, QDATA * x)
{
	QLINK temp = q->front;

	if (!IsEmpty(*q)) 
	{
		*x = temp->d;
		q->front = temp->next;
		free(temp);
	} 
	else
		printf("Cola vacia.\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Asiganr el espacio de los elementos de la lista
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Enque(QUE * q, QDATA x)
{
	QLINK temp;

	//Asignacion de memoria de espacio QELEMENT del tipo QLINK
	temp = (QLINK) malloc(sizeof(QELEMENT));
	temp->d = x;
	temp->next = NULL;	//El siguiente elemento es NULL
	//Verificar que no este vacio
	if (IsEmpty(*q))
		//Si lo esta asigna el espacio recien asigando
		q->front = q->rear = temp;
	else 
	{	//En otro caso se trata del valor siguiente al que ya existia
		q->rear->next = temp;
		q->rear = temp;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir las iso lineas
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteIsoLines(FILE * Isofile, IsoList IsoHead)
{
	QUE qprint = {NULL, NULL};	//Variable tipo QUE  a ser impresa
	QUE qsave = {NULL, NULL};
	QDATA p;		//QDATA es una PairList
	char all_nulls;

	//Mientras exista algo en la lista
	while (IsoHead != NULL) 
	{
		//Imprimir
		fprintf(Isofile, "X%-8lG\tY%-8lG\t", IsoHead->iso_val, IsoHead->iso_val);
		//Asignar espacio a los elementos
		Enque(&qprint, IsoHead->pairs);
		//Apuntar al siguiente
		IsoHead = IsoHead->next;
	}
	fprintf(Isofile, "\n"); //Se escribe un retorno de carro '\n' para separar elementos

	do 
	{
		all_nulls = 1; //Iniciar all_nulls a 0
		//Realizar mientras qprint no este vacia
		while (!IsEmpty(qprint)) 
		{
			//Se van eliminando elementos
			Deque(&qprint, &p);
			if (p != NULL) 
			{
				fprintf(Isofile, "%9.2lE\t%9.2lE\t", p->x, p->y);
				Enque(&qsave, p->next);	//Añadir el siguiente al nuevo QUE para una nueva fila
				if (all_nulls == 1)
					all_nulls = 0;
			} 
			else 
			{
				/* fprintf(Isofile, "%9s\t%9s\t", " ", " "); */
				fprintf(Isofile, "\t\t"); //Imprimir tabulaciones para la separacion de elementos
				Enque(&qsave, p);	//Añadir un NULL a la nueva QUE
			}
		}
		fprintf(Isofile, "\n");
		qprint = qsave;
		qsave.front = NULL;
		qsave.rear = NULL;
	}
	while (!all_nulls); //Realizar todo mientras all_nulls no sea 0
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Liberar la memoria de los elementos pares (o de los pares)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreePairs(PairList Pairs)
{
	PairList this_pair;

	//Recorrer la lista para liberar memoria
	while (Pairs != NULL) 
	{
		this_pair = Pairs;
		Pairs = Pairs->next;
		free(this_pair);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Liberar la memoria de las Isolineas
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeIsoLines(IsoList IsoHead)
{
	IsoList this_iso;
	PairList pair_head;

	//Recorrer toda la lista para liberar memoria
	while (IsoHead != NULL) 
	{
		FreePairs(IsoHead->pairs);
		this_iso = IsoHead;
		IsoHead = IsoHead->next;
		free(this_iso);
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  
 *
 *  Z ->El arreglo 2D Z[i][j]
 *  IXmax -> 0<=i<=IXmax
 *  IYmax -> 0<=j<=IYmax
 *  Dx, Dy -> Las separaciones de la rejilla
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void IsoPlot(double **Z, long int IXmax, long int IYmax, double Dx, double Dy)
{
	FILE *isofile;	//Archivo para mandar las isolineas
	struct XY pmin, pmax; //Las posiciones xy de los minimos & maximos
	double zmin, zmax;
	IsoList isos;
	char fname[128] = "iso";

	//Localizar el mininmo y el maximo
	zmin = ZMin(Z, IXmax, IYmax, Dx, Dy, &pmin);
	zmax = ZMax(Z, IXmax, IYmax, Dx, Dy, &pmax);

	//En el caso de que zmin = zmax o la multiplicacion de IXmax & IYmax sea menor a 4
	if (zmin == zmax || IXmax * IYmax < 4) 
	{
		printf("...No hay suficientes datos para obtener la grafica de contorno\n");
		return;
	}
	//Abrir el archivo para escribir
	isofile = GetWriteFile(fname);
	printf("\nLlego archivo %s", fname);
	if (isofile == NULL)
	{
		printf("\nFallo al abrir archivo %s Error 2", fname);
		fclose(isofile); //Cerrar archivo para evitar errores de memoria
		//Si existiera algún error en la apertura del archivo, forzar su apertura
		//en modo de sobrescritura
		while(1)
		{
			isofile=fopen(fname,"w");
			if(isofile != NULL)
			{
				printf("\nArchivo %s finalmente abierto",fname);
				break;
			}
		}
		//return;
	}

	printf("El intervalo de los valores es %lf to %lf.\n", zmin, zmax);
	//Obtener los iso valores
	isos = GetIsoValues(zmin, zmax);

	//Obtener las iso lineas
	GetIsoLines(isos, Z, IXmax, IYmax, Dx, Dy, pmax);
	//Escribir los iso valores en el archivo
	WriteIsoLines(isofile, isos);

	//Liberar la memoria
	FreeIsoLines(isos);

	//Cerrar el archivo
	fclose(isofile);
}
