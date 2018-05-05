/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Funciones para la escritura de archivos.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
/* Librerias a incluir */
#include "conv.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Centrar una cadena de acuerdo al ancho de la columna
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
char *CenterStr(short int Wid, char *InStr, char *OutStr)
{
	size_t nspaces; //Numero de espacios a ser rellenados antes de InStr

	nspaces = (Wid - strlen(InStr)) / 2;
	if (nspaces < 0)
		nspaces = 0;

	strcpy(OutStr, "");
	while (nspaces--)
		strcat(OutStr, " ");

	strcat(OutStr, InStr);
	
	return (OutStr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Imprimir algunos mensajes antes de que comience la simulacion
 *	e.g. autor, direccion, version del programa, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define COLWIDTH 80
void ShowVersion(void)
{
	char str[STRLEN];

	CenterStr(COLWIDTH, "Convolution of mcml Simulation Data", str);
	puts("");
	puts(str);
	puts("");

	CenterStr(COLWIDTH, "Lihong Wang, Ph.D.", str);
	puts(str);

	CenterStr(COLWIDTH, "Steven L. Jacques, Ph.D.", str);
	puts(str);

	CenterStr(COLWIDTH, "Laser Biology Research Laboratory - 017", str);
	puts(str);

	CenterStr(COLWIDTH, "University of Texas M.D. Anderson Cancer Center", str);
	puts(str);

	CenterStr(COLWIDTH, "Houston, Texas 77030, USA", str);
	puts(str);

	CenterStr(COLWIDTH, "Voice: (713) 792-3664, Fax: (713) 792-3995", str);
	puts(str);

	puts("");

	CenterStr(COLWIDTH, "Version 1.1, 1994", str);
	puts(str);
	puts("\n\n");
}
#undef COLWIDTH

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Abrir el archivo para la salida con la extension Ext. Si 
 *	el archivo existe preguntar si se sobrescribe, añade o cambiar el nombre del archivo
 *
 *	Regresar el apuntador al archivo, el cual puede ser NULL.
 *	Regresar el nombre completo con la extension.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
FILE *GetWriteFile(char *Ext)
{
	FILE *file;
	char fname[STRLEN], fmode[STRLEN];

	do 
	{
		printf("Ingresar el nombre del archivo de salida con extensión .%s (o . para salir): ", Ext);
		gets(fname); //Obtener el nombre del archivo
		//Si se escribe '.' o solo 1 caracter, entonces salir
		if (strlen(fname) == 1 && fname[0] == '.')
		{
			fmode[0] = 'q';
			printf("Error 1");
			break;
		} 
		else	//En otro caso se escribe
			fmode[0] = 'w';

		if ((file = fopen(fname, "r")) != NULL) //El archivo existe
		{
			printf("El archivo %s ya existe, %s", fname, "w=sobrescribir, a=añadir al existente, n=nuevo archivo, q=salir: ");
			do
				gets(fmode);
			while (!strlen(fmode));	//Evitar la linea nula
			fclose(file);
		}
	} 
	while (fmode[0] != 'w' && fmode[0] != 'a' && fmode[0] != 'q'); //ejecutar hasta que se presione una opcion valida

	//Aqui se verifica que el archivo se abre
	if (fmode[0] != 'q')
	{
		file = fopen(fname, fmode);
		printf("\nArchivo %s abierto con exito!", fname);
	}
	// o existio un error en la apertura
	else
	{
		file = NULL;
		printf("\nFallo apertura de archivo %s (NULL)",fname);
	}

	strcpy(Ext, fname);
	return (file); //El nombre del archivo puede ser NULL
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Abrir el archivo para la salida con la extension Ext. y el nombre archon
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
FILE *GetWriteFile2(char *Ext, char *archon)
{
	FILE *file;
	char fname[STRLEN], fmode[STRLEN];
	char *rfv;

	do 
	{
		//printf("Ingresar el nombre del archivo de salida con extensión .%s (o . para salir): ", Ext);
		//gets(fname); //Obtener el nombre del archivo
		rfv=strtok(archon,".");
		sprintf(fname,"%s.%s",rfv,Ext);
		//Si se escribe '.' o solo 1 caracter, entonces salir
		if (strlen(fname) == 1 && fname[0] == '.')
		{
			fmode[0] = 'q';
			printf("Error 1");
			break;
		} 
		else	//En otro caso se escribe
			fmode[0] = 'w';

		if ((file = fopen(fname, "r")) != NULL) //El archivo existe
		{
			printf("El archivo %s ya existe, %s", fname, "w=sobrescribir, a=añadir al existente, n=nuevo archivo, q=salir: ");
			do
				gets(fmode);
			while (!strlen(fmode));	//Evitar la linea nula
			fclose(file);
		}
	} 
	while (fmode[0] != 'w' && fmode[0] != 'a' && fmode[0] != 'q'); //ejecutar hasta que se presione una opcion valida

	//Aqui se verifica que el archivo se abre
	if (fmode[0] != 'q')
	{
		file = fopen(fname, fmode);
		printf("\nArchivo %s abierto con exito!", fname);
	}
	// o existio un error en la apertura
	else
	{
		file = NULL;
		printf("\nFallo apertura de archivo %s (NULL)",fname);
	}

	strcpy(Ext, fname);
	return (file); //El nombre del archivo puede ser NULL
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Regresar el indice de la capa de acuerdo al indice
 *	del sistema de rejilla lineal en la direccion z (Iz).
 *
 *	Usar el centro de la caja.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
short IzToLayer(short Iz, InputStruct * In_Ptr)
{
	short i = 1;	//Indice a la capa
	short num_layers = In_Ptr->num_layers;
	double dz = In_Ptr->dz;

	while ((Iz + 0.5) * dz >= In_Ptr->layerspecs[i].z1 && i < num_layers)
		i++;

	return (i);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir los parametros de entrada para el programa de 
 *  simulacion Monte Carlo en un formato tal que pueda ser leido
 *  directamente sin dificultad
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteInParm(InputStruct * In_Ptr)
{
	short i;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "InP");
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

	fprintf(file, "1.1\t# Version del archivo.\n");
	fprintf(file, "1\t# Numero de ejecuciones.\n\n");
	fprintf(file, "temp.out\tA\t\t#nombre de archivo de salida.\n");

	fprintf(file, "%ld \t\t\t# Num. de fotones.\n", In_Ptr->num_photons);

	fprintf(file, "%G\t%G\t\t# dz, dr.\n", In_Ptr->dz, In_Ptr->dr);
	fprintf(file, "%hd\t%hd\t%hd\t# Num. de dz, dr, da.\n\n", In_Ptr->nz, In_Ptr->nr, In_Ptr->na);

	fprintf(file, "%hd\t\t\t\t\t# Num. de capas.\n", In_Ptr->num_layers);
	fprintf(file, "#n\tmua\tmus\tg\td\t# Una ñinea para cada capa.\n");
	fprintf(file, "%G\t\t\t\t\t# n para el medio por encima.\n", In_Ptr->layerspecs[0].n);
	for (i = 1; i <= In_Ptr->num_layers; i++) 
	{
		LayerStruct s;
		s = In_Ptr->layerspecs[i];
		fprintf(file, "%G\t%G\t%G\t%G\t%G\t# capa %hd\n", s.n, s.mua, s.mus, s.g, s.z1 - s.z0, i);
	}
	fprintf(file, "%G\t\t\t\t\t# n para el medio por debajo.\n\n", In_Ptr->layerspecs[i].n);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la reflectancia , absorcion y transmision.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRAT(OutStruct * Out_Ptr)
{
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "RAT");
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

	fprintf(file,"RAT #Reflectancia, absorcion, transmision.\n");
	fprintf(file, "%-12.4G \t#Reflectancia especular.\n", Out_Ptr->Rsp);
	fprintf(file, "%-12.4G \t#Reflectance difusa.\n", Out_Ptr->Rd);
	fprintf(file, "%-12.4G \t#Absorcion.\n", Out_Ptr->A);
	fprintf(file, "%-12.4G \t#Transmision.\n", Out_Ptr->Tt);
	fprintf(file, "\n");

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la absorcion como funcion de la capa
 *  2 numeros para cada linea: layer, A[layer]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_layer(short Num_Layers, double *A_l)
{
	short i;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Al");
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

	fprintf(file, "capa\t%-s[-]\n", fname);
	for (i = 1; i <= Num_Layers; i++)
		fprintf(file, "%-4hd\t%-12.4G\n", i, A_l[i]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la absorcion en z
 *  2 numeros para cada linea: z, A[z]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_z(InputStruct * In_Ptr, double *A_z)
{
	short nz = In_Ptr->nz;
	double dz = In_Ptr->dz;
	short iz;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Az");
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

	fprintf(file, "%-12s\t%-s[1/cm]\n", "z[cm]", fname);
	for (iz = 0; iz < nz; iz++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (iz + 0.5) * dz, A_z[iz]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la absorcion en r & z
 *  3 numeros para cada linea: r, z, A[r][z]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteA_rz(InputStruct * In_Ptr, double **A_rz)
{
	short ir, iz, nz = In_Ptr->nz, nr = In_Ptr->nr;
	double r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Arz");
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

	fprintf(file, "%-12s\t%-12s\t%-s[1/cm3]\n", "r[cm]", "z[cm]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (iz = 0; iz < nz; iz++) 
		{
			z = (iz + 0.5) * dz;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, A_rz[ir][iz]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la fluencia en z
 *  2 numeros para cada linea: z, F[z]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteF_z(InputStruct * In_Ptr, double *A_z)
{
	FILE *file;
	short iz, nz = In_Ptr->nz;
	double mua, dz = In_Ptr->dz;
	char fname[STRLEN];

	strcpy(fname, "Fz");
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

	fprintf(file, "%-12s\t%-s[-]\n", "z[cm]", fname);
	for (iz = 0; iz < nz; iz++) 
	{
		mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
		if (mua > 0.0)
			fprintf(file, "%-12.4E\t%-12.4E\n", (iz + 0.5) * dz, A_z[iz] / mua);
		else
			fprintf(file, "%-12.4E\t%-12.4E\n", (iz + 0.5) * dz, 0.0);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la fluencia en r & z
 *  3 numeros para cada linea: r, z, F[r][z]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteF_rz(InputStruct * In_Ptr, double **A_rz)
{
	FILE *file;
	short ir, iz, nz = In_Ptr->nz, nr = In_Ptr->nr;
	double mua, r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
	char fname[STRLEN];

	strcpy(fname, "Frz");
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

	fprintf(file, "%-12s\t%-12s\t%-s[1/cm2]\n", "r[cm]", "z[cm]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (iz = 0; iz < nz; iz++) 
		{
			z = (iz + 0.5) * dz;
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, A_rz[ir][iz] / mua);
			else
				fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, z, 0.0);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la reflectancia difusa en r & a
 *  3 numeros para cada linea: r, a, Rd[r][a]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_ra(InputStruct * In_Ptr, double **Rd_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Rra");
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

	fprintf(file, "%-12s\t%-12s\t%-s[1/(cm2sr)]\n", "r[cm]", "a[rad]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (ia = 0; ia < na; ia++) 
		{
			a = (ia + 0.5) * da;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, a, Rd_ra[ir][ia]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la reflectancia difusa en r
 *  2 numeros para cada linea: r, Rd[r]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_r(InputStruct * In_Ptr, double *Rd_r)
{
	short ir, nr = In_Ptr->nr;
	double dr = In_Ptr->dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Rr");
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

	fprintf(file, "%-12s\t%-s[1/cm2]\n", "r[cm]", fname);
	for (ir = 0; ir < nr; ir++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ir + 0.5) * dr, Rd_r[ir]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la reflectancia difusa en a
 *  3 numeros para cada linea: a, Rd[a]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteRd_a(InputStruct * In_Ptr, double *Rd_a)
{
	short ia, na = In_Ptr->na;
	double da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Ra");
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

	fprintf(file, "%-12s\t%-s[1/sr]\n", "a[rad]", fname);
	for (ia = 0; ia < na; ia++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ia + 0.5) * da, Rd_a[ia]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la transmision en r & a (a=theta)
 *  3 numeros para cada linea: r, a, Tt[r][a]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_ra(InputStruct * In_Ptr, double **Tt_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Tra");
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

	fprintf(file, "%-12s\t%-12s\t%-s[1/(cm2sr)]\n", "r[cm]", "a[rad]", fname);
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		for (ia = 0; ia < na; ia++) 
		{
			a = (ia + 0.5) * da;
			fprintf(file, "%-12.4E\t%-12.4E\t%-12.4E\n", r, a, Tt_ra[ir][ia]);
		}
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la transmitancia en r
 *  2 numeros para cada linea: r, Tt[r]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_r(InputStruct * In_Ptr, double *Tt_r)
{
	short ir, nr = In_Ptr->nr;
	double dr = In_Ptr->dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Tr");
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

	fprintf(file, "%-12s\t%-s[1/cm2]\n", "r[cm]", fname);
	for (ir = 0; ir < nr; ir++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ir + 0.5) * dr, Tt_r[ir]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir la transmitancia en theta
 *  2 numeros para cada linea: theta, Tt[theta]
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteTt_a(InputStruct * In_Ptr, double *Tt_a)
{
	short ia, na = In_Ptr->na;
	double da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Ta");
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

	fprintf(file, "%-12s\t%-s[1/sr]\n", "a[rad]", fname);
	for (ia = 0; ia < na; ia++)
		fprintf(file, "%-12.4E\t%-12.4E\n", (ia + 0.5) * da, Tt_a[ia]);

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escribir el archivo de salida en el formato de M. Keijzer's
 *	para que el archivo pueda ser leido por el programa de
 *  convolucion escrito por Keijzer en Pascal
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void WriteKFormat(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	short i, j;
	double dz, dr;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "K");
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

	fputs("archivo.salida\n", file);
	fprintf(file, "%12hd capas\n", In_Ptr->num_layers);
	fprintf(file, "%12s %12s %12s %12s %12s %12s\n", "capa", "mua", "mus", "g", "nt", "espesor");

	for (i = 1; i <= In_Ptr->num_layers; i++)
		fprintf(file,
	    "%12hd %12.6lf %12.6lf %12.6lf %12.6lf %12.6lf\n",
	    i, In_Ptr->layerspecs[i].mua,
	    In_Ptr->layerspecs[i].mus,
	    In_Ptr->layerspecs[i].g, In_Ptr->layerspecs[i].n,
	    In_Ptr->layerspecs[i].z1 -
	    In_Ptr->layerspecs[i].z0);
	fprintf(file, "%12.6lf indice de la refraccion superior\n", In_Ptr->layerspecs[0].n);
	fprintf(file, "%12.6lf indice de la refraccion inferior\n", In_Ptr->layerspecs[i].n);
	fprintf(file, "\n");

	fprintf(file, "%12ld fotones\n", In_Ptr->num_photons);
	fprintf(file, "%12.6lf peso critico\n", In_Ptr->Wth);
	fprintf(file, "%12.6lf profundidad de las cajas en micrones\n", In_Ptr->dz * 1e4);
	fprintf(file, "%12.6lf ancho de las cajas en micrones\n", In_Ptr->dr * 1e4);
	fprintf(file, "%12hd numero de cajas en z \n", In_Ptr->nz);
	fprintf(file, "%12hd numero de cajas en r \n", In_Ptr->nr);
	fprintf(file, "\n");

	fprintf(file,"%12.6lf Reflexion total (incluyendo la R directa)\n",
	Out_Ptr->Rsp + Out_Ptr->Rd);
	for (i = 1; i <= In_Ptr->num_layers; i++)
		fprintf(file, "%12.6lf Absorcion en la capa %12hd\n", Out_Ptr->A_l[i], i);
	fprintf(file, "%12.6lf Transmision total\n", Out_Ptr->Tt);
	fprintf(file, "\n");

	fprintf(file, "Reflectancia & Transmision en [cm-2]\n");
	fprintf(file, "Absorcion en capas z en [cm-1]\n");
	fprintf(file, "Absorcion en cajas z/r en [cm-3]\n");

	fprintf(file, "z/r [cm] Capa");
	dr = In_Ptr->dr;
	for (i = 0; i < In_Ptr->nr; i++)
		fprintf(file, "%12.6lf ", i * dr);
	fprintf(file, "\n");

	fprintf(file, "Refl. ");
	for (i = 0; i < In_Ptr->nr; i++)
		fprintf(file, "%12.6lf ", Out_Ptr->Rd_r[i]);
	fprintf(file, "\n");

	dz = In_Ptr->dz;
	for (i = 0; i < In_Ptr->nz; i++) 
	{
		fprintf(file, "%12.6lf %12.6lf", i * dz, Out_Ptr->A_z[i]);
		for (j = 0; j < In_Ptr->nr; j++)
			fprintf(file, "%12.6lf ", Out_Ptr->A_rz[j][i]);
		fprintf(file, "\n");
	}

	fprintf(file, "Transm.");
	for (i = 0; i < In_Ptr->nr; i++)
		fprintf(file, "%12.6lf ", Out_Ptr->Tt_r[i]);
	fprintf(file, "\n");

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Mostrar el menu de salida como ayuda
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowOutMenu(char *in_fname)
{
	printf("I   = Parametros de entrada de mcml\n");
	printf("3   = reflectancia, absorcion, & transmitancia\n");

	printf("AL  = absorcion vs capa [-]\n");
	printf("Az  = absorcion vs z [1/cm]\n");
	printf("Arz = absorcion vs r & z [1/cm3]\n");
	printf("Fz  = fluencia vs z [-]\n");
	printf("Frz = fluencia vs r & z [1/cm2]\n");

	printf("Rr  = Reflectancia difusa vs radio r [1/cm2]\n");
	printf("Ra  = Reflectancia difusa vs angulo alpha [1/sr]\n");
	printf("Rra = Reflectancia difusa vs radio & angulo a [1/(cm2 sr)]\n");

	printf("Tr  = transmitancia vs radio r [1/cm2]\n");
	printf("Ta  = transmitancia vs angulo alpha [1/sr]\n");
	printf("Tra = transmitancia vs radio & angulo [1/(cm2 sr)]\n");

	printf("K   = Formato Keijzer\n");
	printf("Q   = Regresar al menu principal\n");
	printf("* nombre de archivo de entrada: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para la parte de absorcion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutA(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'L': //Absorcion vs capa {AL}
			WriteA_layer(In_Ptr->num_layers, Out_Ptr->A_l);
			break;
		case 'Z': //Absorcion vs z {Az}
			if (toupper(Cmd_Str[2]) == '\0')
				WriteA_z(In_Ptr, Out_Ptr->A_z);
			else
				puts("...Comando erroneo");
			break;
		case 'R': //Absorcion vs r & z {Arz}
			if (toupper(Cmd_Str[2]) == 'Z')
				WriteA_rz(In_Ptr, Out_Ptr->A_rz);
			else
				puts("...Comando erroneo");
			break;
		default: //Comando erroneo por completo
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para la parte de fluencia
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutF(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'Z': //Fluencia vs z {Fz}
			if (toupper(Cmd_Str[2]) == '\0')
				WriteF_z(In_Ptr, Out_Ptr->A_z);
			else
				puts("...Comando erroneo");
			break;
		case 'R': //Fluencia vs r & z {Frz}
			if (toupper(Cmd_Str[2]) == 'Z')
				WriteF_rz(In_Ptr, Out_Ptr->A_rz);
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para la parte de reflectancia difusa
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutR(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[1])) 
	{
		case 'A': //Reflectancia difusa vs angulo alpha {Rd_a}
			WriteRd_a(In_Ptr, Out_Ptr->Rd_a);
			break;
		case 'R': //Existen dos posibilidades
			ch = toupper(Cmd_Str[2]);
			if (ch == '\0') //Reflectancia difusa vs radio {Rd_r}
				WriteRd_r(In_Ptr, Out_Ptr->Rd_r);
			else if (ch == 'A') //Reflectancia difusa vs radio & angulo a {Rd_ra}
				WriteRd_ra(In_Ptr, Out_Ptr->Rd_ra);
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para la parte de transmitancia
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutT(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[1])) 
	{
		case 'A': //transmitancia vs angulo alpha {Tt_a}
			WriteTt_a(In_Ptr, Out_Ptr->Tt_a);
			break;
		case 'R':
			ch = toupper(Cmd_Str[2]);
			if (ch == '\0') //transmitancia vs radio r {Tt_r}
				WriteTt_r(In_Ptr, Out_Ptr->Tt_r);
			else if (ch == 'A')	//transmitancia vs radio & angulo {Tt_ra}
				WriteTt_ra(In_Ptr, Out_Ptr->Tt_ra);
			else
				puts("...Comando erroneo");
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Para movilizarse dentro del menu de salida
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchOutCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'I':	//Parametros de entrada de mcml
			WriteInParm(In_Ptr);
			break;
		case '3':	//reflectancia, absorcion, & transmitancia
			WriteRAT(Out_Ptr);
			break;
		case 'K':	//Formato Keijzer
			WriteKFormat(In_Ptr, Out_Ptr);
			break;
		case 'A':	//Absorcion
			BranchOutA(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'F':	//Fluencia
			BranchOutF(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'R':	//Reflectancia
			BranchOutR(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'T':	//Transmision
			BranchOutT(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'H':	//AYUDA!
			ShowOutMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Mostrar el prompt para los datos de salida del archivo original sin convolucionar
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void OutputOrigData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else
		do 
		{
			printf("\n> Datos de salida original (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str)); //Hay que evitar la cadena NULL
			BranchOutCmd(cmd_str, In_Ptr, Out_Ptr);
		}
		while (toupper(cmd_str[0]) != 'Q');
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Contornos (menu)
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowContOrigMenu(char *in_fname)
{
	printf("A = Absorcion vs r & z [1/cm3]\n");
	printf("F = Fluencia vs r & z [1/cm2]\n");
	printf("R = Reflectancia difusa vs radio & angulo [1/(cm2 sr)]\n");
	printf("T = Transmitancia vs radio & angulo [1/(cm2 sr)]\n");
	printf("Q   = Salir al menu principal\n");
	printf("* Ingresar nombre de archivo: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Densidad de absorcion a fluencia. A = F/mua;
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void A2F(InputStruct * In_Ptr, double **A_rz)
{
	short nz = In_Ptr->nz, nr = In_Ptr->nr;
	double dr = In_Ptr->dr, dz = In_Ptr->dz;
	short ir, iz;
	double mua;

	for (ir = 0; ir < nr; ir++)
	{
		for (iz = 0; iz < nz; iz++) 
		{
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				A_rz[ir][iz] /= mua;
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Fluencia a densidad de absorcion. F = A*mua;
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void F2A(InputStruct * In_Ptr, double **A_rz)
{
	short       nz = In_Ptr->nz, nr = In_Ptr->nr;
	double      dr = In_Ptr->dr, dz = In_Ptr->dz;
	short       ir, iz;
	double      mua;

	for (ir = 0; ir < nr; ir++)
	{
		for (iz = 0; iz < nz; iz++) 
		{
			mua = In_Ptr->layerspecs[IzToLayer(iz, In_Ptr)].mua;
			if (mua > 0.0)
				A_rz[ir][iz] *= mua;
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Menu para la obtencion de contornos
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchContOrigCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':	//Absorcion vs r & z
			IsoPlot(Out_Ptr->A_rz, In_Ptr->nr - 1, In_Ptr->nz - 1, In_Ptr->dr, In_Ptr->dz);
			break;
		case 'F':	//Fluencia vs r & z
			A2F(In_Ptr, Out_Ptr->A_rz);
			IsoPlot(Out_Ptr->A_rz, In_Ptr->nr - 1, In_Ptr->nz - 1, In_Ptr->dr, In_Ptr->dz);
			F2A(In_Ptr, Out_Ptr->A_rz);
			break;
		case 'R':	//Reflectancia difusa vs radio & angulo
			IsoPlot(Out_Ptr->Rd_ra, In_Ptr->nr - 1, In_Ptr->na - 1, In_Ptr->dr, In_Ptr->da);
			break;
		case 'T':	//Transmitancia vs radio & angulo
			IsoPlot(Out_Ptr->Tt_ra, In_Ptr->nr - 1, In_Ptr->na - 1, In_Ptr->dr, In_Ptr->da);
			break;
		case 'H':	//Ayuda
			ShowContOrigMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir
			break;
		default:
			puts("...Comando erroneo");
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Mostrar el prompt para el ingreso de datos
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ContourOrigData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else
		do 
		{
			printf("\n> Salida de contorno de los datos de mclc (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str));	//Evitar la cadena NULL
			BranchContOrigCmd(cmd_str, In_Ptr, Out_Ptr);
		} 
		while (toupper(cmd_str[0]) != 'Q');
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escaneo
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowScanOrigMenu(char *in_fname)
{
	printf("Ar = Absorcion vs r @ z fija [1/cm3]\n");
	printf("Az = Absorcion vs z @ r fija [1/cm3]\n");
	printf("Fr = Fluencia vs r @ z fija [1/cm2]\n");
	printf("Fz = Fluencia vs z @ r fija [1/cm2]\n");
	printf("Rr = Reflectancia difusa vs r @ angulo fijo [1/(cm2 sr)]\n");
	printf("Ra = Reflectancia difusa vs angulo @ r fijo [1/(cm2 sr)]\n");
	printf("Tr = Transmitancia vs r @ angulo fijo [1/(cm2 sr)]\n");
	printf("Ta = Transmitancia vs angulo @ r fijo [1/(cm2 sr)]\n");
	printf("Q  = Salir\n");
	printf("* Nombre de archivo de entrada: %s \n", in_fname);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escanear el A_r original
 *	Ext es o "Ars" o "Frs".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigA_r(char *Ext, InputStruct * In_Ptr, double **A_rz)
{
	short ir, iz, nr = In_Ptr->nr, nz = In_Ptr->nz;
	double r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
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

	printf("\nseparacion de la rejilla en z is %-10.4lg cm.\n", dz);
	printf("Ingresar indice con z fija (0 - %2hd): ", nz - 1);
	iz = GetShort(0, nz - 1);
	fprintf(file, "%-12s\t%-s@z=%-9.3lg\n", "r[cm]", Ext, dz * (iz + 0.5));
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, A_rz[ir][iz]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escanear el A_z original
 *	Ext es o "Azs" o "Fzs".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigA_z(char *Ext, InputStruct * In_Ptr, double **A_rz)
{
	short ir, iz, nr = In_Ptr->nr, nz = In_Ptr->nz;
	double r, z, dr = In_Ptr->dr, dz = In_Ptr->dz;
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

	printf("\nseparacion de la rejilla en r es %-10.4lg cm.\n", dr);
	printf("Ingresar indice con r fijo (0 - %2hd): ", nr - 1);
	ir = GetShort(0, nr - 1);
	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "z[cm]", Ext, dr * (ir + 0.5));
	for (iz = 0; iz < nz; iz++) 
	{
		z = (iz + 0.5) * dz;
		fprintf(file, "%-12.4E\t%-12.4E\n", z, A_rz[ir][iz]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escanear el Rd_r original
 *	.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigRd_r(InputStruct * In_Ptr, double **Rd_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Rrs");
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

	printf("\nseparacion de la rejilla en angulo es %-10.4lg rad.\n", da);
	printf("Ingresar indice con angulo fijo (0 - %2hd): ", na - 1);
	ia = GetShort(0, na - 1);
	fprintf(file, "%-12s\t%-s@a=%-9.3lg\n", "r[cm]", fname, da * (ia + 0.5));
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, Rd_ra[ir][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Escanear el Rd_a original
 *	.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigRd_a(InputStruct * In_Ptr, double **Rd_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Ras");
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

	printf("\nseparacion de la rejilla en r es %-10.4lg cm.\n", dr);
	printf("Ingresar indice con r fijo (0 - %2hd): ", nr - 1);
	ir = GetShort(0, nr - 1);
	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "a[rad]", fname, dr * (ir + 0.5));
	for (ia = 0; ia < na; ia++) 
	{
		a = (ia + 0.5) * da;
		fprintf(file, "%-12.4E\t%-12.4E\n", a, Rd_ra[ir][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escanear el Tt_r original.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigTt_r(InputStruct * In_Ptr, double **Tt_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Trs");
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

	printf("\nseparacion de la rejilla en el angulo es %-10.4lg rad.\n", da);
	printf("Ingresar indice con angulo fijo (0 - %2hd): ", na - 1);
	ia = GetShort(0, na - 1);
	fprintf(file, "%-12s\t%-s@a=%-9.3lg\n", "r[cm]", fname, da * (ia + 0.5));
	for (ir = 0; ir < nr; ir++) 
	{
		r = (ir + 0.5) * dr;
		fprintf(file, "%-12.4E\t%-12.4E\n", r, Tt_ra[ir][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escanear el Tt_a original.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigTt_a(InputStruct * In_Ptr, double **Tt_ra)
{
	short ir, ia, nr = In_Ptr->nr, na = In_Ptr->na;
	double r, a, dr = In_Ptr->dr, da = In_Ptr->da;
	FILE *file;
	char fname[STRLEN];

	strcpy(fname, "Tas");
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

	printf("\nseparacion de la rejilla en r es %-10.4lg cm.\n", dr);
	printf("Ingresar indice con r fijo (0 - %2hd): ", nr - 1);
	ir = GetShort(0, nr - 1);
	fprintf(file, "%-12s\t%-s@r=%-9.3lg\n", "a[rad]", fname, dr * (ir + 0.5));
	for (ia = 0; ia < na; ia++) 
	{
		a = (ia + 0.5) * da;
		fprintf(file, "%-12.4E\t%-12.4E\n", a, Tt_ra[ir][ia]);
	}

	fclose(file);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Seccion del menu para la absorcion.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanOrigA(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char fname[STRLEN];

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':	//Absorcion vs r @ z fija
			strcpy(fname, "Ars");
			ScanOrigA_r(fname, In_Ptr, Out_Ptr->A_rz);
			break;
		case 'Z':	//Absorcion vs z @ r fija
			strcpy(fname, "Azs");
			ScanOrigA_z(fname, In_Ptr, Out_Ptr->A_rz);
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Seccion del menu para la Fluencia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanOrigF(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char fname[STRLEN];

	A2F(In_Ptr, Out_Ptr->A_rz);

	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':	//Fluencia vs r @ z fija
			strcpy(fname, "Frs");
			ScanOrigA_r(fname, In_Ptr, Out_Ptr->A_rz);
			break;
		case 'Z':	//Fluencia vs z @ r fija
			strcpy(fname, "Fzs");
			ScanOrigA_z(fname, In_Ptr, Out_Ptr->A_rz);
			break;
		default:
			puts("...Comando erroneo");
	}

	F2A(In_Ptr, Out_Ptr->A_rz);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Seccion de Reflectancia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanOrigR(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':	//Reflectancia difusa vs r @ angulo fijo
			ScanOrigRd_r(In_Ptr, Out_Ptr->Rd_ra);
			break;
		case 'A':	//Reflectancia difusa vs angulo @ r fijo
			ScanOrigRd_a(In_Ptr, Out_Ptr->Rd_ra);
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Seccion de Transmitancia.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanOrigT(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd_Str[1])) 
	{
		case 'R':	//Transmitancia vs r @ angulo fijo
			ScanOrigTt_r(In_Ptr, Out_Ptr->Tt_ra);
			break;
		case 'A':	//Transmitancia vs angulo @ r fijo
			ScanOrigTt_a(In_Ptr, Out_Ptr->Tt_ra);
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Menu principal para el escaneo.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchScanOrigCmd(char *Cmd_Str, InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch;

	switch (toupper(Cmd_Str[0])) 
	{
		case 'A':	//Absorcion
			BranchScanOrigA(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'F':	//Fluencia
			BranchScanOrigF(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'R':	//Reflectancia difusa
			BranchScanOrigR(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'T':	//Transmitancia
			BranchScanOrigT(Cmd_Str, In_Ptr, Out_Ptr);
			break;
		case 'H':	//Ayuda
			ShowScanOrigMenu(In_Ptr->in_fname);
			break;
		case 'Q':	//Salir
			break;
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Seccion de escaneo (llamar menus).
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScanOrigData(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char cmd_str[STRLEN];

	if (!Out_Ptr->allocated)
		puts("...No hay datos para la salida");
	else
		do 
		{
			printf("\n> Analisis (escaneo) de los datos de mcml (h para ayuda) => ");
			do
				gets(cmd_str);
			while (!strlen(cmd_str));	//Evitar la cadena NULL
			BranchScanOrigCmd(cmd_str, In_Ptr, Out_Ptr);
		} 
		while (toupper(cmd_str[0]) != 'Q');
}
