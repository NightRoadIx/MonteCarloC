function getmcsub(filename)

global Nfile J F S A E r z
global mua mus g n1 n2
global mcflag radius waist xs ys zs
global NR NZ dr dz Nphotons PLOTON PRINTON

% La funci�n con la que se salva el archivo es:
% SaveFile(	Nfile, J, F, S, A, E,
% 			mua, mus, g, n1, n2, 
% 		 	mcflag, radius, waist, xs, ys, zs, 
% 		 	NR, NZ, dr, dz, Nphotons);
% Se obtendr�n todos los par�metros del archivo salvado

%Abrir el archivo
fid = fopen(filename,'r');

%Obtener los primeros par�metros, leyendo l�nea por l�nea del archivo
s = fgetl(fid);
mua = sscanf(s,'%f');   %Coef. de absorci�n
s = fgetl(fid);
mus = sscanf(s,'%f');   %Coef. de esparcimiento
s = fgetl(fid);
g = sscanf(s,'%f');     %Coef. de anisotrop�a
s = fgetl(fid);
n1 = sscanf(s,'%f');    %�ndice de refracci�n del medio
s = fgetl(fid);
n2 = sscanf(s,'%f');    %�ndice de refracci�n externo

s = fgetl(fid);
mcflag = sscanf(s,'%f');    %Tipo de fuente utilizada
s = fgetl(fid);
radius = sscanf(s,'%f');    %Radio de la fuente
s = fgetl(fid);
waist = sscanf(s,'%f');     %"Cintura" de la fuente (Gaussiana)
s = fgetl(fid);
xs = sscanf(s,'%f');        %Posici�n en x
s = fgetl(fid);
ys = sscanf(s,'%f');        %Posici�n en x
s = fgetl(fid);
zs = sscanf(s,'%f');        %Posici�n en x

s = fgetl(fid);
NR = sscanf(s,'%f');        %# de recipientes radiales
s = fgetl(fid);
NZ = sscanf(s,'%f');        %# de recipientes en profundidad
s = fgetl(fid);
dr = sscanf(s,'%f');        %tama�o del paso del "recipiente" radial
s = fgetl(fid);
dz = sscanf(s,'%f');        %tama�o del paso del "recipiente" profunidad
s = fgetl(fid);
Nphotons = sscanf(s,'%f');  %# de fotones

s = fgetl(fid);
S = sscanf(s,'%f');         %Fracci�n de potencia por reflex. especular
s = fgetl(fid);
A = sscanf(s,'%f');         %Fracci�n de potencia absorbida
s = fgetl(fid); 
E = sscanf(s,'%f');         %Fracci�n de potencia perdida por escape

%Obtener la distancia radial
U = fscanf(fid,'%f',[1 NR+1]);
r = U(2:NR+1)';
%Obtener la densidad de flujo de escape
U = fscanf(fid,'%f',[1 NR+1]);
J = U(2:NR+1)';
%Leer fila por fila del mapa de fluencia
for i=1:NZ
    U = fscanf(fid,'%f', [1 NR+1]);
    %disp(sprintf('z = U(1) = %0.4e', U(1)))
    %Obtener la distancia en profundidad
    z(i,1) = U(1);
    %Obtener el ritmo de fluencia normalizado
    F(i,1:NR) = U(2:NR+1);
end

%Mostrar lo que se obtuvo
disp('-------------------------------------------------------------------')
disp('<<PAR�METROS �PTICOS>>')
disp(sprintf('mua = %0.4f cm^-1', mua))
disp(sprintf('mus = %0.4f cm^-1', mus))
disp(sprintf('g   = %0.4f ', g))
disp(sprintf('nt  = %0.4f ', n1))
disp(sprintf('ni  = %0.4f \n', n2))
disp('<<FUENTE>>')
switch mcflag
    case 0
        disp('** COLIMADA **')
        disp(sprintf('Radio  = %0.4f', radius))
        disp(sprintf('   zs  = %0.4f', zs))
    case 1
        disp('** GAUSSIANA **')
        disp(sprintf('          Radio  = %0.4f', radius))
        disp(sprintf('Distancia axial  = %0.4f', waist))
        disp(sprintf('Distancia focal  = %0.4f', zs))
    case 2
        disp('** PUNTUAL ISOTR�PICA **')
        disp(sprintf('Posici�n x  = %0.4f', xs))
        disp(sprintf('Posici�n y  = %0.4f', ys))        
        disp(sprintf('Posici�n z  = %0.4f', zs))
end
disp('<<RESULTADOS SIMULACI�N>>');
disp(sprintf('Num. Fotones   = %0.0f', Nphotons))
disp(sprintf('Especular      = %0.4f', S))
disp(sprintf('Absorci�n      = %0.4f', A))
disp(sprintf('Por Escape     = %0.4f', E))
disp(sprintf('Total          = %0.4f', S+A+E))
disp('-------------------------------------------------------------------')

%Cerrar el archivo
fclose(fid);