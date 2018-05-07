% lookmcsubfluor.m
% Grafica los resultados de las simulaciones Monte Carlo
%
% Paso 1: Graficar la salida de exitación en mcOUT101.dat
%           Fig 1: Función de punto de propagación de fuga,
%           J(r) [1/cm^2], para excitación and emisión.
%           Fig 2: Función de punto de propagación de transporte de
%           exitación Tex [1/cm^2].
% Paso 2: Graficar la fluorescencia debido a la uniformidad del fluoroforo
%           Fig 3: función de punto de propagación de las emisiones del
%           transporte, Tem [1/cm^2], para fluoroforo homogeneo de fondo
%           Fig 4: razón de funciones de escape Jem(r)/Jex(r).
%           Fig 5: J
% Paso 3: Graficar la fluorescencia de la fluorescencia heterogénea
clear all
close all
clc
home

global Nfile J F S A E r z
global mua mus g n1 n2
global mcflag radius waist xs ys zs
global NR NZ dr dz Nphotons

P = 1; % W

% Número de simulación
numerele=15;

p = pwd;
commandwindow

% Abrir el archivo para obtener los valores de la simulación
filename = strcat('mcOUT',int2str(numerele),'01.dat'); %'mcOUTx01.dat';
getmcsub(filename);
Jex = J;
Fex = F;
u = (1:NR-1);

% Abrir el archivo para obtener los valores de la simulación
filename = strcat('mcOUT',int2str(numerele),'02.dat'); %'mcOUTx02.dat';
getmcsub(filename);
Jem = J;
Fem = F;

% % Abrir el archivo para obtener los valores de la simulación
% filename = 'Joel.dat';
% getmcsub(filename);
% Jex = J;
% Fex = F;
% u = (1:NR-1);

%
figure(1);clf
semilogy(r(u), P*Jex(u),'b-','linewidth',2)
hold on
plot(r(u), P*Jem(u),'r-','linewidth',2)
plot(-r(u), P*Jex(u),'b-','linewidth',2)
plot(-r(u), P*Jem(u),'r-','linewidth',2)
set(gca,'fontsize',14)
xlabel('r [cm]'), ylabel('J(r) [1/cm^-^2]')
title('Flujos de escape E_x & E_m')
legend('Excitación','Emisión')

%
figure(2);clf
y = Jem./Jex;
plot(r(u), y(u),'r-','linewidth',2)
hold on
semilogy(-r(u), y(u),'r-','linewidth',2)
set(gca,'fontsize',14)
xlabel('r [cm]'), ylabel('J_e_m/J_e_x [-]')
title('Proporción de escape E_x/E_m')
axis([-max(r) max(r) 0 max(y(u))*1.2])

%
figure(3);clf
FF = zeros(NZ,2*NR);
FF(:,NR+1:2*NR) = Fex;
FF(:,1:NR) = Fex(:,NR:-1:1);
rr = zeros(2*NR,1);
rr(1:NR) = -r(NR:-1:1);
rr(NR+1:2*NR) = r;
disp(sprintf('total J = %0.4f', sum(Jex.*r*2*pi*dr)))
%imagesc(rr, z, log10(P*FF) )
imagesc( rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ), log10(P*FF( 1:(length(z)-1),2:(length(rr)-1) )) )
colormap(hot)
colorbar
set(gca,'fontsize',14)
xlabel('r [cm]'), ylabel('z [cm]')
title('Excitación \newline log_1_0(T_e_x [1/cm^2]')
axis equal image

%
figure(4);clf
FF = zeros(NZ,2*NR);
FF(:,NR+1:2*NR) = Fem;
FF(:,1:NR) = Fem(:,NR:-1:1);
rr = zeros(2*NR,1);
rr(1:NR) = -r(NR:-1:1);
rr(NR+1:2*NR) = r;
disp(sprintf('total J = %0.4f', sum(Jem.*r*2*pi*dr)))
%imagesc(rr, z, log10(P*FF) )
imagesc( rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ), log10(P*FF( 1:(length(z)-1),2:(length(rr)-1) )) )
colormap(jet)
colorbar
set(gca,'fontsize',14)
xlabel('r [cm]')
ylabel('z [cm]')
title('Fluorescencia de fondo \newline log_1_0(T_e_m [1/cm^2]')
axis equal image

%
figure(5);clf
u = (1:NR-1); % omitir el "recipiente" desbordado J(NR)
% intervalo x positivo de J
filename = strcat('mcOUT',int2str(numerele),'03.dat'); %'mcOUTx03.dat'; 
getmcsub(filename);
FF(:,NR+1:2*NR) = F(:,1:NR);
semilogy(r(u), P*J(u),'b-','linewidth',2)
hold on

% intervalo x negativo de J
filename = strcat('mcOUT',int2str(numerele),'04.dat'); %'mcOUTx04.dat';  
getmcsub(filename);
FF(:,1:NR) = F(:,NR:-1:1);
plot(-r(u), P*J(u),'c--','linewidth',2)
set(gca,'fontsize',14)
xlabel('r [cm]'), ylabel('J(r) [1/cm^-^2]')
title('Fluoroforo local fuera del centro')

%
figure(6);clf
%imagesc(rr,z,log10(FF))
imagesc( rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ), log10(FF( 1:(length(z)-1),2:(length(rr)-1) )) )
colorbar
colormap(jet)
set(gca,'fontsize',14)
xlabel('r [cm]'), ylabel('z [cm]')
title('Fluoroforo local fuera del centro \newline log_1_0(T_e_xT_e_m [1/cm^4]')
axis equal image

ifigs=1:6;