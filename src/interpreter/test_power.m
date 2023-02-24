%/*
%  FILE: test_power.m
%  VERSION: 1.1.0
%  TEST DATE: 17 February 2023
%  DATE: 24 February 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Current draw tests using external power supply
%*/


%% Setup
clear
close all
clc
cd(fileparts(matlab.desktop.editor.getActiveFilename));

format short
format compact

%% Constants
mA_per_mV = 10;                 % TCP2020 sensitivity
scope_sensitivity = 1/10;       % scope gain setting
Fs = 100;                       % scope sampling frequency
input_voltage = 6.00;           % psu input
num_tests = 4;


%% Import Data
opts = delimitedTextImportOptions("NumVariables", 3);
opts.DataLines = [3, Inf];
opts.Delimiter = ",";
opts.VariableNames = ["second", "Volt", "Volt_1"];
opts.VariableTypes = ["double", "double", "double"];
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
scope1 = readtable("scope_0.csv", opts);
scope1 = table2array(scope1);
scope2 = readtable("scope_1.csv", opts);
scope2 = table2array(scope2);
scope3 = readtable("scope_2.csv", opts);
scope3 = table2array(scope3);
scope4 = readtable("scope_3.csv", opts);
scope4 = table2array(scope4);


%% Current Plots
t = scope1(:,1) - 5.96;

% filter trigger signal
probe_trig = scope1(:,2);
probe_trig(probe_trig > 2.5) = 5;
probe_trig(probe_trig <= 2.5) = 0;

probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope1(:,3);
figure,
plot(t, probe_cur, t, probe_trig)
grid on
axis([0 11 -10 150])
title('Test Result #1')
xlabel('t (s)')
ylabel('Current (mA)')

t = scope2(:,1) - 4.66;

% filter trigger signal
probe_trig = scope2(:,2);
probe_trig(probe_trig > 2.5) = 5;
probe_trig(probe_trig <= 2.5) = 0;

probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope2(:,3);
figure,
plot(t, probe_cur, t, probe_trig)
grid on
axis([0 11 -10 150])
title('Test Result #2')
xlabel('t (s)')
ylabel('Current (mA)')

t = scope3(:,1) - 5.98;

% filter trigger signal
probe_trig = scope3(:,2);
probe_trig(probe_trig > 2.5) = 5;
probe_trig(probe_trig <= 2.5) = 0;

probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope3(:,3);
figure,
plot(t, probe_cur, t, probe_trig)
grid on
axis([0 11 -10 150])
title('Test Result #3')
xlabel('t (s)')
ylabel('Current (mA)')

t = scope4(:,1) - 2.36;

% filter trigger signal
probe_trig = scope4(:,2);
probe_trig(probe_trig > 2.5) = 5;
probe_trig(probe_trig <= 2.5) = 0;

probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope4(:,3);
figure,
plot(t, probe_cur, t, probe_trig)
grid on
axis([0 11 -10 150])
title('Test Result #4')
xlabel('t (s)')
ylabel('Current (mA)')


%% Power Plots
t1 = scope1(:,1) - 5.96;
probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope1(:,3);
probe_power = probe_cur * input_voltage;
figure,
plot(t1, probe_power, 'r')
grid on
axis([0 11 -10 1000])
title('Power Draw Result #1')
xlabel('t (s)')
ylabel('Power (mW)')

t2 = scope2(:,1) - 4.66;
probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope2(:,3);
probe_power = probe_cur * input_voltage;
figure,
plot(t2, probe_power, 'r')
grid on
axis([0 11 -10 1000])
title('Power Draw Result #2')
xlabel('t (s)')
ylabel('Power (mW)')

t3 = scope3(:,1) - 5.98;
probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope3(:,3);
probe_power = probe_cur * input_voltage;
figure,
plot(t3, probe_power, 'r')
grid on
axis([0 11 -10 1000])
title('Power Draw Result #3')
xlabel('t (s)')
ylabel('Power (mW)')

t4 = scope4(:,1) - 2.36;
probe_cur = 1000 * scope_sensitivity * mA_per_mV * scope4(:,3);
probe_power = probe_cur * input_voltage;
figure,
plot(t4, probe_power, 'r')
grid on
axis([0 11 -10 1000])
title('Power Draw Result #4')
xlabel('t (s)')
ylabel('Power (mW)')


%% Analysis
% differences in average current are more useful than absolute readings.
% find the average idle power use and peak draw at different transmission
% strengths.

% data channels in regions of interest
startings = [532, 408, 523, 162];
lengths = 915;
endings = startings + lengths - 1;

probetrigs = [scope1(:, 2) scope2(:, 2) scope3(:, 2) scope4(:, 2)];
probecurs = [scope1(:, 3) scope2(:, 3) scope3(:, 3) scope4(:, 3)];
all_probetrig = [];
all_probecur = [];

for n=1:num_tests
    all_probetrig(1:lengths, n) = probetrigs(startings(n):endings(n), n);
    all_probecur(1:lengths, n) = ...
        1000 * scope_sensitivity * mA_per_mV * probecurs(startings(n):endings(n), n);
end

% find the resting current draw
r_avg = 0;
for n = 1:num_tests
    probe_trig = all_probetrig(:, n);
    probe_trig(probe_trig > 2.5) = 5;
    probe_trig(probe_trig <= 2.5) = 0;
    
    probe_cur = all_probecur(:, n);
    
    probe_power = probe_cur * input_voltage;
    
    count = 0;
    accumulator = 0;
    for k=2:length(all_probetrig)
        if(probe_trig(k-1) == 5 && probe_trig(k) == 0)
            for g = k:length(all_probetrig)
                if(probe_trig(g) == 0)
                    accumulator = accumulator + probe_cur(g);
                    count = count + 1;
                end
            end
            break
        end
        
    end
    fprintf('Resting Current Draw #%d: %.2f mA\n', n, accumulator / count)
    r_avg = r_avg + accumulator/count;
