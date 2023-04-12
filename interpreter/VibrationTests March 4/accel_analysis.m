%/*
%  FILE: accel_analysis.m
%  VERSION: 1.0.1
%  TEST DATE: 4 March 2023
%  DATE: 6 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION:
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

%%
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
end


%% New: Bruder's recommendation
figure(2), hold on
sustained_duration = 4; % how many samples to average
z_mean = zeros(1,sustained_duration);
for idx = (sustained_duration+1):length(z_pow)
    z_mean = [z_mean mean(z_pow((idx-sustained_duration):idx))];
end
plot(t, z_mean, 'k', 'linewidth', 3)
legend('Power',['Power (LPF mean of ', num2str(sustained_duration), ' samples)'])



%% Finite Filter Tuning
format longg
figure(10)
hold off
plot(0,0)
hold on
grid on 

plot(t, z_pow, 'color', [0.9,0.9,0.9])
Fs = 100;
lpf_hz = 12;

% FIR filter
omega = 2*pi*lpf_hz ; % cutoff frequency rad/sample
N = 8;
tau = (0:N-1)./Fs;
FIR = exp(-1*omega*tau); % inverse laplace LPF 1/(s+Wc)
FIR = FIR./sum(FIR); % force filter to have no DC offset

X = z_pow;
H = FIR;

% replicates conv('full')
L = length(X)+length(H)-1;
y = zeros(1, L);

for n=0:length(X)-1
    y(n+1) = 0;
    for k = 0:n
        if((n-k+1) <= length(H)) % can we index H here or is it zero?
            y(n+1) = y(n+1) + X(k+1) * H(n - k + 1);
        end
    end
end

plot(t, y(1:length(t)), 'm')

xlabel('t (s)')
ylabel('Power')


%%

fer = [0.5308,0.2497,0.1175,0.0553,0.0260,0.0122,0.0058,0.0027];
er = max(FIR-fer)

ogData = [0.53011,4.77098,0.71073,0.09398,0.02350,0.02350,0.47578,0.09398,0.01322,0.00587,0.00147,0.03671,0.00147,1.79885,0.00587,1.23496,0.37592,0.24817,0.42438,0.37592,0.07195,0.05286,0.24817,0.28782,0.24817,0.11894,0.33040,0.77681,0.84583,0.21146,0.14684,0.11894,0.01322,0.14684,0.07195,0.00147,0.03671,0.05286,0.28782,0.42438,0.00587,0.02350,0.05286,0.02350,0.07195,0.00147,0.03671,0.11894,0.05286,0.07195,0.17768,0.28782,3.38330,1.50369,0.07195,0.71073,0.24817,0.02350,0.02350,0.09398,0.11894,0.09398,0.00587,0.01322,0.14684,0.07195,0.14684,0.07195,7.40244,0.14684,0.64758,0.47578,0.00147,0.71073,0.71073,0.24817,0.07195,0.14684,0.17768,0.02350,0.03671,0.05286,0.00587,0.00000,0.00147,0.00147,0.00147,0.00147,0.00587,0.00000,0.00147,0.00147,0.00147,0.00147,0.00147,0.01322,0.00147,0.00587,0.00587,0.00000,0.00147,0.00147,0.00000,0.01322,0.00000,0.00147,0.00587,0.00000,0.00147,0.00587,0.00147,0.00147,0.00587,0.00147,0.00147,0.00000,0.00587,0.00000,0.00147,0.00587,0.00000,0.00147,0.00147,0.00147,0.00000,0.00147,0.00147,0.00147,0.00587,0.00147,0.00147,0.00147,0.00000,0.00147,0.00000,0.00147,0.00147,0.00147,0.00147,0.00587,0.00587,0.00147,0.00147,0.00587,0.00147,0.00000,0.00000,0.00147,0.00147,0.00587,0.00587,0.00147,0.00147,0.00147,0.00147,0.00147,0.00147,0.00147,0.00000,0.00147,0.00147,0.00000,0.00000,0.00587,0.00147,0.00587,0.00000,0.00000,0.00587,0.00000,0.00587,0.00147,0.00147,0.00587,0.00147,0.00587,0.00587,0.00587,0.00147,0.00147,0.00587,0.00147,0.00147,0.00587,0.00587,0.00147,0.00147,0.00000,0.00587,0.00147,0.00147,0.00000,0.00587,0.00000,0.00000,0.00147,0.00000,0.00000,0.00000,0.00147];
ys = [0.28137, 2.66475, 1.63098, 0.81724, 0.39697, 0.19924, 0.34628, 0.21280, 0.10646, 0.04713, 0.02205, 0.02974, 0.01474, 0.96171, 0.45498, 0.86945, 0.60858, 0.41805, 0.42194, 0.39801, 0.22545, 0.13184, 0.19374, 0.24235, 0.24527, 0.17821, 0.25868, 0.53354, 0.69989, 0.44146, 0.28533, 0.19701, 0.09939, 0.12455, 0.09637, 0.04513, 0.03964, 0.04644, 0.17443, 0.30717, 0.14762, 0.08174, 0.06642, 0.04372, 0.05872, 0.02834, 0.03245, 0.07786, 0.06468, 0.06860, 0.12652, 0.21226, 1.89558, 1.68998, 0.83326, 0.76913, 0.49353, 0.24458, 0.12732, 0.10942, 0.11030, 0.09986, 0.05001, 0.02964, 0.09157, 0.08125, 0.11614, 0.09271, 3.97257, 1.94688, 1.25971, 0.84520, 0.39825, 0.56452, 0.64266, 0.43400, 0.23295, 0.18736, 0.18163, 0.09732, 0.06527, 0.05786, 0.02944, 0.01353, 0.00705, 0.00391, 0.00239, 0.00188, 0.00395, 0.00179, 0.00162, 0.00154, 0.00150, 0.00148, 0.00148, 0.00771, 0.00440, 0.00519, 0.00556, 0.00261, 0.00201, 0.00172, 0.00081, 0.00738, 0.00347, 0.00240, 0.00424, 0.00200, 0.00172, 0.00392, 0.00263, 0.00200, 0.00406, 0.00269, 0.00204, 0.00096, 0.00357, 0.00167, 0.00156, 0.00385, 0.00180, 0.00163, 0.00154, 0.00151, 0.00070, 0.00111, 0.00130, 0.00138, 0.00377, 0.00255, 0.00198, 0.00171, 0.00080, 0.00116, 0.00054, 0.00103, 0.00126, 0.00137, 0.00142, 0.00378, 0.00490, 0.00308, 0.00223, 0.00416, 0.00274, 0.00129, 0.00060, 0.00106, 0.00127, 0.00371, 0.00486, 0.00306, 0.00222, 0.00182, 0.00164, 0.00155, 0.00151, 0.00148, 0.00069, 0.00110, 0.00130, 0.00061, 0.00028, 0.00325, 0.00231, 0.00420, 0.00198, 0.00093, 0.00355, 0.00167, 0.00390, 0.00261, 0.00200, 0.00405, 0.00269, 0.00438, 0.00517, 0.00555, 0.00338, 0.00237, 0.00423, 0.00276, 0.00208, 0.00409, 0.00503, 0.00314, 0.00225, 0.00106, 0.00361, 0.00248, 0.00194, 0.00091, 0.00354, 0.00166, 0.00078, 0.00115, 0.00053, 0.00025, 0.00011, 0.00083, 0.00038, 0.00018, 0.00009, 0.00004, 0.00002, 0.00001, 0.00000];
figure
hold on
plot(ogData,'r')
plot(ys,'b', 'linewidth', 2)

plot(conv(ogData + 0.1, FIR),'g')




