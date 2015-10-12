g = 9.81;
m = 2.99e2/g;

Xudot = -9.30e-1; % kg
Yvdot = -3.55e1;  % kg
Yrdot =  1.93e0;  % kg.m/rad
Zwdot = -3.55e1;  % kg
Zqdot = -1.93e0;  % kg.m/rad
Kpdot = -7.04e-2; % kg.m²/rad
Mwdot = -1.93e0;  % kg.m
Mqdot = -4.88e0;  % kg.m²/rad
Nvdot =  1.93e0;  % kg.m
Nrdot = -4.88e0;  % kg.m²/rad

xg = 0;
yg = 0;
zg = 1.96e-2;

Ixx = 1.77e-1;  % kg.m²
Iyy = 3.45e0;   % kg.m²
Izz = 3.45e0;   % kg.m²

% formation de la matrice

M = [m-Xudot 0          0           0         m*zg        -m*yg
     0       m-Yvdot    0           -m*zg     0           m*xg-Yrdot
     0       0          m-Zwdot     m*yg      -m*xg-Zqdot 0
     0       -m*zg      m*yg        Ixx-Kpdot 0           0
     m*zg    0          -m*xg-Mwdot 0         Iyy-Mqdot   0
     -m*yg   m*xg-Nvdot 0           0         0           Izz-Nrdot];
 Minv = inv(M)