end

fprintf('Average Resting Current Draw: %.2f mA\n', r_avg / num_tests)


% data channels in regions of interest
startings = [426, 305, 421, 53];
lengths = 1110;
endings = startings + lengths - 1;

probetrigs = [scope1(:, 2) scope2(:, 2) scope3(:, 2) scope4(:, 2)];
probecurs = [scope1(:, 3) scope2(:, 3) scope3(:, 3) scope4(:, 3)];
all_probetrig = [];
all_probecur = [];

for n=1:num_tests
    all_probetrig(1:lengths, n) = probetrigs(startings(n):endings(n), n);
    all_probecur(1:lengths, n) = ...
        1000 * scope_sensitivity * mA_per_mV * probecurs(startings(n):endings(n), n);
end

% find tx draw
txpows_avg = [];

txpows_std = zeros(num_tests, 20, 2); 
% will capture transmission draw for computing standard deviation

for n = 1:num_tests
    probe_trig = all_probetrig(:, n);
    probe_trig(probe_trig > 2.5) = 5;
    probe_trig(probe_trig <= 2.5) = 0;
    
    probe_cur = all_probecur(:, n);
    
    probe_power = probe_cur * input_voltage;
    
    txpow = 2;
    k = 1;
    while k < length(probe_trig) && txpow <=20
        while k < length(probe_trig) && probe_trig(k) == 0
            k = k + 1;
        end
        power_accumulator = 0;
        nums_accumulated = 0;
        while k < length(probe_trig) && probe_trig(k) == 5
            power_accumulator = power_accumulator + probe_cur(k);
            nums_accumulated = nums_accumulated + 1;
            if probe_trig(k + 1) > 0
                % prevents overrun by single-sample desync
                txpows_std(n, txpow, nums_accumulated) = probe_cur(k+1);
            end
            k = k + 1;       
        end
        meanpow = power_accumulator/nums_accumulated;
        txpow_avg(txpow - 1, n) = meanpow;
        
        if txpow == 18
            txpow = 20;
        end
        fprintf('B#%d TX%d: %.2fmA\n', n, txpow, meanpow)
        txpow = txpow + 1;
    end
end

for t = 1:17
    x = t + 1;
    if x == 18
        x = 20;
    end
    fprintf('While in TX%d Average Draw: %.2f mA\n', x, mean(txpow_avg(t, :)))
end


%% Current & Power Averages
figure, hold on
for t = 1:17
    x = t + 1;
    if x == 18
        x = 20;
    end
    i_inc = mean(txpow_avg(t, :)) - r_avg/num_tests;
    fprintf('TX%d Increase: %.1f mA (%.1f mW)\n', x, i_inc, i_inc * input_voltage)
    stem(x, i_inc, 'b', 'LineWidth', 2)
end
grid on
title('Increase in Current Draw at different Transmission Powers')
subtitle(['Murata LoRa Module. V_{in} = ',num2str(input_voltage, 1),'V'])
xlabel('Transmit Power (TX)')
ylabel('Current Draw (mA)')
xlim([1 21])
xticks(2:20)
yticks(0:5:100)

figure, hold on
for t = 1:17
    x = t + 1;
    if x == 18
        x = 20;
    end
    p_avg = (mean(txpow_avg(t, :)) - r_avg/num_tests) * input_voltage;
    stem(x, p_avg, 'r-', 'LineWidth', 2)
end
grid on
title('Increase in Power Draw at different Transmission Powers')
subtitle('Murata LoRa Module')
xlabel('Transmit Power (TX)')
ylabel('Power Draw (mW)')
xlim([1 21])
xticks(2:20)
yticks(0:20:520)


% Per Zapata Request
% plot with quiescent current included

figure, hold on
for t = 1:17
    x = t + 1;
    if x == 18
        x = 20;
    end
    
    stem(x, mean(txpow_avg(t, :)), 'b', 'LineWidth', 2)
    h=stem(x, r_avg/num_tests, 'r', 'LineWidth', 2);
    set(h, 'Marker', 'none');
end
grid on
title('Current Draw at different Transmission Powers')
subtitle(['Murata LoRa Module. V_{in} = ',num2str(input_voltage, 1),'V'])
xlabel('Transmit Power (TX)')
ylabel('Current Draw (mA)')
xlim([1 21])
ylim([0 150])
xticks(2:20)
yticks(0:10:150)
l1 = legend( 'Mean Transmitting Current', 'Quiescent Draw');
set(l1,...
    'Position',[0.15 0.76 0.35 0.0]);


%% Power Draw with Errorbar

figure, hold on
for t = 1:17
    x = t + 1;
    if x == 18
        x = 20;
    end
    draws = [reshape(txpows_std(1, t+1, :),1,[])...
        reshape(txpows_std(2, t+1, :),1,[])...
        reshape(txpows_std(3, t+1, :),1,[])...
        reshape(txpows_std(4, t+1, :),1,[])];
    p_avg = (mean(txpow_avg(t, :)) - r_avg/num_tests) * input_voltage ;
    bar(x, p_avg, 'r')
    errorbar(x, p_avg, input_voltage*std(draws)*ones(size(x)), 'k');
end
grid on
title('Power Required by Transmission Levels')
subtitle('Murata LoRa Module')
xlabel('Transmit Power (TX)')
ylabel('Power Draw (mW)')
xlim([1 21])
xticks(2:20)
yticks(0:50:1000)