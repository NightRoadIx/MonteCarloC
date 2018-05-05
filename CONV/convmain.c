/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Programa de convolucion para la simulacion Monte Carlo de
 *	la distribucion de fotones en un medio turbio multicapa
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Librerias usadas */
/* Incluir la libreria con todas las estructuras que se van a requerir para la ejecución del programa */
#include "conv.h"

#define EPS 0.1	//Error relativo default en la convolucion

 /* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * FUNCIONES PROTOTIPO
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 /*
   Se declaran antes de usarse en el main(), ya que estás funciones están definidas
   en el archivo de cabecera
 */
void ReadMcoFile(InputStruct *, OutStruct *);
void ReadMcoFile2(InputStruct *, OutStruct *, char *);
void OutputOrigData(InputStruct *, OutStruct *);
void OutputConvData(InputStruct *, OutStruct *);
void OutputConvData2(InputStruct *, OutStruct *, char *);
void ContourOrigData(InputStruct *, OutStruct *);
void ContourConvData(InputStruct *, OutStruct *);
void ScanOrigData(InputStruct *, OutStruct *);
void ScanConvData(InputStruct *, OutStruct *);
void LaserBeam(BeamStruct *, OutStruct *);
void LaserBeam2(BeamStruct *, OutStruct *);
void ConvResolution(InputStruct *, OutStruct *);
void ConvError(InputStruct *, OutStruct *);

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para mostrar el menu principal
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ShowMainMenu()
{
	puts("i  = Ingresar el nombre del archivo de salida de la simulacion MCML.");
	puts("b  = Especificiar el tipo de haz de incidencia.");
	puts("r  = Resolucion de la convolucion.");
	puts("e  = Error de convolucion.");
	puts("oo = Datos de salida original.");
	puts("oc = Datos de salida de la convolucion.");
	puts("co = Salida de contorno de los datos originales");
	puts("cc = Salida de contorno de los datos convulocionados");
	puts("so = Escaneo de salida de los datos originales");
	puts("sc = Escaneo de salida de los datos convulocionados");
	puts("q  = Salir");
	puts("* [[Los comandos distinguen mayusculas y minusculas]] ");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Funcion para cuando se presiona "q" para salir del programa
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void QuitProgram(InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	char ch, cmd_str[STRLEN];

	printf("Realmente deseas salir (y/n): ");
	gets(cmd_str);
	sscanf(cmd_str, "%c", &ch);
	if(toupper(ch) == 'Y') 
	{
		if (Out_Ptr->allocated) 
		{
			FreeOrigData(In_Ptr, Out_Ptr);
			FreeConvData(In_Ptr, Out_Ptr);
		}
		exit(0);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion de caracteres de entrada (comandos) de una sola letra
 *  Cmd es el caracter de comando, se envian estructuras de entrada y salida
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmd1(char *Cmd, InputStruct *In_Ptr, OutStruct *Out_Ptr)
{
	switch (toupper(Cmd[0])) 
	{
		//Ingresar el nombre del archivo de salida de la simulacion MCML
		case 'I':
			ReadMcoFile(In_Ptr, Out_Ptr);
			break;
		//Especificar el tipo de haz de incidencia
		case 'B':
			LaserBeam(&In_Ptr->beam, Out_Ptr);
			break;
		//Especificar la resolucion de la convolucion
		case 'R':
			ConvResolution(In_Ptr, Out_Ptr);
			break;
		//Especificar el error de la convolucion
		case 'E':
			ConvError(In_Ptr, Out_Ptr);
			break;
		//Mostrar la ayuda
		case 'H':
			ShowMainMenu();
			break;
		//Salir del programa
		case 'Q':
			QuitProgram(In_Ptr, Out_Ptr);
			break;
		//Comando erroneo
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion de caracteres de entrada (comandos) de una sola letra
 *  Cmd es el caracter de comando, se envian estructuras de entrada y salida
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmd1x(char *Cmd, InputStruct *In_Ptr, OutStruct *Out_Ptr, char *archon)
{
	printf("2 ");
	switch (toupper(Cmd[0])) 
	{
		//Ingresar el nombre del archivo de salida de la simulacion MCML
		case 'I':
			printf("I ");
			ReadMcoFile2(In_Ptr, Out_Ptr, archon);
			break;
		//Especificar el tipo de haz de incidencia
		case 'B':
			printf("B ");
			LaserBeam2(&In_Ptr->beam, Out_Ptr);
			break;
		//Especificar la resolucion de la convolucion
		case 'R':
			ConvResolution(In_Ptr, Out_Ptr);
			break;
		//Especificar el error de la convolucion
		case 'E':
			ConvError(In_Ptr, Out_Ptr);
			break;
		//Mostrar la ayuda
		case 'H':
			ShowMainMenu();
			break;
		//Salir del programa
		case 'Q':
			QuitProgram(In_Ptr, Out_Ptr);
			break;
		//Comando erroneo
		default:
			puts("...Comando erroneo");
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion de caracteres de entrada (comandos) de dos letras
 *  Cmd es el caracter de comando, se envian estructuras de entrada y salida
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmd2(char *Cmd,InputStruct * In_Ptr, OutStruct * Out_Ptr)
{
	switch (toupper(Cmd[0])) 
	{
		case 'O':
			if (toupper(Cmd[1]) == 'O') //Datos de salida del archivo original
				OutputOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Datos de salida de la convolucion
				OutputConvData(In_Ptr, Out_Ptr);
			break;
		case 'C':
			if (toupper(Cmd[1]) == 'O')	//Salida de contorno de los datos convulocionados
				ContourOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Escaneo de salida de los datos originales
				ContourConvData(In_Ptr, Out_Ptr);
			break;
		case 'S':
			if (toupper(Cmd[1]) == 'O')	//Escaneo de salida de los datos originales
				ScanOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Escaneo de salida de los datos convulocionados
				ScanConvData(In_Ptr, Out_Ptr);
			break;
		default:
			puts("...Comando erroneo");
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion de caracteres de entrada (comandos) de dos letras
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmd2x(char *Cmd,InputStruct * In_Ptr, OutStruct * Out_Ptr, char *archon)
{
	switch (toupper(Cmd[0])) 
	{
		case 'O':
			if (toupper(Cmd[1]) == 'O') //Datos de salida del archivo original
				OutputOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Datos de salida de la convolucion
				OutputConvData2(In_Ptr, Out_Ptr, archon);
			break;
		case 'C':
			if (toupper(Cmd[1]) == 'O')	//Salida de contorno de los datos convulocionados
				ContourOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Escaneo de salida de los datos originales
				ContourConvData(In_Ptr, Out_Ptr);
			break;
		case 'S':
			if (toupper(Cmd[1]) == 'O')	//Escaneo de salida de los datos originales
				ScanOrigData(In_Ptr, Out_Ptr);
			else if (toupper(Cmd[1]) == 'C') //Escaneo de salida de los datos convulocionados
				ScanConvData(In_Ptr, Out_Ptr);
			break;
		default:
			puts("...Comando erroneo");
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Seleccion de caracteres de entrada (comandos) del menu principal
 *  Cmd es el caracter de comando, se envian estructuras de entrada y salida
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmd(char *Cmd)
{
	static InputStruct in_parm;
	static OutStruct out_parm = NULLOUTSTRUCT;
	static char first_time = 1;	//Usado para cuando se inicializa

	if (first_time) //Cuando se inicia por primera vez el programa
	{
		in_parm.eps = EPS;
		first_time = 0;
	}
	if (strlen(Cmd) == 1) //Cuando son comandos de una sola letra
		BranchMainCmd1(Cmd, &in_parm, &out_parm);
	else if (strlen(Cmd) == 2) //Cuando son comandos de dos letras
		BranchMainCmd2(Cmd, &in_parm, &out_parm);
	else
		puts("...Comando erroneo");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Se manda el caracter de comando y una cadena con el nombre del archivo a abrir
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void BranchMainCmdx(char *Cmd, char *archon)
{
	static InputStruct in_parm;
	static OutStruct out_parm = NULLOUTSTRUCT;
	static char first_time = 1;	//Usado para cuando se inicializa

	if (first_time) //Cuando se inicia por primera vez el programa
	{
		in_parm.eps = EPS;
		first_time = 0;
	}
	printf("1 ");
	if (strlen(Cmd) == 1) //Cuando son comandos de una sola letra
		BranchMainCmd1x(Cmd, &in_parm, &out_parm, archon);
	else if (strlen(Cmd) == 2) //Cuando son comandos de dos letras
		BranchMainCmd2x(Cmd, &in_parm, &out_parm, archon);
	else
		puts("...Comando erroneo");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Programa principal
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void main()
{
	char cmd_str[STRLEN];

	char *cad1[3]={"cL","cM","cH"};
        char *cad2[2]={"o","d"};
        char *cad3[3]={"3","1","0"};
        char *cad4[2]={"660", "940"};
        char *cad5[11]={"10","20","30","40","50","60","70","80","90","95","99"};
        char *arch, *rvn;
        int g, h, i, j, k;

	ShowVersion();
        system("cls");
		
/*							//Crer el nombre del archivo
							//sprintf(arch, "%s%s%s%s%s.mco\n", cad1[1], cad2[1], cad3[1], cad4[1], cad5[1]);
							strcpy(arch,"cMd194020.mco");
							//Mostrar en pantalla en que archivo se encuentra el proceso
							puts(arch);
							
							//Enviar el comando de apertura de archivo y el nombre
							BranchMainCmdx("i", arch);
							//Cargar el haz de incidencia
							BranchMainCmdx("b", arch);
							//Calcular la convolución de los datos
							BranchMainCmdx("oc", arch);*/
		
	/*do
	{
		printf("\n> Menu principal (h para ayuda) => ");
		//Obtener la cadena de comando
		do
			gets(cmd_str);
		while (!strlen(cmd_str));
		BranchMainCmd(cmd_str);
	}
	while (1);*/

        for(g=0; g < 3; g++)
        {
			for(h=0; h < 2; h++)
			{
				for(i=0; i < 3; i++)
				{
					for(j=0; j < 2; j++)
					{
						for(k=0; k < 11; k++)
						{
							//Crer el nombre del archivo
							sprintf(arch, "%s%s%s%s%s.mco", cad1[g], cad2[h], cad3[i], cad4[j], cad5[k]);
							//Mostrar en pantalla en que archivo se encuentra el proceso
							puts(arch);
							
							//Enviar el comando de apertura de archivo y el nombre
							BranchMainCmdx("i", arch);
							//Cargar el haz de incidencia
							BranchMainCmdx("b", arch);
							//Calcular la convolución de los datos
							BranchMainCmdx("oc", arch);
							
							//getch();
						}
					}
				}
			}	
		}
}
