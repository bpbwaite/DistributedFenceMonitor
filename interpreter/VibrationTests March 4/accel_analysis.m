%/*
%  FILE: accel_analysis.m
%  VERSION: 1.0.2
%  TEST DATE: 4 March 2023
%  DATE: 6 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Debiases and plot acceleration in power domain
%*/

%% Load Data
cd(fileparts(matlab.desktop.editor.getActiveFilename)); % Set the working dir to dir of this file
test_name = 'accel_3.mat';
close all
load(test_name) % gets 'databuf' variable

% test circumstances: 6 mph winds, 10ft wide panels, mounted center
% test 3: lucas hops fence like pro
% test 4: jack holding it still (inside job)
% test 5: shake
% test 6: lucas runs into fence
% test 7: the rhino test (insane run and slam)
% test 8: shaking 2 panels away
% test 9: 5 panels away, simulating attempt to climb
%%
% ADXL configuration
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
lower_raw_limit = -1*2^adxl_adc_bits / 2;
upper_raw_limit = -1*lower_raw_limit-1;

g = 9.81;


%% Plots
t = 0:(1/Fs):length(databuf.x)/Fs;
t = t(1:length(databuf.x));

figure(1), hold on
grid on
plot(t, databuf.z, 'r', 'linewidth', 2);
% Plot 'error'
%plot(t, databuf.z * 1.01, 'k-.', 'linewidth', 1);
%plot(t, databuf.z * 0.99, 'k-.', 'linewidth', 1);

xlabel('t (s)')
ylabel('Raw ADC Signal')
xlim([0 t(end)])
ylim([lower_raw_limit upper_raw_limit])
xticks(0:1:floor(t(end)))
title(['Test "', test_name, '" Datapoints'],'Interpreter','none')

% scan through increments. if the greatest difference is less
% than a threshold difference, in m/s2,
% then average the increment and set that as the quiescent level
% (will be run on embedded hardware in the future)

figure(2), hold on

% Scale the variable to m/s^2
z = g * databuf.z / adxl_lsbs_per_g;

inc_size_sec = 1;

thresh = 50; % modify this value (lsbs)

% read the below value for software
thresh_in_62_5_mg_incs = ceil((thresh / adxl_lsbs_per_g) / (62.5/1000));
thresh = thresh * g /  adxl_lsbs_per_g;


found_quiet = 0;
for dx = 1:ceil(inc_size_sec*Fs):floor(t(end)*Fs)
    dx_end = min(dx + inc_size_sec*Fs, length(z));
    d_examine = z(dx:dx_end);
    if abs(diff([max(d_examine), min(d_examine)])) < thresh
        found_quiet = 1;
        break
    end
end

m = mean(z(dx:dx_end));

% expand the endpoint (for visualization purposes):
while dx_end < length(z) && abs(diff([z(dx_end), m])) < thresh/2
    dx_end = dx_end + 1;
end
if found_quiet
    m = mean(z(dx:dx_end));
    z = z - m;
end

z_pow = z.^2;

grid on
plot(t, z_pow, 'r', 'linewidth', 2);
xlabel('t (s)')
ylabel('Signal Power (m^2/s^4)')
xlim([0 t(end)])
xticks(0:1:floor(t(end)))
title("Power Analysis")

if found_quiet
    plot([dx/Fs dx_end/Fs], [0 0], 'g', 'linewidth',3)
    
    m = m/g*adxl_lsbs_per_g;
    figure(1), hold on
    plot(t, databuf.z - m, 'b', 'linewidth', 1);
    legend('Raw', 'After Subtract Offset')
end
