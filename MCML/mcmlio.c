/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Entrada/Salida de datos
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mcml.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *Estructura usada para revisar contra nombres de archivos duplicados
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct NameList {
  char name[STRLEN];
  struct NameList * next;
};
//Se utilizan para la asignacion dinamica de memoria
typedef struct NameList NameNode;
typedef NameNode * NameLink;

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Centrar una cadena de acuerdo al ancho de la columna
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
char *CenterStr(short  Wid, char * InStr, char * OutStr)
{
	size_t nspaces;	// nuermo de espacios a ser rellenados  antes de InStr
  
	nspaces = (Wid - strlen(InStr))/2;
	if(nspaces<0) nspaces = 0;
  
	strcpy(OutStr, "");
	while(nspaces--)  strcat(OutStr, " ");
  
	strcat(OutStr, InStr);
  
	return(OutStr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Imprimir algunos mensajes antes de que comience la simulacion
 *	e.g. autor, direccion, version del programa, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define COLWIDTH 80
void ShowVersion(char *version)
{
  char str[STRLEN];
  
  CenterStr(COLWIDTH,
	"MCML - Monte Carlo Simulation of Multi-layered Turbid Media",
	str);
  puts(str);
  puts("");

  CenterStr(COLWIDTH, "Lihong Wang, Ph. D.", str); 
  puts(str);

  CenterStr(COLWIDTH, "Steven L. Jacques, Ph. D.", str); 
  puts(str);

  CenterStr(COLWIDTH, 
	"Laser Biology Research Laboratory - Box 17",str); 
  puts(str);

  CenterStr(COLWIDTH, "University of Texas / "
			"M.D. Anderson Cancer Center", str); 
  puts(str);
  
  CenterStr(COLWIDTH, "Houston, TX 77030", str); 
  puts(str);

  CenterStr(COLWIDTH, "Phone: (713) 792-3664", str); 
  puts(str); 

  CenterStr(COLWIDTH, "Fax:   (713) 792-3995", str); 
  puts(str); 

  CenterStr(COLWIDTH, "e-mails: lihong@laser.mda.uth.tmc.edu", str); 
  puts(str); 
  CenterStr(COLWIDTH, "         or slj@laser.mda.uth.tmc.edu", str); 
  puts(str); 
  puts("");

  CenterStr(COLWIDTH, version, str); 
  puts(str);
  puts("\n\n\n\n");	
}
#undef COLWIDTH

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener un nombre de archivo y abrirlo para lectura
 *	intentar de nuevo hasta que el archivo pueda ser abierto
 *  '.' termina el programa
 *      
 *	If Fname != NULL, try Fname first.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
FILE *GetFile(char *Fname)
{
	FILE * file=NULL;
	Boolean firsttime=1;
  
	do 
	{
		if(firsttime && Fname[0]!='\0') 
		{ 
			// Usar el nombre del archivo de la linea de comandos
			firsttime = 0;  //Flag
		}
		else 
		{
			//Ingresar el nombre del archivo a buscar
			printf("Ingresar el nombre del archivo (o . para salir): ");
			scanf("%s", Fname);
			firsttime = 0;  //Flag
		}

		if(strlen(Fname) == 1 && Fname[0] == '.') 
			exit(1);			// Salir del programa si no se tecleo un nombre de archivo
    
		file = fopen(Fname, "r");
	}  
	while(file == NULL); //Hacer hasta que archivo no sea nuelo
  
	return(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Terminar el char i-esimo (contando a partir de 0), 
 *  empujar los siguientes caracteres presentados por 1
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void KillChar(size_t i, char * Str)
{
	size_t sl = strlen(Str);
  
	for(;i<sl;i++) 
		Str[i] = Str[i+1];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Elimina los caracteres en una cadena, la cual no esta 
 *	imprimiendo caracteres o espacios
 *
 *	Los espacios incluyen ' ', '\f', '\t' etc.
 *
 *	Regresa 1 si no se encuentran caracteres no imprimibles,
 *	de otra forma regresa 0.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean CheckChar(char * Str)
{
	Boolean found = 0;	// encontrar un caractere erroneo
	size_t sl = strlen(Str);
	size_t i=0;
  
	while(i<sl) 
		if (Str[i]<0 || Str[i]>255)
			nrerror("Non-ASCII file\n");
		else if(isprint(Str[i]) || isspace(Str[i])) 
			i++;
		else 
		{
			found = 1;
			KillChar(i, Str);
			sl--;
		}
  
	return(found);	
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresa 1 si esta linea es una linea de comentario en
 *	la cual el primer caractere no espacio es "#".
 *
 *	Además regresa 1 si esta linea es una linea de espacio
 * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean CommentLine(char *Buf)
{
	size_t spn, cspn;
  
	spn = strspn(Buf, " \t");	
	// La longitud abarca los carcateres de espacio o tabulacion

	cspn = strcspn(Buf, "#\n");
	// Longitud despues del 1er # o retorno de carro

	if(spn == cspn) 	// Linea comentada o espacio en blanco
		return(1);
	else				// en caso de que la linea no tenga datos
		return(0);		
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Saltar espacio o linea comentada t regresar una linea de datos solamente
 * * * * * * * * * * * * * * * * * * * * * * * * * */
char * FindDataLine(FILE *File_Ptr)
{
	char buf[STRLEN];
  
	buf[0] = '\0';
	do 
	{	// ltar el espacioo linea de contarios
		if(fgets(buf, 255, File_Ptr) == NULL)  
		{
			printf("Datos incompletos\n");
			buf[0]	='\0';
			break;
		}
		else
		CheckChar(buf);
	} 
	while(CommentLine(buf));
  
  return(buf);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Saltar la version del archivo, luego leer el numero de ejecuciones
 * * * * * * * * * * * * * * * * * * * * * * * * * */
short ReadNumRuns(FILE* File_Ptr)
{
	char buf[STRLEN];
	short n=0;
  
	FindDataLine(File_Ptr); // Saltar la version del archivo

	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') nrerror("Leyendo el número de ejecuciones\n");
	sscanf(buf, "%hd",&n);	        
	
	return(n);
}

  
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer el nombre y formato del archivo.
 *
 *	El formato del archivo puede ser A para ASCII y B para binario 
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadFnameFormat(FILE *File_Ptr, InputStruct *In_Ptr)
{
	char buf[STRLEN];

	/* leer en el nombre y formato del archivo */
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
		nrerror("Leyendo nombre y formato del archivo.\n");
	//Crear la cadena con el nombre y formato del archivo
	sscanf(buf, "%s %c", In_Ptr->out_fname, &(In_Ptr->out_fformat) );
	if(toupper(In_Ptr->out_fformat) != 'B') 
		In_Ptr->out_fformat = 'A';
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer el numero de fotones.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadNumPhotons(FILE *File_Ptr, InputStruct *In_Ptr)
{
	char buf[STRLEN];

	/* leer el numero de fotones. */
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
		nrerror("Leyendo el numero de fotones.\n");
	sscanf(buf, "%ld", &In_Ptr->num_photons);
	if(In_Ptr->num_photons<=0) 
		nrerror("Numero de fotones no positivos.\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los miembros de dz & dr
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadDzDr(FILE *File_Ptr, InputStruct *In_Ptr)
{  
	char buf[STRLEN];

	/* leer en dz, dr. */
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') nrerror("Leyendo dz, dr.\n");
	//Crear la cadena de datos
	sscanf(buf, "%lf%lf", &In_Ptr->dz, &In_Ptr->dr);
	if(In_Ptr->dz<=0) nrerror("dz no-positivo.\n");
	if(In_Ptr->dr<=0) nrerror("dr no-positivo.\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los miembros nz, nr, na.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadNzNrNa(FILE *File_Ptr, InputStruct *In_Ptr)
{
	char buf[STRLEN];

	/** Leer en los numeros de dz, dr, da. **/
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
		nrerror("Leyendo los numeros de dz, dr, da's.\n");
	sscanf(buf, "%hd%hd%hd", &In_Ptr->nz, &In_Ptr->nr, &In_Ptr->na);
	if(In_Ptr->nz<=0) 
		nrerror("Numeros no-positivo de dz's.\n");
	if(In_Ptr->nr<=0) 
		nrerror("Numeros no-positivo de dr's.\n");
	if(In_Ptr->na<=0) 
		nrerror("Numeros no-positivo de da's.\n");
	In_Ptr->da = 0.5*PI/In_Ptr->na;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer numero de capas
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadNumLayers(FILE *File_Ptr, InputStruct *In_Ptr)
{
	char buf[STRLEN];

	/* leer el numero de capas. */
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
		nrerror("Leyendo el numero de capas.\n");
	sscanf(buf, "%hd", &In_Ptr->num_layers);
	if(In_Ptr->num_layers<=0) 
		nrerror("Numero de capas no-positivas.\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer el indice de refraccion n del ambiente
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadAmbient(FILE *File_Ptr, LayerStruct * Layer_Ptr, char *side)
{
	char buf[STRLEN], msg[STRLEN];
	double n;

	//Lee el indice de refraccion deñ ambiente
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
	{
		sprintf(msg, "Leyendo n del %s ambiente.\n", side);
		nrerror(msg);
	}

	sscanf(buf, "%lf", &n );
	if(n<=0) nrerror("n Erroneo.\n");
	Layer_Ptr->n = n;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los parametros de una capa
 *
 *	Regresa 1 si se detecta un error
 *	Regresa 0 en otro caso
 *
 *	*Z_Ptr es la coordenada z de la capa actual, la cual
 *	es usada para convertir la anchura de la capa en coordenadas z
 *	de los dos limites de la capa.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean ReadOneLayer(FILE *File_Ptr, LayerStruct * Layer_Ptr, double *Z_Ptr)
{
	char buf[STRLEN], msg[STRLEN];
	/* d es el ancho, los demas los parametros opticos. */
	double d, n, mua, mus, g;	

	//Lee los valores requeridos del buffer para la capa
	strcpy(buf, FindDataLine(File_Ptr));
	if(buf[0]=='\0') 
		return(1);	/* error si no hay ningun parametro. */

	//Cargar los parametros leidos en sus respectivas variables
	sscanf(buf, "%lf%lf%lf%lf%lf", &n, &mua, &mus, &g, &d);
	if(d<0 || n<=0 || mua<0 || mus<0 || g<0 || g>1) 
		return(1);			/* error de valor en las variables. */
    
	Layer_Ptr->n	= n;
	Layer_Ptr->mua  = mua;	
	Layer_Ptr->mus  = mus;	
	Layer_Ptr->g    = g;
	Layer_Ptr->z0	= *Z_Ptr;
	*Z_Ptr += d;
	Layer_Ptr->z1	= *Z_Ptr;

	return(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los parametros de una de las capas a la vez
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadLayerSpecs(FILE *File_Ptr, short Num_Layers, LayerStruct ** Layerspecs_PP)
{
	char msg[STRLEN];
	short i=0;
	double z = 0.0;	// recordar que esta es la coordenada z para la capa actual
  
	// Asignar un arreglo para los parametros de la capa mediante asigancion dinamica de memoria
	// layer 0 & layer Num_Layers + 1 son para las capas del ambiente o externas
	*Layerspecs_PP = (LayerStruct *)malloc((unsigned) (Num_Layers+2)*sizeof(LayerStruct));
	if (!(*Layerspecs_PP)) 
		nrerror("Asignacion fallida en la funcion ReadLayerSpecs()");
  
	ReadAmbient(File_Ptr, &((*Layerspecs_PP)[i]), "superior"); 
	for(i=1; i<=Num_Layers; i++)  
		if(ReadOneLayer(File_Ptr, &((*Layerspecs_PP)[i]), &z)) 
		{
			sprintf(msg, "Error de lectura %hd de %hd capas\n", i, Num_Layers);
			nrerror(msg);
		}
	ReadAmbient(File_Ptr, &((*Layerspecs_PP)[i]), "inferior"); 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Calcular los angulos criticos para la reflexion interna
 *	total de acuerdo al índice de reflexion relativo de la capa
 *	Todas las capas son procesadas.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void CriticalAngle( short Num_Layers, LayerStruct ** Layerspecs_PP)
{
	short i=0;
	double n1, n2;
	
	//Obtener el angulo critico para cada una de las capas
	for(i=1; i<=Num_Layers; i++)  
	{
		n1 = (*Layerspecs_PP)[i].n;
		n2 = (*Layerspecs_PP)[i-1].n;
		//Se calcula el angulo de acuerdo a si n1 > n2 o no
		(*Layerspecs_PP)[i].cos_crit0 = n1>n2 ? sqrt(1.0 - n2*n2/(n1*n1)) : 0.0;
    
		n2 = (*Layerspecs_PP)[i+1].n;
		(*Layerspecs_PP)[i].cos_crit1 = n1>n2 ? sqrt(1.0 - n2*n2/(n1*n1)) : 0.0;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los parametros de entrada para una ejecucion.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadParm(FILE* File_Ptr, InputStruct * In_Ptr)
{
	char buf[STRLEN];
  
	//Obtener el peso del paquete de fotones
	In_Ptr->Wth = WEIGHT;
	
	//Leer el formato del archivo
	ReadFnameFormat(File_Ptr, In_Ptr);
	//Leer el numero de fotones
	ReadNumPhotons(File_Ptr, In_Ptr);
	//Leer dz & dr
	ReadDzDr(File_Ptr, In_Ptr);
	//Leer nz, nr & na
	ReadNzNrNa(File_Ptr, In_Ptr);
	//Leer el numero de capas
	ReadNumLayers(File_Ptr, In_Ptr);
	
	//Leer las especificaciones de las capas
	ReadLayerSpecs(File_Ptr, In_Ptr->num_layers, &In_Ptr->layerspecs);
	//Calcular el angulo critico
	CriticalAngle(In_Ptr->num_layers, &In_Ptr->layerspecs);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresa 1 si el nombre esta en la lista de nombres
 *	Regresa 0 en otro caso.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean NameInList(char *Name, NameLink List)
{
	while (List != NULL) 
	{
		if(strcmp(Name, List->name) == 0) 
			return(1);
		List = List->next;
	};
	return(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Añadir el nombre a la lista
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void AddNameToList(char *Name, NameLink * List_Ptr)
{
	NameLink list = *List_Ptr;

	if(list == NULL)  //Primer nodo
	{
		*List_Ptr = list = (NameLink)malloc(sizeof(NameNode));
		strcpy(list->name, Name);
		list->next = NULL;
	}
	else //Nodos subsecuentes
	{
		//Mover al ultimo nodo
		while(list->next != NULL)
			list = list->next;

		//Añadir un nodo a la lista
		list->next = (NameLink)malloc(sizeof(NameNode));
		list = list->next;
		strcpy(list->name, Name);
		list->next = NULL;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Verificar por nombres de archivos duplicados.
 *
 *	Una lista enlazada esta configurada para almacenar los
 *	nombres de los archivos utilizados en el archivo de
 *  entrada de datos.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean FnameTaken(char *fname, NameLink * List_Ptr)
{
	//Verificar si el nombre esta en la lista
	if(NameInList(fname, *List_Ptr))
		return(1); //En ese caso la funcion regresa 1
	else
	{
		//En otro caso se añade el nombre a la lista
		AddNameToList(fname, List_Ptr);
		return(0);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Liberar cada nodo en la lista de nombres de archivo.
 * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeFnameList(NameLink List)
{
	NameLink next;
	
	while(List != NULL) 
	{
		next = List->next;
		free(List);
		List = next;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Verificar los parametros de entrada para cada ejecucion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CheckParm(FILE* File_Ptr, InputStruct * In_Ptr)
{
	short i_run;
	short num_runs;	//Numero de ejecuciones independientes
	NameLink head = NULL;	//Se inicia a null 
	//Los archivos de salida comparten el mismo nombre de archivo
	Boolean name_taken;
	char msg[STRLEN];
  
	num_runs = ReadNumRuns(File_Ptr);
	for(i_run=1; i_run<=num_runs; i_run++)  
	{
		printf("Revisando los datos de entrada para cada ejecucion %hd\n", i_run);
		ReadParm(File_Ptr, In_Ptr);  //Leer el nombre del archivo en el apuntador

		//Verificar por nombres de archivos duplicados
		name_taken = FnameTaken(In_Ptr->out_fname, &head);
		if(name_taken) //En caso de hallar uno =. . . 
			sprintf(msg, "Nombre del archivo %s esta duplicado.\n", In_Ptr->out_fname);
		
		//Si no se hallo un nombre duplicado, se libera memoria
		free(In_Ptr->layerspecs);
		//Pero en caso de que algo saliera mal en la ejecucion, se manda un mensaje de error
		if(name_taken) 
			nrerror(msg);
	}
	FreeFnameList(head);
	rewind(File_Ptr);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Asignar los arreglos en OutStruct para una ejecucion
 *	y los elementos del arreglo son automaticamente iniciados a 0.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void InitOutputData(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nz = In_Parm.nz;
	short nr = In_Parm.nr;
	short na = In_Parm.na;
	short nl = In_Parm.num_layers;	
	/* Se debe recordar que hayq eu usar n1+2 por las 2 capas del ambiente */
  
	//Si alguno de los indices de refraccion es menor o igual a cero
	if(nz<=0 || nr<=0 || na<=0 || nl<=0) 
		nrerror("Parametros de la rejilla erroneos.\n");
  
	//Iniciar los numeros puros
	Out_Ptr->Rsp = 0.0;
	Out_Ptr->Rd  = 0.0;
	Out_Ptr->A   = 0.0;
	Out_Ptr->Tt  = 0.0;
  
	//Asignar los arreglos y las matrices
	Out_Ptr->Rd_ra = AllocMatrix(0,nr-1,0,na-1);
	Out_Ptr->Rd_r  = AllocVector(0,nr-1);
	Out_Ptr->Rd_a  = AllocVector(0,na-1);
  
	Out_Ptr->A_rz  = AllocMatrix(0,nr-1,0,nz-1);
	Out_Ptr->A_z   = AllocVector(0,nz-1);
	Out_Ptr->A_l   = AllocVector(0,nl+1);
  
	Out_Ptr->Tt_ra = AllocMatrix(0,nr-1,0,na-1);
	Out_Ptr->Tt_r  = AllocVector(0,nr-1);
	Out_Ptr->Tt_a  = AllocVector(0,na-1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Deshacer lo que InitOutputData hizo.
 *  por ejemplo, liberar las asignaciones de los datos.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeData(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nz = In_Parm.nz;
	short nr = In_Parm.nr;
	short na = In_Parm.na;
	short nl = In_Parm.num_layers;	
	/* Se debe recordar que hayq eu usar n1+2 por las 2 capas del ambiente */
  
	free(In_Parm.layerspecs);
  
	FreeMatrix(Out_Ptr->Rd_ra, 0,nr-1,0,na-1);
	FreeVector(Out_Ptr->Rd_r, 0,nr-1);
	FreeVector(Out_Ptr->Rd_a, 0,na-1);
  
	FreeMatrix(Out_Ptr->A_rz, 0, nr-1, 0,nz-1);
	FreeVector(Out_Ptr->A_z, 0, nz-1);
	FreeVector(Out_Ptr->A_l, 0,nl+1);
  
	FreeMatrix(Out_Ptr->Tt_ra, 0,nr-1,0,na-1);
	FreeVector(Out_Ptr->Tt_r, 0,nr-1);
	FreeVector(Out_Ptr->Tt_a, 0,na-1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener los elementos en el arreglo 1D mediante la suma
 *  de los elementos de la matriz 2D. (sobre ir e ia)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Sum2DRd(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nr = In_Parm.nr;
	short na = In_Parm.na;
	short ir,ia;
	double sum;
  
	for(ir=0; ir<nr; ir++)
	{
		sum = 0.0;
		for(ia=0; ia<na; ia++) 
			sum += Out_Ptr->Rd_ra[ir][ia];
		Out_Ptr->Rd_r[ir] = sum;
	}
  
	for(ia=0; ia<na; ia++)  
	{
		sum = 0.0;
		for(ir=0; ir<nr; ir++) 
			sum += Out_Ptr->Rd_ra[ir][ia];
		Out_Ptr->Rd_a[ia] = sum;
	}
  
	sum = 0.0;
	for(ir=0; ir<nr; ir++) 
		sum += Out_Ptr->Rd_r[ir];
	Out_Ptr->Rd = sum;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresar el indice de la capa de acuerdo al indice
 *	del sistema de rejilla lineal en la direccion z (Iz).
 *
 *	Usar el centro de la caja.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
short IzToLayer(short Iz, InputStruct In_Parm)
{
	short i=1;	//Indice a la capa
	short num_layers = In_Parm.num_layers;
	double dz = In_Parm.dz;
	
	//Recorrer la rejilla de capa a capa
	while( (Iz+0.5)*dz >= In_Parm.layerspecs[i].z1 && i<num_layers) 
		i++;
  
	return(i);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener los elementos en el arreglo 1D mediante la suma
 *  de los elementos de la matriz 2D. (sobre iz)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Sum2DA(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nz = In_Parm.nz;
	short nr = In_Parm.nr;
	short iz,ir;
	double sum;
  
	for(iz=0; iz<nz; iz++)  
	{
		sum = 0.0;
		for(ir=0; ir<nr; ir++) 
			sum += Out_Ptr->A_rz[ir][iz];
		Out_Ptr->A_z[iz] = sum;
	}
  
	sum = 0.0;
	for(iz=0; iz<nz; iz++) 
	{
		sum += Out_Ptr->A_z[iz];
		Out_Ptr->A_l[IzToLayer(iz, In_Parm)] += Out_Ptr->A_z[iz];
	}
	Out_Ptr->A = sum;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener los elementos en el arreglo 1D mediante la suma
 *  de los elementos de la matriz 2D. (sobre ir e ia)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Sum2DTt(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nr = In_Parm.nr;
	short na = In_Parm.na;
	short ir,ia;
	double sum;
  
	for(ir=0; ir<nr; ir++)  
	{
		sum = 0.0;
		for(ia=0; ia<na; ia++) 
			sum += Out_Ptr->Tt_ra[ir][ia];
		Out_Ptr->Tt_r[ir] = sum;
	}
  
	for(ia=0; ia<na; ia++)  
	{
		sum = 0.0;
		for(ir=0; ir<nr; ir++) 
			sum += Out_Ptr->Tt_ra[ir][ia];
		Out_Ptr->Tt_a[ia] = sum;
	}
  
	sum = 0.0;
	for(ir=0; ir<nr; ir++) sum += Out_Ptr->Tt_r[ir];
  Out_Ptr->Tt = sum;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escalar Rd y Tt apropiadamente.
 *
 *	"a" significa el angulo alfa.
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escalar Rd(r,a) y Tt(r,a) por
 *      (area perpendicular a la direccion del foton)
 *		x(angulo solido)x(Num. de fotones).
 *	o
 *		[2*PI*r*dr*cos(a)]x[2*PI*sin(a)*da]x[Num. de fotones]
 *	o
 *		[2*PI*PI*dr*da*r*sin(2a)]x[Num. de fotones]
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escalar Rd(r) & Tt(r) por
 *		(area de la superficie)x(Num. de fotones).
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escalar Rd(a) & Tt(a) por
 *		(angulo solido)x(Num. de fotones).
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScaleRdTt(InputStruct In_Parm, OutStruct *	Out_Ptr)
{
	short nr = In_Parm.nr;
	short na = In_Parm.na;
	double dr = In_Parm.dr;
	double da = In_Parm.da;
	short ir,ia;
	double scale1, scale2;
  
	// El factor (ir+0.5)*sin(2a) a ser añadido.
	scale1 = 4.0*PI*PI*dr*sin(da/2)*dr*In_Parm.num_photons;
	
	for(ir=0; ir<nr; ir++)  
		for(ia=0; ia<na; ia++) 
		{
			scale2 = 1.0/((ir+0.5)*sin(2.0*(ia+0.5)*da)*scale1);
			Out_Ptr->Rd_ra[ir][ia] *= scale2;
			Out_Ptr->Tt_ra[ir][ia] *= scale2;
		}
		
	// el area es 2*PI*[(ir+0.5)*dr]*dr.
	// ir+0.5 a ser añadido.  
	scale1 = 2.0*PI*dr*dr*In_Parm.num_photons;  

	for(ir=0; ir<nr; ir++) 
	{
		scale2 = 1.0/((ir+0.5)*scale1);
		Out_Ptr->Rd_r[ir] *= scale2;
		Out_Ptr->Tt_r[ir] *= scale2;
	}
	
	// el angulo solido es 2*PI*sin(a)*da. A ser añadido el sin(a). */
	scale1  = 2.0*PI*da*In_Parm.num_photons;

	for(ia=0; ia<na; ia++) 
	{
		scale2 = 1.0/(sin((ia+0.5)*da)*scale1);
		Out_Ptr->Rd_a[ia] *= scale2;
		Out_Ptr->Tt_a[ia] *= scale2;
	}
  
	scale2 = 1.0/(double)In_Parm.num_photons;
	Out_Ptr->Rd *= scale2;
	Out_Ptr->Tt *= scale2;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escalar apropiadamente los arreglos de absorcion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScaleA(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	short nz = In_Parm.nz;
	short nr = In_Parm.nr;
	double dz = In_Parm.dz;
	double dr = In_Parm.dr;
	short nl = In_Parm.num_layers;
	short iz,ir;
	short il;
	double scale1;
  
	// Escalar A_rz
	scale1 = 2.0*PI*dr*dr*dz*In_Parm.num_photons;	
	// el volumen es 2*pi*(ir+0.5)*dr*dr*dz.
	// a ser añadido ir+0.5 to be added.
	for(iz=0; iz<nz; iz++) 
		for(ir=0; ir<nr; ir++) 
			Out_Ptr->A_rz[ir][iz] /= (ir+0.5)*scale1;
  
	// Escalar A_z.
	scale1 = 1.0/(dz*In_Parm.num_photons);
	for(iz=0; iz<nz; iz++) 
		Out_Ptr->A_z[iz] *= scale1;
  
	// Escalar A_l. (Hay que evitar int/int).
	scale1 = 1.0/(double)In_Parm.num_photons;	
	for(il=0; il<=nl+1; il++)
		Out_Ptr->A_l[il] *= scale1;
  
	Out_Ptr->A *=scale1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Sumar y escalar los resultados de la ejecucion actual.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SumScaleResult(InputStruct In_Parm, OutStruct * Out_Ptr)
{
	//Obtener los resultados 1D & 0D. */
	Sum2DRd(In_Parm, Out_Ptr);
	Sum2DA(In_Parm,  Out_Ptr);
	Sum2DTt(In_Parm, Out_Ptr);
  
	ScaleRdTt(In_Parm, Out_Ptr);
	ScaleA(In_Parm, Out_Ptr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir el numeor de version como la primera cadena
 *	del archivo.
 *	Usar solo caracteres para que puedan ser leidos tanto
 *	como ASCII o binario.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteVersion(FILE *file, char *Version)
{
  fprintf(file, "%s \t# Numero de version del archivo.\n\n", Version);
  fprintf(file, "####\n# Categorias de datos incluyen: \n");
  fprintf(file, "# InParm, RAT, \n");
  fprintf(file, "# A_l, A_z, Rd_r, Rd_a, Tt_r, Tt_a, \n");
  fprintf(file, "# A_rz, Rd_ra, Tt_ra \n####\n\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir los parametros de entrada al archivo.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteInParm(FILE *file, InputStruct In_Parm)
{
	short i;
  
	fprintf(file, "InParm \t\t\t# Parametros de entrada. Se utilizan cm.\n");
  
	fprintf(file, "%s \tA\t\t# nombre de archivo de salida, ASCII.\n", In_Parm.out_fname);
	fprintf(file, "%ld \t\t\t# Num. de fotones\n", In_Parm.num_photons);
  
	fprintf(file, "%G\t%G\t\t# dz, dr [cm]\n", In_Parm.dz,In_Parm.dr);
	fprintf(file, "%hd\t%hd\t%hd\t# No. of dz, dr, da.\n\n", In_Parm.nz, In_Parm.nr, In_Parm.na);
  
	fprintf(file, "%hd\t\t\t\t\t# Numero de capas\n", In_Parm.num_layers);
	fprintf(file, "#n\tmua\tmus\tg\td\t# Una linea por cada capa\n"); 
	fprintf(file, "%G\t\t\t\t\t# n para el medio arriba\n", In_Parm.layerspecs[0].n); 
	for(i=1; i<=In_Parm.num_layers; i++)  
	{
		LayerStruct s;
		s = In_Parm.layerspecs[i];
		fprintf(file, "%G\t%G\t%G\t%G\t%G\t# capa %hd\n",s.n, s.mua, s.mus, s.g, s.z1-s.z0, i);
	}
	fprintf(file, "%G\t\t\t\t\t# n para el medio debajo\n\n", In_Parm.layerspecs[i].n); 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la reflectancia , absorcion y transmision.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRAT(FILE * file, OutStruct Out_Parm)
{
	//bandera o encabezado
	fprintf(file, "RAT #Reflectancia, absorcion, transmision. \n");

	fprintf(file, "%-14.6G \t#Reflectancia especular [-]\n", Out_Parm.Rsp);
	fprintf(file, "%-14.6G \t#Reflectancia difusa [-]\n", Out_Parm.Rd);
	fprintf(file, "%-14.6G \t#Fraccion absorbida [-]\n", Out_Parm.A);
	fprintf(file, "%-14.6G \t#Transmitancia [-]\n", Out_Parm.Tt);
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la absorcion como funcion de la capa
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_layer(FILE * file, short Num_Layers, OutStruct Out_Parm)
{
	short i;
  
	//bandera o encabezado
	fprintf(file, "A_l #Absorcion como funcion de la capa. [-]\n");

	for(i=1; i<=Num_Layers; i++)
		fprintf(file, "%12.4G\n", Out_Parm.A_l[i]);
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Rd_ra
 *
 *	5 numeros por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_ra(FILE * file, short Nr, short Na, OutStruct Out_Parm)
{
	short ir, ia;
	
	//bandera o encabezado
	fprintf(file, 
	  "%s\n%s\n%s\n%s\n%s\n%s\n",
	  "# Rd[r][angulo]. [1/(cm2sr)].",
	  "# Rd[0][0], [0][1],..[0][na-1]",
	  "# Rd[1][0], [1][1],..[1][na-1]",
	  "# ...",
	  "# Rd[nr-1][0], [nr-1][1],..[nr-1][na-1]",
	  "Rd_ra");
  
	for(ir=0;ir<Nr;ir++)
		for(ia=0;ia<Na;ia++)  
		{
			fprintf(file, "%12.4E ", Out_Parm.Rd_ra[ir][ia]);
			if( (ir*Na + ia + 1)%5 == 0) 
				fprintf(file, "\n");
		}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Rd_r
 *
 *	1 numero por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_r(FILE * file, short Nr, OutStruct Out_Parm)
{
	short ir;
	
	//bandera o encabezado
	fprintf(file, "Rd_r #Rd[0], [1],..Rd[nr-1]. [1/cm2]\n");
  
	for(ir=0;ir<Nr;ir++) 
	{
		fprintf(file, "%12.4E\n", Out_Parm.Rd_r[ir]);
	}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Rd_a
 *
 *	1 numero por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_a(FILE * file, short Na, OutStruct Out_Parm)
{
	short ia;
  
	//bandera o encabezado
	fprintf(file, "Rd_a #Rd[0], [1],..Rd[na-1]. [sr-1]\n");
  
	for(ia=0;ia<Na;ia++) 
	{
		fprintf(file, "%12.4E\n", Out_Parm.Rd_a[ia]);
	}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Tt_ra
 *
 *	5 numeros por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_ra(FILE * file, short Nr, short Na, OutStruct Out_Parm)
{
	short ir, ia;
	
	//bandera o encabezado
	fprintf(file, 
	  "%s\n%s\n%s\n%s\n%s\n%s\n",
	  "# Tt[r][angulo]. [1/(cm2sr)].",
	  "# Tt[0][0], [0][1],..[0][na-1]",
	  "# Tt[1][0], [1][1],..[1][na-1]",
	  "# ...",
	  "# Tt[nr-1][0], [nr-1][1],..[nr-1][na-1]",
	  "Tt_ra");
  
	for(ir=0;ir<Nr;ir++)
		for(ia=0;ia<Na;ia++)  
		{
			fprintf(file, "%12.4E ", Out_Parm.Tt_ra[ir][ia]);
			if( (ir*Na + ia + 1)%5 == 0) 
				fprintf(file, "\n");
		}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Aa_rz
 *
 *	5 numeros por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_rz(FILE * file, short Nr, short Nz, OutStruct Out_Parm)
{
	short iz, ir;

	//bandera o encabezado
	fprintf(file, 
	  "%s\n%s\n%s\n%s\n%s\n%s\n",
	  "# A[r][z]. [1/cm3]",
	  "# A[0][0], [0][1],..[0][nz-1]",
	  "# A[1][0], [1][1],..[1][nz-1]",
	  "# ...",
	  "# A[nr-1][0], [nr-1][1],..[nr-1][nz-1]",
	  "A_rz");
  
	for(ir=0;ir<Nr;ir++)
		for(iz=0;iz<Nz;iz++)  
		{
			fprintf(file, "%12.4E ", Out_Parm.A_rz[ir][iz]);
			if( (ir*Nz + iz + 1)%5 == 0) 
				fprintf(file, "\n");
		}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir A_z
 *
 *	1 numero por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_z(FILE * file, short Nz, OutStruct Out_Parm)
{
	short iz;

	//bandera o encabezado
	fprintf(file, "A_z #A[0], [1],..A[nz-1]. [1/cm]\n");
  
	for(iz=0;iz<Nz;iz++) 
	{
		fprintf(file, "%12.4E\n", Out_Parm.A_z[iz]);
	}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Tt_r
 *
 *	1 numero por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_r(FILE * file, short Nr, OutStruct Out_Parm)
{
	short ir;

	//bandera o encabezado	
	fprintf(file, "Tt_r #Tt[0], [1],..Tt[nr-1]. [1/cm2]\n");
  
	for(ir=0;ir<Nr;ir++) 
	{
		fprintf(file, "%12.4E\n", Out_Parm.Tt_r[ir]);
	}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir Tt_a
 *
 *	1 numero por cada linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_a(FILE * file, short Na, OutStruct Out_Parm)
{
	short ia;
  
	fprintf(file, "Tt_a #Tt[0], [1],..Tt[na-1]. [sr-1]\n");

	//bandera o encabezado
	for(ia=0;ia<Na;ia++) 
	{
		fprintf(file, "%12.4E\n", Out_Parm.Tt_a[ia]);
	}
  
	fprintf(file, "\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escribir todos los resultados en el archivo
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteResult(InputStruct In_Parm, OutStruct Out_Parm, char * TimeReport)
{
	FILE *file;
  
	file = fopen(In_Parm.out_fname, "w");
	if(file == NULL) //Si falla la apaertura del archivo
		nrerror("Imposible abrir el archivo para escritura.\n");
  
	//Escribir la version del archivo (A1->ASCII, B1->Binario)
	if(toupper(In_Parm.out_fformat) == 'A') 
		WriteVersion(file, "A1");
	else 
		WriteVersion(file, "B1");

	//Cuanto tiempo tarda
	fprintf(file, "# %s", TimeReport);
	fprintf(file, "\n");
	
	//Escribir la reflectancia, absorcion y transmitancia
	WriteInParm(file, In_Parm);
	WriteRAT(file, Out_Parm);	
  
	//Arreglos de 1D (A_layer, A_z, Rd_r, Rd_a, Tt_r, Tt_a)
	WriteA_layer(file, In_Parm.num_layers, Out_Parm);
	WriteA_z(file, In_Parm.nz, Out_Parm);
	WriteRd_r(file, In_Parm.nr, Out_Parm);
	WriteRd_a(file, In_Parm.na, Out_Parm);
	WriteTt_r(file, In_Parm.nr, Out_Parm);
	WriteTt_a(file, In_Parm.na, Out_Parm);
  
	//Arreglos de 2D (A_rz, Rd_ra, Tt_ra)
	WriteA_rz(file, In_Parm.nr, In_Parm.nz, Out_Parm);
	WriteRd_ra(file, In_Parm.nr, In_Parm.na, Out_Parm);
	WriteTt_ra(file, In_Parm.nr, In_Parm.na, Out_Parm);
  
	//Cerrar el archivo, finaliza escritura
	fclose(file);
}
