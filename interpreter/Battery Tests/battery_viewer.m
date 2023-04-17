%/*
%  FILE: battery_viewer.m
%  VERSION: 1.0.6
%  TEST DATE: 10 March 2023
%  DATE: 22 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Graph the discharge curve of a LiPo battery
%*/
% TESTS: 
% Confounds: extra time to write to SD card, ADC power

% LOG310_1: 2 day test. at TX 15 every 10 seconds for 46 ms.
% Notes: Started at 4.13V, ended at 4.035. 

% LOG312_4: week-long test. at TX 3 every 15 seconds for 46 ms.
% Notes: Started at 4.13V, 7:37 pm 3/11/2023. Awake for 73ms.

%% Setup
clear
close all
clc
cd(fileparts(matlab.desktop.editor.getActiveFilename));

format short
format compact

%% Constants
voltage_hundred = 4.15; % typically around 4.2
voltage_zero = 3.3; 
% PMIC demands the voltage ends at 3.2V
% battery can go as low as 2.75V. must not discharge faster than 2 Hours

R_vdiv_top = 330000; % voltage divider upper resistor
R_vdiv_bottom = 680000; % voltage divider lower resistor
ADC_Vref = 3.3;
ADC_depth = 10; % Code can set to 10 or 12

%% Import Data
opts = delimitedTextImportOptions("NumVariables", 4);
opts.DataLines = [4, Inf];
opts.Delimiter = ",";
opts.VariableNames = ["n_unused", "epoch", "reading", "flag_unused"];
opts.VariableTypes = ["double", "double", "double", "double"];
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
SD_data_1 = table2array(readtable("LOG312_4.TXT", opts)); 

%% Conversion factors
t_start = SD_data_1(1, 2);
t = (SD_data_1(:, 2) - t_start) ;
t = t ./ 3600 ./ 24; % t in hours -> t in days

v_at_adc = ADC_Vref * SD_data_1(:, 3) / (2^ADC_depth - 1);
v_at_bat = min(voltage_hundred, ...
    v_at_adc * (R_vdiv_top + R_vdiv_bottom) / (R_vdiv_bottom));

bat_percentage = 100*...
    (v_at_bat-voltage_zero) / (voltage_hundred-voltage_zero);

% take sample averages
n = 7;
Y = conv(bat_percentage, ones(1, n), 'valid');
smoothened_bat = Y(1:n:end) ./ n;
t_short = t(1:n:end);
t_short = t_short(1:length(smoothened_bat));

%% Plots

figure,
subplot(2,1,1)
hold on
grid on

plot(t_short, smoothened_bat, 'b')
xlabel('t (days)')
xticks(0:0.5:t_short(end))
ylabel('Battery Charge (%)')
ylim([0 100])
yticks(0:5:100)
title('Long Term Power Test')

% line of best fit
%plot(0:240, -0.357*(0:240) + 100, 'r-.', 'LineWidth', 2) 

subplot(2,1,2)
hold on
grid on

plot(t, v_at_bat, 'r')
xlabel('t (days)')
xticks(0:0.5:t(end))
ylabel('Voltage')
ylim([3.0 4.3])
yticks(0:0.1:5)

