%% Load Data
clearvars
close all
load('accel_1.mat') % gets 'databuf' variable

%% ADXL configuration
% ( from datasheet )
% SENS     LSB PER G
%   2       256
%   4       128
%   8       64
%   16      32
lsb_per_g_map = [0 256 0 128 0 0 0 64 0 0 0 0 0 0 0 32];
adxl_sens = 2; % range in +- g
adxl_adc_bits = 10; % 10
Fs = 100;

adxl_lsbs_per_g = lsb_per_g_map(adxl_sens); 
%% Scale the variables to G's
databuf.x = databuf.x / adxl_lsbs_per_g;
databuf.y = databuf.y / adxl_lsbs_per_g;
databuf.z = databuf.z / adxl_lsbs_per_g;

%% Plots
t = 0:(1/Fs):length(databuf.x)/Fs;
t = t(1:length(databuf.x));

figure, hold on
grid on
plot(t, databuf.x, 'r', 'linewidth', 2)
plot(t, databuf.y, 'g', 'linewidth', 2)
plot(t, databuf.z, 'b', 'linewidth', 2)
xlabel('t (s)')
ylabel('Acceleration, G''s')
