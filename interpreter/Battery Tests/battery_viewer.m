%/*
%  FILE: .m
%  VERSION: 1.0.0
%  TEST DATE: 10 March 2023
%  DATE: 11 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: 
%*/
% TESTS: LOG310_1: at TX 15 every 10 seconds for 46 ms, small packet.
% Notes: Started at 4.13V, ended at 4.035.
% Confounds: extra time to write to SD card, ADC power

% test LOG311_X: at TX 3 every 15 seconds for 46 ms, small packet.
% Notes: Started at 4.13V, 7:37 pm 3/11/2023
%% Setup
clear
close all
clc
cd(fileparts(matlab.desktop.editor.getActiveFilename));

format short
format compact

%% Constants
voltage_hundred = 4.2;
voltage_zero = 3.2;
R_vdiv_top = 330000; % voltage divider upper resistor
R_vdiv_bottom = 680000; % voltage divider lower resistor
ADC_Vref = 3.3;
ADC_depth = 10; % see about turning up to 12 in the code

%% Import Data
opts = delimitedTextImportOptions("NumVariables", 4);
opts.DataLines = [4, Inf];
opts.Delimiter = ",";
opts.VariableNames = ["n_unused", "epoch", "reading", "flag_unused"];
opts.VariableTypes = ["double", "double", "double", "double"];
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
SD_data_1 = table2array(readtable("LOG310_1.TXT", opts)); 

%% Conversion factors
t_start = SD_data_1(1, 2);
t = (SD_data_1(:, 2) - t_start) ./ 3600; % t in hours

v_at_adc = ADC_Vref * SD_data_1(:, 3) / (2^ADC_depth - 1);
v_at_bat = v_at_adc * (R_vdiv_top + R_vdiv_bottom) / (R_vdiv_bottom);
% cap to 100%
for k=1:length(t)
   v_at_bat(k) = min(voltage_hundred, v_at_bat(k));
end
%% Plots
bat_percentage = 100 * (v_at_bat - voltage_zero);

figure,
hold on 
grid on
plot(t, bat_percentage, 'b')
xlabel('t (h)')
ylabel('Battery Charge (%)')
title('Long Term Power Test')

tim = 0:250;
plot(tim, -0.357*tim + 89.55, 'r-.', 'LineWidth', 2) % line of best fit

