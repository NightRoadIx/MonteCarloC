/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *  Programa principal para la simulacion Monte Carlo de
 *  la distribucion de fotones en un medio turbio multi-capa
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

 /* * * * * * * * DEFINICIONES * * * * * * * */
/*
 *  THINKCPROFILER esta definido para generar llamadas de perfil en
 *	Think C. Si 1, hay que "encender" "Generate profiler
 *	calls" en el menú de opciones
 */
#define THINKCPROFILER 0
/* GNU cc no soporta difftime() y CLOCKS_PER_SEC */
#define GNUCC 0

/* Esta sección es verdadera cuando THINKCPROFILER es 1 */
#if THINKCPROFILER
#include <profile.h>
#include <console.h>
#endif

/* Librerias usadas */
/* Incluir la libreria con todas las estructuras que se van a requerir para la ejecución del programa */
#include "mcml.h"

 /* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * FUNCIONES PROTOTIPO
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
 /*
   Se declaran antes de usarse en el main(), ya que estás funciones están definidas
   en el archivo de cabecera
 */
FILE *GetFile(char *);
short ReadNumRuns(FILE* );
void ReadParm(FILE* , InputStruct * );
void CheckParm(FILE* , InputStruct * );
void InitOutputData(InputStruct, OutStruct *);
void FreeData(InputStruct, OutStruct *);
double Rspecular(LayerStruct * );
void LaunchPhoton(double, LayerStruct *, PhotonStruct *);
void HopDropSpin(InputStruct  *,PhotonStruct *,OutStruct *);
void SumScaleResult(InputStruct, OutStruct *);
void WriteResult(InputStruct, OutStruct, char *);

/*
 *  Si F = 0, reiniciar el reloj y regresar 0.
 *
 *  Si F = 1, pasar el tiempo de usuario a Msg e imprimirlo en
 *  pantalla, regresar el tiempo real ya que F=0
 *
 *  Si F = 2, es igual que si F=1, expecto que no se imprime nada
 *
 *  Hay que notar que clock() y time() regresan el tiempo de usuario
 *  y real respectivamente
 *  El tiempo de usuario es lo que el sistema asigan al tiempo de
 *  ejecución del programa.
 *  El tiempo real es el reloj del sistema. En un sistema compartido en tiempo
 *  estos pueden no ser iguales.
 *	
 *	clock() solo ocupa enteros de 16 bits, lo cual es alrededor de 32768 
 *	clock ticks.
 */
time_t PunchTime(char F, char *Msg)
{
	#if GNUCC //En caso de que se este usando GNUCC, que no soporta 
		return(0);
	#else
		static clock_t ut0;	// Referencia de tiempo del usuario
		static time_t  rt0;	// Referencia de tiempo real
		double secs;
		char s[STRLEN];
  
	/* Los posibles casos */
	if(F==0) 
	{
		ut0 = clock(); 		// Cargar a ut0 el valor que este en clock
		rt0 = time(NULL); 	// Cargar en rt0 el valor que este en time(NULL)
		return(0);			// Regresa 0
	}
	else if(F==1)  
	{
		secs = (clock() - ut0)/(double)CLOCKS_PER_SEC;		//Obtener segundos
		if (secs<0) 
			secs=0;	// clock() puede desbordarse
		//Crear la cadena con los datos de tiempo
		sprintf(s, "Tiempo ejecucion: %8.0lf sec = %8.2lf hr.  %s\n", secs, secs/3600.0, Msg);
		//Imprimir en pantalla
		puts(s);
    	strcpy(Msg, s); //Copiar la cadena s en Msg
		return(difftime(time(NULL), rt0)); //Regresar la diferencia de tiempos
	}
	else if(F==2) 	//De la misma forma regresar la diferencia de tiempos
		return(difftime(time(NULL), rt0));
	else 	// En cualquier otro caso, regesar un 0
		return(0);
#endif
}

/*
 * Imprimir el tiempo actual y el tiempo estimado para la finalización
 *
 *	P1 es en número de paquetes de fotones calculados
 *	Pt es el número total de paquetes de fotones.
 */
void PredictDoneTime(long P1, long Pt)	
{
	//Variables locales para el manejo de tiempo
	time_t now, done_time;
	struct tm *date;
	char s[80];
  
	now = time(NULL); 	//Obtener el tiempo actual
	date = localtime(&now); //El tiempo local
	//Copiar al puntero "s" en máx 80 espacios el tiempo date con el formato específico "%H:%M %x"
	strftime(s, 80, "%H:%M %x", date); 
	//Imprimir la cadena
	printf("Actual %s, ", s);
	
	//Ahora calcular el tiempo de ejecución
	done_time = now + (time_t) (PunchTime(2,"")/(double)P1*(Pt-P1));
	//Obtener el tiempo local con la variable antes obtenida
	date = localtime(&done_time);
	//Copiar el tiempo obtenido en el puntero s
	strftime(s, 80, "%H:%M %x", date);
	//Imprimir
	printf("Finaliza %s\n", s);
}

