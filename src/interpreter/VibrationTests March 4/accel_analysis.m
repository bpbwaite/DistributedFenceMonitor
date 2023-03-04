%% Load Data

close all
load('accel_9.mat') % gets 'databuf' variable

% test circumstances: 6 mph winds
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
g = 9.81;
% Scale the variables to m/s^2
databuf.x = g* databuf.x / adxl_lsbs_per_g;
databuf.y = g* databuf.y / adxl_lsbs_per_g;
databuf.z = g* databuf.z / adxl_lsbs_per_g;

%% Plots
t = 0:(1/Fs):length(databuf.x)/Fs;
t = t(1:length(databuf.x));

z = databuf.z;
% scan whole 1-second increments. if the greatest power difference is less
% than ~6 (where acceleration in m/s2 was squared)
% then average the increment and set it as the quiescent level
% will be run on embedded hardware in the future

thresh = 6.0;
found_quiet = 0;
for dx = 1:1*Fs:t(end)
    dx_end = dx + 1*Fs;
    d_examine = z(dx:dx_end);
    if abs(diff([max(d_examine), min(d_examine)])) < thresh
        found_quiet = 1;
        break
    end
end
if found_quiet
    z = z - mean(z(dx:dx_end));
end
z_pow = z.^2;

figure, hold on
grid on
plot(t, z_pow, 'r', 'linewidth', 2);
xlabel('t (s)')
ylabel('Acceleratory Power')
xlim([0 t(end)])
ylim([0 600])
%xticks(0:1:floor(t(end)))
%yticks(0:50:600)