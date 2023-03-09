%/*
%  FILE: Better Fence Test Viewer
%  VERSION: 1.0.0
%  TEST DATE:
%  DATE: 8 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION:
%*/

%% Load Data
clearvars
close all
load('fence_N.mat')

%% Error Corrections?
samples = 3;
%% Plots

figure, hold on
grid on
all_tx = 2:18;
for t=all_tx
    if t == 18
        tx_to_plot = 20;
    else
        tx_to_plot = t;
    end
    
    panels_at_tx = databuf.panel(databuf.tx == tx_to_plot);
    rssi_at_tx = databuf.rssi(databuf.tx == tx_to_plot);
    plot(panels_at_tx+t/64, rssi_at_tx, 'o', 'Color', [t/18 1-t/18 0])
    
end

title('Signal Strength vs. Distance')
subtitle(['Configuration: Ch.1 on SF7BW125, ', num2str(samples), ' Samples'])
xlabel('Distance Between Transceivers (Panels)')
ylabel('RSSI (dB mW)')
xlim([0 25])
xticks(0:1:25)
ylim([-125 -30])
yticks(-120:5:-30)

lgd = legend();
for L = 1:length(all_tx)
    if all_tx(L) == 18
        lgd.String{L} = 'TX\_PA\_BOOST';
    else
        lgd.String{L} = ['TX', num2str(all_tx(L))];
    end
    
end

