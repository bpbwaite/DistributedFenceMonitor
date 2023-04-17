%/*
%  FILE: accel_integrate.m
%  VERSION: 1.0.1
%  DATE: 16 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Testing the accel drift via integration
%*/


%% Load Data
cd(fileparts(matlab.desktop.editor.getActiveFilename)); % Set the working dir to dir of this file
test_name = 'accel_8.mat';
close all
load(test_name) % gets 'databuf' variable

%%
% ADXL configuration

adxl_sens = 2; % range in +- g
adxl_adc_bits = 10; % 10
adxl_lsbs_per_g = 265;

Fs = 100;
g = 9.802;


%% Plot Signal in m/s2
t = 0:(1/Fs):length(databuf.x)/Fs;
t = t(1:length(databuf.x));

% Scale the variable to m/s^2
z = g * databuf.z / adxl_lsbs_per_g;


acceleration = z - mean(z); % in m/s2
velocity = detrend(cumtrapz(acceleration ./ Fs)); % in m/s
position = cumtrapz(velocity ./ Fs); % in cm

position = position  * 100; % in cm

figure, 
hold on
grid on

plot(t, acceleration, 'r');
plot(t, velocity, 'b', 'linewidth', 2);
plot(t, position, 'k');

xlabel('t (s)')
ylabel('Magnitude')
xlim([0 t(end)])
xticks(0:1:ceil(t(end)))
title('Acceleration-Velocity-Position Graph', 'Interpreter', 'none')
legend('A (m/s^{2})', 'V (m/s)', 'S (cm)')