/*
 *	Reportar el tiempo y escribir los resultados
 */
void ReportResult(InputStruct In_Parm, OutStruct Out_Parm)
{
	char time_report[STRLEN];
	
	//Mostrar el tiempo de ejecución utilizado
	strcpy(time_report, " Tiempo de simulación de esta ejecucion.");
	PunchTime(1, time_report);
	
	//Mostrar resultados
	SumScaleResult(In_Parm, &Out_Parm);
	WriteResult(In_Parm, Out_Parm, time_report);
}

/*
 *	Obtener el nombre del archivo de los datos de entrada del archivo
 *  que fue llamado comom argumento en la línea de comandos
 */
void GetFnameFromArgv(int argc, char * argv[],char * input_filename)
{
	if(argc>=2) 
	{	// Copiar el nombre que esta como argumento en main(int argc, char *argv[])
		strcpy(input_filename, argv[1]);
	}
	else //En otro caso regresar el caractere nulo
		input_filename[0] = '\0';
} 

    
/*
 *	Ejecutar la simulación Monte Carlo para una ejecución independiente
 */
void DoOneRun(short NumRuns, InputStruct *In_Ptr)
{
	//Variables locales
	//Esta variable se le pide al compilador que la asigne en un registro con fines de aumentar la velocidad de ejecucion
	register long i_photon;	
	OutStruct out_parm;		// Distribucion de fotones
	PhotonStruct photon;
	long num_photons = In_Ptr->num_photons, photon_rep=10;

	//De nuevo al preprocesador se le indica en caso de que THINKCPROFILER este activo
	#if THINKCPROFILER
		InitProfile(200,200); cecho2file("prof.rpt",0, stdout);
	#endif
    
	//Iniciar los datos de salida
	InitOutputData(*In_Ptr, &out_parm);
	//La parte de la reflexión especular
	out_parm.Rsp = Rspecular(In_Ptr->layerspecs);	
	//Se asigan el contador de fotones
	i_photon = num_photons;
	//Y el "Punch Time" para contar el tiempo de ejecución
	PunchTime(0, "");
    
	/* Iniciar simulación */
	do 
	{
		//En caso de que se halla llegado al # de fotones == a photon_rep
		if(num_photons - i_photon == photon_rep) 
		{
			printf("%ld fotones & quedan %hd ejecuciones, ", i_photon, NumRuns);
			//Para el tiempo de ejecucion actual y el estimado
			PredictDoneTime(num_photons - i_photon, num_photons);
			//Multiplicar x10 este numero de fotones "photon_rep"
			photon_rep *= 10;
		}
		//Lanzar el foton con todos sus parametros
		LaunchPhoton(out_parm.Rsp, In_Ptr->layerspecs, &photon);
		//Ejecutar los pasos del fotón de HOP, DROP y SPIN, hasta que el fotón desaparezca
		do  
			HopDropSpin(In_Ptr, &photon, &out_parm);
		while (!photon.dead);
	} 
	while(--i_photon); //Hasta que todos los fotones hallan sido simulados
    
	//De nuevo si THINKCPROFILER esta activo, se termina abruptamente el programa
	#if THINKCPROFILER
		exit(0);
	#endif
    
	//Finalmente mandar a que se guarden los resultados
	ReportResult(*In_Ptr, out_parm);
	//Liberar memoria
	FreeData(*In_Ptr, &out_parm);
}

/* * * * * * * * PROGRAMA PRINCIPAL * * * * * * * */
/*
  El argumento a la línea de comandos es el nombre del archivo,
  si existe. Macintosh no soporta la línea de comandos.
*/
char main(int argc, char *argv[]) 
{
	//Variables locales
	char input_filename[STRLEN]; //Para manejar el archivo de entrada
	FILE *input_file_ptr; //Para el manejo del archivo
	short num_runs;	// Número de llamadas independientes
	InputStruct in_parm; //Estructura de entrada
	
	//Mostrar versión del programa
	ShowVersion("Version 1.2, 1993");
        system("cls");
	/* Obtener todos los parámetros de entrada del archivo  */
	GetFnameFromArgv(argc, argv, input_filename);
	input_file_ptr = GetFile(input_filename);
	CheckParm(input_file_ptr, &in_parm);	
	num_runs = ReadNumRuns(input_file_ptr);
	
	//Ejecutar mientras num_runs no sea 0
	while(num_runs--)  
	{
		//Leer los parametros del archivo de entrada
		ReadParm(input_file_ptr, &in_parm);
		DoOneRun(num_runs, &in_parm);
	}
	
	//Cerrar el archivo
	fclose(input_file_ptr);
	//Acabar el programa
	return(0);
}
