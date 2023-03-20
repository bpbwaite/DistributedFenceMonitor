%/*
%  FILE: Live Viewer for accelerometer
%  VERSION: 1.0.0
%  DATE: 15 March 2023
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
%% ADXL configuration
% ( from datasheet )
% SENS     LSB PER G
%   2       256
%   4       128
%   8       64
%   16      32
lsb_per_g_xy = 265; % you read that correctly. assumes vcc = 3.3v
lsb_per_g_z = 256;
g = 9.81;
adxl_sens = 2; % range in +- g
adxl_adc_bits = 10; % 10
BW = 50;
Fs = 2*BW;
%% Connection
spla = serialportlist';
spla = spla(1);
s = serialport(spla, 115200);
s.Timeout = timeout;
    
disp("Setup complete!")
disp("Receiving...")

%% Ingest
databuf.x = []; 
databuf.y = []; 
databuf.z = [];
n = 1;

t_window = 10; % seconds of data to show
%keep_back = Fs * t_window;

figure(1), hold on
grid on
shg
xlabel('t (s)')
ylabel('Acceration (g)')
h1 = plot(0,0,'r');
h2 = plot(0,0,'g');
h3 = plot(0,0,'b');
ylim([-2.05 2.05])
legend('X','Y','Z')

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

        if ~mod(n, 800) % determines show freq
            xlower = max(0, n/Fs - t_window);
            xupper = max(t_window, n/Fs);
            X = (1:length(databuf.x))./Fs;
            
            figure(1)
            set(h1, 'XData', X, 'YData', databuf.x / lsb_per_g_xy);
            set(h2, 'XData', X, 'YData', databuf.y / lsb_per_g_xy);
            set(h3, 'XData', X, 'YData', databuf.z / lsb_per_g_z);
            drawnow
            xlim([xlower, xupper])
        end
        % terminator
        newl = read(s, 2, "uint8");
        
        n = n+1;
       
        
end
%% CTRL-C to break from data collection

