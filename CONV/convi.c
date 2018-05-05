/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Funciones para la lectura de archivos.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

 /* * * * * * * * LIBRERIAS INCLUIDAS * * * * * * * */
#include "conv.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener un nombre de archivo y abrirlo para lectura
 *	intentar de nuevo hasta que el archivo pueda ser abierto
 *  '.' termina el programa
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
FILE *GetFile(char *Fname)
{
	FILE *file = NULL;

	do 
	{
		printf("Ingresar el nombre de archivo de la salida de mcml (o presionar . para quitar): ");
		scanf("%s", Fname);
		if (strlen(Fname) == 1 && Fname[0] == '.')
			break;

		file = fopen(Fname, "r");
	} 
	while (file == NULL);

	return (file);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Obtener un nombre de archivo y abrirlo para lectura
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
FILE *GetFile2(char *Fname, char *archon)
{
	FILE *file = NULL;

	printf("%s", archon);
	do 
	{
/*		printf("Ingresar el nombre de archivo de la salida de mcml (o presionar . para quitar): ");
		scanf("%s", Fname);
		if (strlen(Fname) == 1 && Fname[0] == '.')
			break;*/
		//Copiar la cadena del nombre del archivo a la que debe de abrirse
		strcpy(Fname, archon);
		printf("%s", Fname);
		
		file = fopen(Fname, "r");
	} 
	while (file == NULL);
	
	printf("%s", archon);

	return (file);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Asiganar los arreglos para los datos originales de mcml
 *	calculados de mcml en OutStrcut para cada ejecucion, y
 *	deben de ser automaticamente iniciados a cero.
 *
 *  Regresar 1 si se realizo correctamente & 0 en otro caso.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean AllocOrigData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short nz = In_Ptr->nz;
	short nr = In_Ptr->nr;
	short na = In_Ptr->na;
	short nl = In_Ptr->num_layers;	//Recordar que se requieren +2 para el ambiente

	if (nz <= 0 || nr <= 0 || na <= 0 || nl <= 0) 
	{
		printf("Parametros de rejilla erroneos.\n");
		return (0);
	}
	//Asignar los arreglos y las matrices para 
	//Reflexion
	Out_Ptr->Rd_ra = AllocMatrix(0, nr - 1, 0, na - 1);
	Out_Ptr->Rd_r = AllocVector(0, nr - 1);
	Out_Ptr->Rd_a = AllocVector(0, na - 1);

	//Absorcion
	Out_Ptr->A_rz = AllocMatrix(0, nr - 1, 0, nz - 1);
	Out_Ptr->A_z = AllocVector(0, nz - 1);
	Out_Ptr->A_l = AllocVector(0, nl + 1);

	//Transmision
	Out_Ptr->Tt_ra = AllocMatrix(0, nr - 1, 0, na - 1);
	Out_Ptr->Tt_r = AllocVector(0, nr - 1);
	Out_Ptr->Tt_a = AllocVector(0, na - 1);

	return (1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Asignar los arreglos para la convolucion en OutStruct para
 *	cada ejecucion, y son automaticamente asigandos a ceros.
 *
 *  Regresar 1 si se realizo correctamente, de otra forma es 0.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean AllocConvData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short nz = In_Ptr->nz;
	short nr = In_Ptr->nrc;	//Usar nrc en lugar de nr.
	short na = In_Ptr->na;

	if (nz <= 0 || nr <= 0 || na <= 0) 
	{
		printf("Parametros de rejilla erroneos.\n");
		return (0);
	}
	//Asignar los arreglos y las matrices (Reflexion, Absorcion y Transmision)
	Out_Ptr->Rd_rac = AllocMatrix(0, nr - 1, 0, na - 1);
	Out_Ptr->Rd_rc = AllocVector(0, nr - 1);
	Out_Ptr->A_rzc = AllocMatrix(0, nr - 1, 0, nz - 1);
	Out_Ptr->Tt_rac = AllocMatrix(0, nr - 1, 0, na - 1);
	Out_Ptr->Tt_rc = AllocVector(0, nr - 1);

	return (1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Liberar los arreglos en OutStruct de mcml.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeOrigData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short nz = In_Ptr->nz;
	short nr = In_Ptr->nr;
	short na = In_Ptr->na;
	short nl = In_Ptr->num_layers; //Recordar que se requieren +2 para el ambiente

	if (nz <= 0 || nr <= 0 || na <= 0 || nl <= 0) 
	{
		printf("Parametros de rejilla erroneos.\n");
		return;
	}
	//Libear la memoria de
	//Reflexion
	if (Out_Ptr->Rd_ra != NULL)
		FreeMatrix(Out_Ptr->Rd_ra, 0, nr - 1, 0, na - 1);
	if (Out_Ptr->Rd_r != NULL)
		FreeVector(Out_Ptr->Rd_r, 0, nr - 1);
	if (Out_Ptr->Rd_a != NULL)
		FreeVector(Out_Ptr->Rd_a, 0, na - 1);

	//Absorcion
	if (Out_Ptr->A_rz != NULL)
		FreeMatrix(Out_Ptr->A_rz, 0, nr - 1, 0, nz - 1);
	if (Out_Ptr->A_z != NULL)
		FreeVector(Out_Ptr->A_z, 0, nz - 1);
	if (Out_Ptr->A_l != NULL)
		FreeVector(Out_Ptr->A_l, 0, nl + 1);

	//Transmision
	if (Out_Ptr->Tt_ra != NULL)
		FreeMatrix(Out_Ptr->Tt_ra, 0, nr - 1, 0, na - 1);
	if (Out_Ptr->Tt_r != NULL)
		FreeVector(Out_Ptr->Tt_r, 0, nr - 1);
	if (Out_Ptr->Tt_a != NULL)
		FreeVector(Out_Ptr->Tt_a, 0, na - 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Liberar los arreglos en OutStruct de la convolucion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FreeConvData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short nz = In_Ptr->nz;
	short nr = In_Ptr->nrc;
	short na = In_Ptr->na;

	if (nz <= 0 || nr <= 0 || na <= 0) 
	{
		printf("Parametros de rejilla erroneos.");
		return;
	}
	//Libear la memoria de Reflecion, Absorcion y Transmision
	if (Out_Ptr->Rd_rac != NULL)
		FreeMatrix(Out_Ptr->Rd_rac, 0, nr - 1, 0, na - 1);
	if (Out_Ptr->Rd_rc != NULL)
		FreeVector(Out_Ptr->Rd_rc, 0, nr - 1);
	if (Out_Ptr->A_rzc != NULL)
		FreeMatrix(Out_Ptr->A_rzc, 0, nr - 1, 0, nz - 1);
	if (Out_Ptr->Tt_rac != NULL)
		FreeMatrix(Out_Ptr->Tt_rac, 0, nr - 1, 0, na - 1);
	if (Out_Ptr->Tt_rc != NULL)
		FreeVector(Out_Ptr->Tt_rc, 0, nr - 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Terminar el char i-esimo (contando a partir de 0), 
 *  empujar los siguientes caracteres presentados por 1
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void KillChar(size_t i, char *Str)
{
	size_t sl = strlen(Str);

	for (; i < sl; i++)
		Str[i] = Str[i + 1];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Elimina los caracteres en una cadena, la cual no esta 
 *	imprimiendo caracteres o espacios
 *
 *	Los espacios incluyen ' ', '\f', '\t' etc.
 *
 *	Regresa 1 si no se encuentran caracteres no imprimibles,Return 1 if no nonprinting chars found, otherwise 
 *	de otra forma regresa 0.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean CheckChar(char *Str)
{
	Boolean found = 0; // encontrar un caracter erroneo
	size_t sl = strlen(Str);
	size_t i = 0;

	while (i < sl)
		if (isprint(Str[i]) || isspace(Str[i]))
			i++;
		else 
		{
			found = 1;
			KillChar(i, Str);
			sl--;
		}

	return (found);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresa 1 si esta linea es una linea de comentario en
 *	la cual el primer caractere no espacio es "#".
 *
 *	Además regresa 1 si esta linea es una linea de espacio
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean CommentLine(char *Buf)
{
	size_t spn, cspn;

	// La longitud abarca los caracteres de espacio o tabulacion
	spn = strspn(Buf, " \t");
	// Longitud despues del 1er # o retorno de carro
	cspn = strcspn(Buf, "#\n");

	if (spn == cspn) // Linea comentada o espacio en blanco
		return (1);
	else			// en caso de que la linea no tenga datos
		return (0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Saltar espacio o linea comentada t regresar una linea de datos solamente
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
char *FindDataLine(FILE * File_Ptr)
{
	char buf[STRLEN];

	do 
	{ // Saltar el espacio linea de contarios
		if (fgets(buf, STRLEN, File_Ptr) == NULL) 
		{
			printf("Datos incompletos.\n");
			exit(1);
		}
		CheckChar(buf);
	} 
	while (CommentLine(buf));

	return (buf);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Buscar por la palabra clave, la cual es la primera palabra en la linea.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean FoundKeyWord(char *LineBuf, char *Key)
{
	char *sub_str;
	Boolean found = 0;

	if ((sub_str = strstr(LineBuf, Key)) != NULL)
		if (sub_str == LineBuf)
			found = 1;

	return (found);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Funcion para buscar la palabra clave
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SeekKey(FILE * File_Ptr, char *Key)
{
	char buf[STRLEN];

	do
		strcpy(buf, FindDataLine(File_Ptr));
	while (!FoundKeyWord(buf, Key));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer los parametros de la capa
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadLayerParm(FILE * File_Ptr, short Num_Layers, LayerStruct ** Layers_PP)
{
	char buf[STRLEN];
	short i = 0;
	double d, n, mua, mus, g;
	double z = 0.0; //Coordenada z para la capa actual

	//La capa 0 y la capa Num_Layers + 1 son para el ambiente
	*Layers_PP = (LayerStruct *)malloc((unsigned) (Num_Layers + 2) * sizeof(LayerStruct));
	if (!(*Layers_PP))
		nrerror("Fallo de asignacion en ReadLayerParm()");

	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &n);
	(*Layers_PP)[i].n = n;
	for (i = 1; i <= Num_Layers; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf%lf%lf%lf%lf", &n, &mua, &mus, &g, &d);
		(*Layers_PP)[i].n = n;
		(*Layers_PP)[i].mua = mua;
		(*Layers_PP)[i].mus = mus;
		(*Layers_PP)[i].g = g;
		(*Layers_PP)[i].z0 = z;
		z += d;
		(*Layers_PP)[i].z1 = z;
	}
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &n);
	(*Layers_PP)[i].n = n;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer en los parametros de entrada los cuales fueron
 *	utilizados en las simulaciones Monte Carlo.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadInParm(FILE * File_Ptr, InputStruct * In_Ptr)
{
	char buf[STRLEN];

	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%c", &(In_Ptr->in_fformat));
	if (toupper(In_Ptr->in_fformat) != 'B')
		In_Ptr->in_fformat = 'A';

	//Encontrar la palabra clave "InParm".
	do
		strcpy(buf, FindDataLine(File_Ptr));
	while (!FoundKeyWord(buf, "InParm"));

	//Nombre de archivo de escapa & formato de archivo
	FindDataLine(File_Ptr);

	//Leer en el numero de fotones
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%ld", &In_Ptr->num_photons);

	//Asignar en Wth (peso del foton crítico, para la ruleta)
	In_Ptr->Wth = 1E-4;

	//Leer en dz, dr.
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf%lf", &In_Ptr->dz, &In_Ptr->dr);

	//Leer en nz, nr, na & calcular da.
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%hd%hd%hd", &In_Ptr->nz, &In_Ptr->nr, &In_Ptr->na);
	In_Ptr->da = 0.5 * PI / In_Ptr->na;

	//Leer en el numero de capas
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%hd", &In_Ptr->num_layers);

	ReadLayerParm(File_Ptr, In_Ptr->num_layers, &In_Ptr->layerspecs);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la reflectancia, fraccion absorbida y transmitancia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadRAT(FILE * File_Ptr, OutStruct * Out_Ptr)
{
	char buf[STRLEN];

	//Reflexion especular
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &(Out_Ptr->Rsp));

	//Reflexion difusa
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &(Out_Ptr->Rd));

	//Absorcion
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &(Out_Ptr->A));

	//Transmision
	strcpy(buf, FindDataLine(File_Ptr));
	sscanf(buf, "%lf", &(Out_Ptr->Tt));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la absorcion de la capa 
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadA_layer(FILE * File_Ptr, short Num_Layers, OutStruct * Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 1; i <= Num_Layers; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf", &(Out_Ptr->A_l[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la absorcion en el eje z
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadA_z(FILE * File_Ptr, short Nz, OutStruct * Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 0; i < Nz; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf", &(Out_Ptr->A_z[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la reflectancia difusa en r.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadRd_r(FILE * File_Ptr, short Nr, OutStruct * Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 0; i < Nr; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf", &(Out_Ptr->Rd_r[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la reflectancia difusa en a.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadRd_a(FILE *File_Ptr, short Na, OutStruct *Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 0; i < Na; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf", &(Out_Ptr->Rd_a[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la Transmitancia en r.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadTt_r(FILE * File_Ptr, short Nr, OutStruct * Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 0; i < Nr; i++) 
	{
		strcpy(buf, FindDataLine(File_Ptr));
		sscanf(buf, "%lf", &(Out_Ptr->Tt_r[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la Transmitancia en a.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadTt_a(FILE * File_Ptr, short Na, OutStruct * Out_Ptr)
{
	char buf[STRLEN];
	short i;

	for (i = 0; i < Na; i++) 
	{
		//strcpy(buf, FindDataLine(File_Ptr)); 
		//sscanf(buf, "%lf",&(Out_Ptr->Tt_a[i]));
		fscanf(File_Ptr, "%lf", &(Out_Ptr->Tt_a[i]));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la abosrcion en r & z.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadA_rz(FILE * File_Ptr, short Nr, short Nz, OutStruct * Out_Ptr)
{
	short iz, ir;

	for (ir = 0; ir < Nr; ir++)
		for (iz = 0; iz < Nz; iz++)
			fscanf(File_Ptr, "%lf ", &(Out_Ptr->A_rz[ir][iz]));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la absorcion en a.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadRd_ra(FILE * File_Ptr, short Nr, short Na, OutStruct * Out_Ptr)
{
	short ir, ia;

	for (ir = 0; ir < Nr; ir++)
		for (ia = 0; ia < Na; ia++)
			fscanf(File_Ptr, "%lf ", &(Out_Ptr->Rd_ra[ir][ia]));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer la transmision en r & a.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadTt_ra(FILE * File_Ptr, short Nr, short Na, OutStruct * Out_Ptr)
{
	short ir, ia;

	for (ir = 0; ir < Nr; ir++)
		for (ia = 0; ia < Na; ia++)
			fscanf(File_Ptr, "%lf ", &(Out_Ptr->Tt_ra[ir][ia]));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Leer en los parametros de salida de MonteCarlo
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadOutMC(FILE * File_Ptr, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	//Iniciar al parametro definido {0, 0, 0, 0, 0}
	ConvStruct  null_conved = NULLCONVSTRUCT;

	SeekKey(File_Ptr, "RAT");
	ReadRAT(File_Ptr, Out_Ptr);	//Lectura de reflexion, absorcion y transmision

	//Arreglos 1D
	//Absorcion
	SeekKey(File_Ptr, "A_l");
	ReadA_layer(File_Ptr, In_Ptr->num_layers, Out_Ptr);

	//Absorcion en z
	SeekKey(File_Ptr, "A_z");
	ReadA_z(File_Ptr, In_Ptr->nz, Out_Ptr);

	//Reflexion difusa en r
	SeekKey(File_Ptr, "Rd_r");
	ReadRd_r(File_Ptr, In_Ptr->nr, Out_Ptr);

	//Reflexion difusa en a
	SeekKey(File_Ptr, "Rd_a");
	ReadRd_a(File_Ptr, In_Ptr->na, Out_Ptr);

	//Transmision en r
	SeekKey(File_Ptr, "Tt_r");
	ReadTt_r(File_Ptr, In_Ptr->nr, Out_Ptr);

	//Transmision en a
	SeekKey(File_Ptr, "Tt_a");
	ReadTt_a(File_Ptr, In_Ptr->na, Out_Ptr);

	//Arreglos 2D
	//Absorcion en r & z
	SeekKey(File_Ptr, "A_rz");
	ReadA_rz(File_Ptr, In_Ptr->nr, In_Ptr->nz, Out_Ptr);

	//Reflexion en r & a
	SeekKey(File_Ptr, "Rd_ra");
	ReadRd_ra(File_Ptr, In_Ptr->nr, In_Ptr->na, Out_Ptr);

	//Transmision en r & a
	SeekKey(File_Ptr, "Tt_ra");
	ReadTt_ra(File_Ptr, In_Ptr->nr, In_Ptr->na, Out_Ptr);

	Out_Ptr->conved = null_conved;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Despues de que los parametros de entrada son leidos
 *	los parametros drc & nrc son iniciados a dr & nr respectivamente.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadMcoFile(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
   FILE *infile;

   //Obtener el archivo de una cadena de texto
   infile = GetFile(In_Ptr->in_fname);
   //En caso de que no se encuentre el archivo sale de la funcion
	if (infile == NULL)
		return;

	if (Out_Ptr->allocated) 
	{
		OutStruct null_out = NULLOUTSTRUCT;
		FreeOrigData(In_Ptr, Out_Ptr);
		*Out_Ptr = null_out;
	}
	//Leer parametros de entrada
	ReadInParm(infile, In_Ptr);
	In_Ptr->beam.type = pencil;
	In_Ptr->drc = In_Ptr->dr;
	In_Ptr->nrc = In_Ptr->nr;

	//Asignar memoria
	AllocOrigData(In_Ptr, Out_Ptr);
	AllocConvData(In_Ptr, Out_Ptr);
	Out_Ptr->allocated = 1;

	ReadOutMC(infile, In_Ptr, Out_Ptr);
	//Cerrar el archivo
	fclose(infile);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Para leer de forma automática los archivos
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ReadMcoFile2(InputStruct * In_Ptr, OutStruct * Out_Ptr, char *archon)
{
   FILE *infile;

   puts("3 ");
   //Obtener el archivo de una cadena de texto
   infile = GetFile2(In_Ptr->in_fname, archon);
   //En caso de que no se encuentre el archivo sale de la funcion
	if (infile == NULL)
		return;

	if (Out_Ptr->allocated) 
	{
		OutStruct null_out = NULLOUTSTRUCT;
		FreeOrigData(In_Ptr, Out_Ptr);
		*Out_Ptr = null_out;
	}
	//Leer parametros de entrada
	ReadInParm(infile, In_Ptr);
	In_Ptr->beam.type = pencil;
	In_Ptr->drc = In_Ptr->dr;
	In_Ptr->nrc = In_Ptr->nr;

	//Asignar memoria
	AllocOrigData(In_Ptr, Out_Ptr);
	AllocConvData(In_Ptr, Out_Ptr);
	Out_Ptr->allocated = 1;

	ReadOutMC(infile, In_Ptr, Out_Ptr);
	//Cerrar el archivo
	fclose(infile);
}
