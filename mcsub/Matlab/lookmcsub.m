clear all
close all
clc
home

global Nfile J F S A E r z
global mua mus g n1 n2
global mcflag radius waist xs ys zs
global NR NZ dr dz Nphotons
% Leer la respuesta a la fuente grabada en mcOUT0.dat
getmcsub('MCOUT84.dat');

%% Mostrar Fluencia
% Grafica Fzr(z,rr), un mapa en 2D del ritmo de fluencia con rr = -r:+r.
% Los "recipientes" desbordados en rr = 1 & NR, & en z = NZ.

Fzr = zeros(NZ,2*NR);
Fzr(:,NR+[1:NR]) = F;
Fzr(:,[NR:-1:1]) = F;
rr               = zeros(2*NR,1);
rr(NR+[1:NR])    = r;
rr(NR:-1:1)      = -r;

%Mostrar el mapa de ritmo de fluencia normalizado (log base 10)
figure(1);clf
imagesc(rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ),log10(Fzr( 1:(length(z)-1),2:(length(rr)-1) ))) 
colormap(hot)
colorbar
set(gca,'fontsize',12)
xlabel('r [cm]')
ylabel('z [cm]')
title('log_{10}(Ritmo de Fluencia relativo [W/cm^2 por W])')
figure,
vx=r(1:(length(r)-1));
vy=F(1:(length(r)-1),1);
plot(vx,vy,'k')
xlabel('r [cm]')
ylabel('\Phi_D/F_0')
title('Ritmo de Fluencia a Profundidad')

% %Ritmo de fluencia normalizado
% figure(2);clf
% imagesc(rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ),Fzr( 1:(length(z)-1),2:(length(rr)-1) )), colorbar
% set(gca,'fontsize',12)
% xlabel('r [cm]')
% ylabel('z [cm]')
% title('(Ritmo de Fluencia relativo [W/cm^2 por W])')
% 
% %Ritmo de fluencia normalizado (ln )
% figure(3);clf
% imagesc(rr( 2:(length(rr)-1) ),z( 1:(length(z)-1) ),log(Fzr( 1:(length(z)-1),2:(length(rr)-1) ) )), colorbar
% set(gca,'fontsize',12)
% xlabel('r [cm]')
% ylabel('z [cm]')
% title('ln(Ritmo de Fluencia relativo [W/cm^2 por W])')