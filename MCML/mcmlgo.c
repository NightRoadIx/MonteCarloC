/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright Univ. of Texas M.D. Anderson Cancer Center
 *  1992.
 *
 *	Lanza, mueve y graba el peso del foton.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */

 //Se incluye la libreria creada mcml.h
#include "mcml.h"

//Probar el programa usando una semilla aleatoria (rnd) fija
#define STANDARDTEST 0

// 1=foton dividido, 0=reflexion estadistica
#define PARTIALREFLECTION 0     

// Coseno de alrededor de 1e-6 rad
#define COSZERO (1.0-1.0E-12)	

// Coseno de alrededor de 1.57 - 1e-6 rad
#define COS90D  1.0E-6		

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Generador de numeros aleatorios de Numerical Recipes in C.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*DEFINIR ALGUNOS TERMINOS*/
#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC 1.0E-9

float ran3(int *idum)
{
	static int inext,inextp;
	static long ma[56];
	static int iff=0;
	long mj,mk;
	int i,ii,k;
  
	//Colocar la semilla
	if (*idum < 0 || iff == 0) 
	{
		iff=1;
		mj=MSEED-(*idum < 0 ? -*idum : *idum);
		mj %= MBIG;
		ma[55]=mj;
		mk=1;
		for (i=1;i<=54;i++) 
		{
			ii=(21*i) % 55;
			ma[ii]=mk;
			mk=mj-mk;
			if (mk < MZ) 
				mk += MBIG;
			mj=ma[ii];
		}
		for (k=1;k<=4;k++)
			for (i=1;i<=55;i++)
			{
				ma[i] -= ma[1+(i+30) % 55];
				if (ma[i] < MZ) 
					ma[i] += MBIG;
			}
			
		inext=0;
		inextp=31;
		*idum=1;
	}
	if (++inext == 56) 
		inext=1;
	if (++inextp == 56) 
		inextp=1;
	mj=ma[inext]-ma[inextp];
	if (mj < MZ) 
		mj += MBIG;
	ma[inext]=mj;
	
	return mj*FAC;
}
/*QUITAR LAS DEFINICIONES*/
#undef MBIG
#undef MSEED
#undef MZ
#undef FAC


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Generar un numero aleatorio entre 0 & 1. Tomar un
 *	numero como semilla la primera vez que se entra a una funcion.
 *	La semilla esta limitada a 1<<15.  
 *	Se ha encontrado que si idum es muy grande, ran3 regresa
 *	numeros fuera de 0 & 1.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double RandomNum(void)
{
	static Boolean first_time=1;
	static int idum;	//La semilla para ran3
  
	if(first_time) 
	{
		//Usar la semilla fijada para probar el programa 
		#if STANDARDTEST
			idum = - 1;
		#else
			//Usar un entero de 16 bits como la semilla
			idum = -(int)time(NULL)%(1<<15);
		#endif 
		ran3(&idum);
		first_time = 0;
		idum = 1;
	}
  
	return( (double)ran3(&idum) );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Calcular la reflexion especular.
 *
 *	Si la primera capa es un medio turbio, usar la reflexion
 *	de Fresnel de la frontera de la primera capa como
 *	la reflectancia especular.
 *
 *	Si la primera capa es vidrio, se consideran multiples
 *	reflexiones en la primera capa para obtener la reflectancia
 *	especular.
 *
 *	La subrutina asuma que el arreglo Layerspecs esta
 *	correctamente iniciado.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double Rspecular(LayerStruct *Layerspecs_Ptr)
{
	double r1, r2; //Reflexiones directas de las capas 1 & 2
	double temp;
  
	temp =(Layerspecs_Ptr[0].n - Layerspecs_Ptr[1].n)/(Layerspecs_Ptr[0].n + Layerspecs_Ptr[1].n);
	r1 = temp*temp;
  
	//Si la capa es de vidrio (los coeficientes de absorcion y esparcimiento son 0)
	if((Layerspecs_Ptr[1].mua == 0.0) && (Layerspecs_Ptr[1].mus == 0.0))  
	{
		temp = (Layerspecs_Ptr[1].n - Layerspecs_Ptr[2].n)/(Layerspecs_Ptr[1].n + Layerspecs_Ptr[2].n);
		r2 = temp*temp;
		r1 = r1 + (1-r1)*(1-r1)*r2/(1-r1*r2);
	}
  
	return (r1);	
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Iniciar el paquete de foton.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LaunchPhoton(double Rspecular, LayerStruct *Layerspecs_Ptr, PhotonStruct *Photon_Ptr)
{
	Photon_Ptr->w	 	= 1.0 - Rspecular;	
	Photon_Ptr->dead 	= 0;
	Photon_Ptr->layer = 1;
	Photon_Ptr->s	= 0;
	Photon_Ptr->sleft= 0;
  
	Photon_Ptr->x 	= 0.0;	
	Photon_Ptr->y	 	= 0.0;	
	Photon_Ptr->z	 	= 0.0;	
	Photon_Ptr->ux	= 0.0;	
	Photon_Ptr->uy	= 0.0;	
	Photon_Ptr->uz	= 1.0;	
  
	//Capa de vidrio
	if((Layerspecs_Ptr[1].mua == 0.0) && (Layerspecs_Ptr[1].mus == 0.0))  
	{
		Photon_Ptr->layer 	= 2;
		Photon_Ptr->z	= Layerspecs_Ptr[2].z0;	
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escoger (muestrear) un nuevo angulo theta para la
 *	propagacion de fotones de acuerdo a la anisotropia.
 *
 *	Si la anisotropia g es 0, entonces
 *		cos(theta) = 2*rand-1.
 *	de otra forma
 *		muestrear de acuerdo a la funcion Henyey-Greenstein.
 *
 *	Regresar el coseno del angulo de desviacion polar theta.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double SpinTheta(double g)
{
	double cost;
  
	if(g == 0.0) 
		cost = 2*RandomNum() -1;
	else 
	{
		double temp = (1-g*g)/(1-g+2*g*RandomNum());
		cost = (1+g*g - temp*temp)/(2*g);
		if(cost < -1) 
			cost = -1;
		else if(cost > 1) 
			cost = 1;
	}
	
	return(cost);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Escoger una nueva direccion para la propagacion de
 *	fotones por el muestreo del angulo polar de desviacion 
 *	theta y el angulo azimutal psi.
 *
 *	Nota:
 *  	theta: 0 - pi por lo que sen(theta) siempre es positivo
 *  	puede usarse sqrt() para cos(theta)
 * 
 *  	psi:   0 - 2pi 
 *  	para 0-pi  sen(psi) es + 
 *  	para pi-2pi sen(psi) es - 
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Spin(double g, PhotonStruct * Photon_Ptr)
{
	//El cos & sen del angulo polar de desviacion theta
	double cost, sint;
	//El cos & sen del angulo azimutal psi
	double cosp, sinp;
	double ux = Photon_Ptr->ux;
	double uy = Photon_Ptr->uy;
	double uz = Photon_Ptr->uz;
	double psi;

	cost = SpinTheta(g);
	//Hay que recordar que sqrt() es mas rapido que sin()
	sint = sqrt(1.0 - cost*cost);	

	psi = 2.0*PI*RandomNum(); //giro de psi de 0 a 2pi
	cosp = cos(psi);
	if(psi<PI)
		sinp = sqrt(1.0 - cosp*cosp); //de nuevo sqrt() mas rapido
	else
		sinp = - sqrt(1.0 - cosp*cosp);	
  
	if(fabs(uz) > COSZERO)  //Normal incidente
	{
		Photon_Ptr->ux = sint*cosp;
		Photon_Ptr->uy = sint*sinp;
		//La funcion SIGN() es mas rapida que la division
		Photon_Ptr->uz = cost*SIGN(uz);	
	}
	else  //Regular incidente
	{
		double temp = sqrt(1.0 - uz*uz);
		Photon_Ptr->ux = sint*(ux*uz*cosp - uy*sinp)/temp + ux*cost;
		Photon_Ptr->uy = sint*(uy*uz*cosp + ux*sinp)/temp + uy*cost;
		Photon_Ptr->uz = -sint*cosp*temp + uz*cost;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Mover el foton s lejos en la capa actual del medio
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Hop(PhotonStruct *	Photon_Ptr)
{
	double s = Photon_Ptr->s;	//Se asigna el foton s

	//Se mueve en las direeciones x, y & z
	Photon_Ptr->x += s*Photon_Ptr->ux;
	Photon_Ptr->y += s*Photon_Ptr->uy;
	Photon_Ptr->z += s*Photon_Ptr->uz;
}			

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Si uz != 0, regresa el tamaño del paso del foton en vidrio,
 *	de otra forma, regresa 0
 *
 *	El tamaño del paso es la distance entre la posicion actual
 *	y el limite en la direccion del foton
 *
 *	Verificar que uz !=0 antes de llamar a esta funcion
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void StepSizeInGlass(PhotonStruct *  Photon_Ptr,InputStruct  *  In_Ptr)
{
	double dl_b;	//Tamaño del paso a la frontera
	short  layer = Photon_Ptr->layer;
	double uz = Photon_Ptr->uz;
  
	//Tamaño de paso a la frontera
	if(uz>0.0)
		dl_b = (In_Ptr->layerspecs[layer].z1 - Photon_Ptr->z)/uz;
	else if(uz<0.0)
		dl_b = (In_Ptr->layerspecs[layer].z0 - Photon_Ptr->z)/uz;
	else
		dl_b = 0.0;
  
	Photon_Ptr->s = dl_b;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Tomar un tamaño de paso para un paquete de foton cuando
 *	esta en el tejido.
 *	Si el miembro (sleft) es cero, hacer un nuevo tamaño de paso con:
 *	 -log(rnd)/(mua+mus).
 *	de otra forma, Otherwise, recoger el sobrante en sleft
 *
 *	La capa es el indice de la capa
 *	In_Ptr contiene los parametros de entrada
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void StepSizeInTissue(PhotonStruct * Photon_Ptr,InputStruct  * In_Ptr)
{
	short  layer = Photon_Ptr->layer;
	double mua = In_Ptr->layerspecs[layer].mua;
	double mus = In_Ptr->layerspecs[layer].mus;
  
	if(Photon_Ptr->sleft == 0.0) //Hacer un nuevo paso
	{
		double rnd;

		//Generar un numero aleatorio, evitando que sea cero
		do 
			rnd = RandomNum(); 
		while( rnd <= 0.0 );
		Photon_Ptr->s = -log(rnd)/(mua+mus);
	}
	else //tomar lo que sobra
	{
		Photon_Ptr->s = Photon_Ptr->sleft/(mua+mus);
		Photon_Ptr->sleft = 0.0;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Veriricar si el paso golperara la frontera.
 *	Regresa 1 si golpea la frontera
 *	Regresa 0 de otra forma.
 *
 * 	Si el paso proyectado golpea la frontera, los miembros
 *	s & sleft de Photon_Ptr son actualizados.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
Boolean HitBoundary(PhotonStruct *  Photon_Ptr,InputStruct  *  In_Ptr)
{
	double dl_b;  //Longitud a la frontera
	short  layer = Photon_Ptr->layer;
	double uz = Photon_Ptr->uz;
	Boolean hit;
  
	//Distancia a la frontera
	if(uz>0.0)
		dl_b = (In_Ptr->layerspecs[layer].z1 - Photon_Ptr->z)/uz;	// dl_b>0.
	else if(uz<0.0)
		dl_b = (In_Ptr->layerspecs[layer].z0 - Photon_Ptr->z)/uz;	// dl_b>0.
  
	if(uz != 0.0 && Photon_Ptr->s > dl_b) 
	{
		//No horizontal o cruzando
		double mut = In_Ptr->layerspecs[layer].mua + In_Ptr->layerspecs[layer].mus;

		Photon_Ptr->sleft = (Photon_Ptr->s - dl_b)*mut;
		Photon_Ptr->s    = dl_b;
		hit = 1;
	}
	else
		hit = 0;
  
	return(hit);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Drop(dejar) el peso del foton dentro del tejido (no vidrio).
 *
 *  Se asume que el foton no esta muerto (vivo)
 *
 *	El peso perdido es is dw = w*mua/(mua+mus).
 *
 *	El paso perdido es asigano a los elementos del arreglo de absorcion 
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Drop(InputStruct  *In_Ptr, PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	double dwa;		//peso absorbido
	double x = Photon_Ptr->x;
	double y = Photon_Ptr->y;
	short  iz, ir;	//Indice para z & r
	short  layer = Photon_Ptr->layer;
	double mua, mus;		
  
	//Calcular los indices del arreglo
	iz = (short)(Photon_Ptr->z/In_Ptr->dz);
	if(iz>In_Ptr->nz-1) 
		iz=In_Ptr->nz-1;
  
	ir = (short)(sqrt(x*x+y*y)/In_Ptr->dr);
	if(ir>In_Ptr->nr-1) 
		ir=In_Ptr->nr-1;
  
	//Actualizar el peso del foton
	mua = In_Ptr->layerspecs[layer].mua;
	mus = In_Ptr->layerspecs[layer].mus;
	dwa = Photon_Ptr->w * mua/(mua+mus);
	Photon_Ptr->w -= dwa;
  
	//Asignar dwa al arreglo de absorcion en la posicion (ir,iz)
	Out_Ptr->A_rz[ir][iz]	+= dwa;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Si el peso del foton es muy pequeño (), 
 *	entonces el paquete del foton intenta sobrevivir a
 *  la ruleta
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Roulette(PhotonStruct *Photon_Ptr)
{
	//Sin peso el foton esta muerto
	if(Photon_Ptr->w == 0.0)	
		Photon_Ptr->dead = 1;
	//generar un numero aleatorio y si es menor que CHANCE
	//sobrevive a la ruleta
	else if(RandomNum() < CHANCE)
		Photon_Ptr->w /= CHANCE;
	//si no sobrevive a la ruleta, entonces tambien muere
	else 
		Photon_Ptr->dead = 1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Calcular la reflectancia de Fresnel.
 *
 *	Asegurarse que el coseno del angulo incidente a1
 *	es positivo, y el caso cuando el angulo es mayor 
 *	que el angulo critico esta decartado.
 *
 * 	Se Evita operaciones con funciones trigonometricas
 *	todo lo posible, por que requieren de una carga/tiempo computacional
 *  importante
 *  
 *  Se utiliza como argumentos de la funcion:
 *   n1->indice de refraccion incidente
 *   n2->indice de refraccion transmitido
 *   cal->coseno del angulo indicente, (0,90) grados
 *   *ca2_Ptr->apuntador al coseno del angulo de transmision, a2>0
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
double RFresnel(double n1,double n2,double ca1,double * ca2_Ptr)
{
	double r;
  
	if(n1==n2) //Indices de refraccion iguales (Matched boundary)
	{
		*ca2_Ptr = ca1;
		r = 0.0;
	}
	else if(ca1>COSZERO) //Normalmente incidente
	{
		*ca2_Ptr = ca1;
		r = (n2-n1)/(n2+n1);
		r *= r;
	}
	else if(ca1<COS90D)  //Muy inclinado, Coseno de alrededor de 1.57 - 1e-6 rad
	{
		*ca2_Ptr = 0.0;
		r = 1.0;
	}
	else    //Otros angulos de incidencia
	{
		double sa1, sa2; //Sin del angulo de incidencia y transmision
		double ca2;
    
		sa1 = sqrt(1-ca1*ca1);
		sa2 = n1*sa1/n2;
		if(sa2>=1.0) 
		{	
			//Corroborar para la reflexion total interna
			*ca2_Ptr = 0.0;
			r = 1.0;
		}
		else  
		{
			//Los cosenos de la suma ap o la diferencia am de los dos angulos
			// ap = a1 + a2
			// am = a1 - a2
			double cap, cam;
			//Los senos de la suma o la diferencia
			double sap, sam;
      
			*ca2_Ptr = ca2 = sqrt(1-sa2*sa2);
      
			cap = ca1*ca2 - sa1*sa2; /* c+ = cc - ss. */
			cam = ca1*ca2 + sa1*sa2; /* c- = cc + ss. */
			sap = sa1*ca2 + ca1*sa2; /* s+ = sc + cs. */
			sam = sa1*ca2 - ca1*sa2; /* s- = sc - cs. */
			//Se reordena para mejorar la velocidad de ejecucion (tunning)
			r = 0.5*sam*sam*(cam*cam+cap*cap)/(sap*sap*cam*cam); 
		}
	}
	return(r);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Grabar el peso del foton saliendo de la primera capa (uz<0),
 *	no importando si la capa es vidrio o no, para agregar
 *	al arreglo de reflexion
 *
 *	Actualizar también el peso del foton.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RecordR(double	Refl,InputStruct *In_Ptr,PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	double x = Photon_Ptr->x;
	double y = Photon_Ptr->y;
	//Indice para r & el angulo
	short  ir, ia;
  
	ir = (short)(sqrt(x*x+y*y)/In_Ptr->dr);
	if(ir > In_Ptr->nr-1) 
		ir=In_Ptr->nr-1;
  
	ia = (short)(acos(-Photon_Ptr->uz)/In_Ptr->da);
	if(ia > In_Ptr->na-1) 
		ia=In_Ptr->na-1;
  
	//Asignar el peso del foton al arreglo de reflexion
	Out_Ptr->Rd_ra[ir][ia] += Photon_Ptr->w*(1.0-Refl);
  
	Photon_Ptr->w *= Refl;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Grabar el paso del foton que sale de la ultima capa (uz>0),
 *	no importando si la capa es vidrio o no, al arreglo de
 *	transmitancia.
 *
 *	Actualizar tambien el peso del foton
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RecordT(double Tral, InputStruct *In_Ptr, PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	double x = Photon_Ptr->x;
	double y = Photon_Ptr->y;
	short  ir, ia;	//Indice para r & el angulo
  
	ir = (short)(sqrt(x*x+y*y)/In_Ptr->dr);
	if(ir>In_Ptr->nr-1) 
		ir=In_Ptr->nr-1;
  
	ia = (short)(acos(Photon_Ptr->uz)/In_Ptr->da);
	if(ia>In_Ptr->na-1) 
		ia=In_Ptr->na-1;
  
	//Asignar el peso del foton al arreglo de transmitancia
	Out_Ptr->Tt_ra[ir][ia] += Photon_Ptr->w*(1.0-Tral);
  
	Photon_Ptr->w *= Tral;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Decidir si ele foton sera transmitido o reflejado
 *	en la frontera superior (uz<0) de la capa actual.
 *
 *	Si "capa" es la primera capa, el paquete de foton sera
 *	parcialmente transmitido y reflejado si 
 *	PARTIALREFLECTION se coloca a 1,
 *	o si el paquete de foton sera transmitido o reflejado
 *	determinado estadisticamente si
 *	PARTIALREFLECTION se coloca a 0.
 *
 *	Grabar el paso del foton transmitido como reflexion
 *
 *	Si la "capa" no es la primera capa y el paquete de foton
 *	es transmitido, mover el foton a "layer-1"
 *
 *	Actualizar los parametros del foton.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CrossUpOrNot(InputStruct *In_Ptr, PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	double uz = Photon_Ptr->uz; //Coseno direccional en z
	double uz1;	//Cosenos de transmision alfa siempre positivos
	double r=0.0;	//Reflectancia
	short  layer = Photon_Ptr->layer;
	double ni = In_Ptr->layerspecs[layer].n;
	double nt = In_Ptr->layerspecs[layer-1].n;
  
	//Obtener r
	if( - uz <= In_Ptr->layerspecs[layer].cos_crit0) 
		r=1.0;	//En caso de reflexion interna total
	else 
		r = RFresnel(ni, nt, -uz, &uz1);
  
	#if PARTIALREFLECTION
		if(layer == 1 && r<1.0) //Parcialmente transmitido
		{
			Photon_Ptr->uz = -uz1;	//foton transmitido
			RecordR(r, In_Ptr, Photon_Ptr, Out_Ptr);
			Photon_Ptr->uz = -uz;	//foton reflejado
		}		
		else if(RandomNum() > r) //Transmitido a la capa-1
		{
			Photon_Ptr->layer--;
			Photon_Ptr->ux *= ni/nt;
			Photon_Ptr->uy *= ni/nt;
			Photon_Ptr->uz = -uz1;
		}
		else //Reflejado
			Photon_Ptr->uz = -uz;
	#else
		if(RandomNum() > r)  //Transmitido a la capa-1
		{
			if(layer==1) 
			{
				Photon_Ptr->uz = -uz1;
				RecordR(0.0, In_Ptr, Photon_Ptr, Out_Ptr);
				Photon_Ptr->dead = 1;
			}
			else 
			{
				Photon_Ptr->layer--;
				Photon_Ptr->ux *= ni/nt;
				Photon_Ptr->uy *= ni/nt;
				Photon_Ptr->uz = -uz1;
			}
		}
		else //reflejado
			Photon_Ptr->uz = -uz;
	#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Decidir si el foton sera transmitido o reflejado
 *	en la frontera inferior (uz>0) de la capa actual.
 *
 *  La funcion se comporta como CrossUpOrNot
 *
 *	Si el foton es transmitido, mover el foton a
 *	"layer+1". Si "layer" es a ultima capa, grabar el
 *	peso transmitido como transmitancia.
 *
 *	Actualizar los parametros del foton
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CrossDnOrNot(InputStruct *In_Ptr, PhotonStruct *Photon_Ptr, OutStruct *Out_Ptr)
{
	double uz = Photon_Ptr->uz; //Coseno direccional z
	double uz1;	//Cosenos de transmision alfa
	double r=0.0;	//reflectancia
	short  layer = Photon_Ptr->layer;
	double ni = In_Ptr->layerspecs[layer].n;
	double nt = In_Ptr->layerspecs[layer+1].n;
  
	//Obtener r
	if( uz <= In_Ptr->layerspecs[layer].cos_crit1) 
		r=1.0;	//En el caso de la reflexion interna total
	else 
		r = RFresnel(ni, nt, uz, &uz1); //Obtenerlo en otro caso
  
	#if PARTIALREFLECTION	
		if(layer == In_Ptr->num_layers && r<1.0)
		{
			Photon_Ptr->uz = uz1;
			RecordT(r, In_Ptr, Photon_Ptr, Out_Ptr);
			Photon_Ptr->uz = -uz;
		}
		else if(RandomNum() > r) //Transmitido a la capa+1
		{
			Photon_Ptr->layer++;
			Photon_Ptr->ux *= ni/nt;
			Photon_Ptr->uy *= ni/nt;
			Photon_Ptr->uz = uz1;
		}
		else //Reflejado
			Photon_Ptr->uz = -uz;
	#else
		if(RandomNum() > r) //Transmitido a la capa+1
		{
			if(layer == In_Ptr->num_layers) 
			{
				Photon_Ptr->uz = uz1;
				RecordT(0.0, In_Ptr, Photon_Ptr, Out_Ptr);
				Photon_Ptr->dead = 1;
			}
			else 
			{
				Photon_Ptr->layer++;
				Photon_Ptr->ux *= ni/nt;
				Photon_Ptr->uy *= ni/nt;
				Photon_Ptr->uz = uz1;
			}
		}
		else //Reflejado
			Photon_Ptr->uz = -uz;
	#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Verificar si el foton cruzo alguna frontera
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CrossOrNot(InputStruct *In_Ptr, PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	if(Photon_Ptr->uz < 0.0)
		CrossUpOrNot(In_Ptr, Photon_Ptr, Out_Ptr);
	else
		CrossDnOrNot(In_Ptr, Photon_Ptr, Out_Ptr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Mover el paquete del foton en la capa de vidrio.
 *	Los fontes horizontales son destruidos por que ellos
 *	no interactuaran con el tejido de nuevo.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HopInGlass(InputStruct *In_Ptr,PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	double dl; //Tamaño del paso 1/cm
  
	if(Photon_Ptr->uz == 0.0) 
	{ 
		//Los fotones horiontales en vidrio son destruidos
		Photon_Ptr->dead = 1;
	}
	else //Todos los demas si interactuan con el tejido
	{
		StepSizeInGlass(Photon_Ptr, In_Ptr);
		Hop(Photon_Ptr);
		CrossOrNot(In_Ptr, Photon_Ptr, Out_Ptr);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	Colocar un tamaño de paso, mover le foton, quitar algo de peso
 *	escoger una nueva direccion del foton para la propagacion.
 *
 *	Cuando un tamaño de paso del foton es lo suficientemente largo para 
 *	golpear una interfase, este paso esta dividido en dos pasos.
 *	Primero, mover el foton a la frontera libre de absorcion
 *	o esparcimiento, entonces decidir si el foton es reflejado
 *	o transmitido.
 *	Entonces mover el foton en la capa actual o en la transmitida
 *	con el tamaño del paso no terminado para el sitio de interaccion
 *	Si el tamaño de paso no terminado aun sigue muy grande, 
 *	se repite el proceso.
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HopDropSpinInTissue(InputStruct *In_Ptr,PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	StepSizeInTissue(Photon_Ptr, In_Ptr);
  
	if(HitBoundary(Photon_Ptr, In_Ptr)) 
	{
		Hop(Photon_Ptr); //Mover al plano de la frontera
		CrossOrNot(In_Ptr, Photon_Ptr, Out_Ptr);
	}
	else 
	{
		Hop(Photon_Ptr);
		Drop(In_Ptr, Photon_Ptr, Out_Ptr);
		Spin(In_Ptr->layerspecs[Photon_Ptr->layer].g, Photon_Ptr);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  La funcion de HOP, DROP & SPIN para el foton en el medio
 *  ya sea vidrio o tejido, ademas de que si el peso del foton
 *  disminuye a un limite establecido, entra a la ruleta para verificar
 *  si continua o no "vivo".
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HopDropSpin(InputStruct *In_Ptr,PhotonStruct *Photon_Ptr,OutStruct *Out_Ptr)
{
	short layer = Photon_Ptr->layer;

	if((In_Ptr->layerspecs[layer].mua == 0.0) && (In_Ptr->layerspecs[layer].mus == 0.0)) 
		//La capa de vidrio
		HopInGlass(In_Ptr, Photon_Ptr, Out_Ptr);
	else
		HopDropSpinInTissue(In_Ptr, Photon_Ptr, Out_Ptr);
  
	if( Photon_Ptr->w < In_Ptr->Wth && !Photon_Ptr->dead) 
		Roulette(Photon_Ptr);
}
