%/*
%  FILE: accel_threshFinder.m
%  VERSION: 1.0.0
%  DATE: 16 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Reference tool for finding an appropriate threshold level
%  for a given signal.
%*/

%% Load Data & Setup
cd(fileparts(matlab.desktop.editor.getActiveFilename)); % Set the working dir to dir of this file
test_name = 'accel_3.mat';
close all
load(test_name)


% ADXL configuration

adxl_sens = 2; % range in +- g
adxl_adc_bits = 10; % 10
adxl_lsbs_per_g = 256; % for Z axis in 2g mode
lower_raw_limit = -1*2^adxl_adc_bits / 2;
upper_raw_limit = -1*lower_raw_limit - 1;

Fs = 100;
g = 9.81;

%% Plot Signal in m/s2
t = 0:(1/Fs):length(databuf.x)/Fs;
t = t(1:length(databuf.x));

% Scale the variable to m/s^2
z = g * databuf.z / adxl_lsbs_per_g;

% Find the increment with the smallest range
inc_size_sec = 1; % should be >  ~0.3 sec, < length of data
inc_samples = inc_size_sec * Fs;
list_of_ranges = zeros(1, floor(t(end) / inc_size_sec));

for n = 1:length(list_of_ranges)
    dx_start = floor((n-1)*inc_samples) + 1;
    dx_end = min(dx_start + inc_samples, length(z));
    list_of_ranges(n) = abs(diff([max(z(dx_start:dx_end)), min(z(dx_start:dx_end))]));
end

[~, idx] = min(list_of_ranges);
dx_start = floor(idx*Fs);
dx_end = min(dx_start + inc_samples, length(z));
bias = mean(z(dx_start:dx_end));

figure, 
grid off
plot(t, z - bias, 'b');

xlabel('t (s)')
ylabel('DC - Decoupled Accelerometer Signal (m/s^{2})')
xlim([0 t(end)])
ylim([-1*adxl_sens*g adxl_sens*g])
xticks(0:1:ceil(t(end)))
yticks(unique(sort([-2*g:(62.5*g/1000):2*g ])))
title('Activity Thresholds and Reference Signal', 'Interpreter', 'none')
hold on

%% Plot Thresholds
for act_val = 1:1: min((255/(16/2))+1, 255)
    act_g = act_val * (62.5*g/1000);
    plot([0 t(end)], [act_g act_g], '--', 'Color', [.50 .50 .50])
    plot([0 t(end)], [-1*act_g -1*act_g], '--', 'Color', [.50 .50 .50])
    text(t(end) + .25, act_g, ['T_A = ', num2str(act_val)])
    text(t(end) + .25, -1*act_g, ['T_A = ', num2str(act_val)])
end


