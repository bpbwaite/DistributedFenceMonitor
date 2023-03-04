%/*
%  FILE:
%  VERSION: 0.0.1
%  DATE: 
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: 
%*/

%% Setup
close all
clear
clc
format compact

%% Constants

baud = 115200; % bps
timeout = 10; % s
gmtoffset = -25200; % s

%% Connection
spla = "COM15";
s = serialport(spla, 115200);
s.Timeout = timeout;
    
disp("Setup complete!")
disp("Receiving.")

%% Ingest
databuf.x = []; 
databuf.y = []; 
databuf.z = [];
n = 1;
while 1
    while 1
       c = read(s, 1, "uint8");
       if c == 35 % start of a packet
           break
       end
    end
        databuf.x(n) = read(s, 1, "int32");
        databuf.y(n) = read(s, 1, "int32");
        databuf.z(n) = read(s, 1, "int32");
        
        % terminator
        newl = read(s, 2, "uint8");
        
        n = n+1;
       
        
end
%% CTRL-C to break from data collection
save('accel_N.mat', 'databuf');

