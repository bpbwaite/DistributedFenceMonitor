%/*
%  FILE: test_fence_view.m
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

%% Plots

figure, hold on
grid on
plot(databuf.panel, databuf.rssi, 'ro', 'linewidth', 2)
xlabel('Panels Between Transceivers')
ylabel('RSSI, dB mW')

% plot mean at each panel
for p=min(databuf.panel):max(databuf.panel)
    acc = 0;
    n = 0;
    for m = 1:length(databuf.panel)
        if(databuf.panel(m) == p)
            acc = acc + databuf.rssi(m);
            n = n + 1;
        end
    end
    if(n > 0)
        plot(p, acc/n, 'k*', 'linewidth', 2)
    end
end
%% Plots

figure, hold on
grid on

x = databuf.panel;
y = databuf.rssi;

% plot mean at each panel only
for p=min(databuf.panel):max(databuf.panel)
    acc = 0;
    n = 0;
    for m = 1:length(databuf.panel)
        if(databuf.panel(m) == p)
            acc = acc + databuf.rssi(m);
            n = n + 1;
        end
    end
    if(n > 0)
        plot(p, acc/n, 'r*', 'linewidth', 2)
    end
end

title('Signal Strength vs. Distance')
subtitle('Configuration: Ch.1 on SF7BW125, 3 Samples')
xlabel('Distance Between Transceivers (Panels)')
ylabel('Mean RSSI (dB mW)')
xticks(0:1:25)
yticks(-120:5:-45)