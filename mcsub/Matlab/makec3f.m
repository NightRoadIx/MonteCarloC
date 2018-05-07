function c2 = makec3f

% makecolormap.m

% ----> c3(64,3)
%  rojo
%  amarillo
%  negro
%  cyan
%  azul
c2 = zeros(64,3);

for ii=1:10   % negro a gris
	c2(ii,1) = 0.1 + 0.4*ii/10;     % rojo
	c2(ii,2) = 0.1 + 0.4*ii/10;     % verde
	c2(ii,3) = 0.1 + 0.4*ii/10;     % azul
end
for ii=11:20   % grey to blue
	c2(ii,1) = 0.5*(1-(ii-10)/10);  % rojo
	c2(ii,2) = 0.5*(1-(ii-10)/10);  % verde
	c2(ii,3) = 0.5 + 0.5*(ii-10)/10;% azul
end
a = 21; b = 32;
for ii=a:b % azul a cyan
	c2(ii,1) = 0;   
	c2(ii,2) = (ii-a+1)/(b-a+1);   
	c2(ii,3) = 1; 
end

% a = 29; b = 32;
% for ii=a:b % cyan
% 	c2(ii,1) = 0;   
% 	c2(ii,2) = 1;   
% 	c2(ii,3) = 1; 
% end
% a = 33; b = 37;
% for ii=a:b % cyan to green
% 	c2(ii,1) = 0;   
% 	c2(ii,2) = 1;   
% 	c2(ii,3) = 1 - (ii-a+1)/(b-a+1); 
% end

a = 33; b = 45;
for ii=a:b % verde a amarillo
	c2(ii,1) = (ii-a+1)/(b-a+1);   
	c2(ii,2) = 1;  
	c2(ii,3) = 0; 
end
a = 46; b = 53;
for ii=a:b % amarillo
	c2(ii,1) = 1;   
	c2(ii,2) = 1;   
	c2(ii,3) = 0; 
end
a = 54; b = 64;
for ii=a:b % amarillo a rojo
	c2(ii,1) = 1;   
	c2(ii,2) = 1 - (ii-a+1)/(b-a+1);  
	c2(ii,3) = 0; 
end