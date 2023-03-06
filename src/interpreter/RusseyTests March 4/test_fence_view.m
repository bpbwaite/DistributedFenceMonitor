%/*
%  FILE: test_fence_view.m
%  VERSION: 1.0.0
%  TEST DATE: 4 March 2023
%  DATE: 4 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION:
%*/

%% Load Data
clearvars
close all
load('fence_3.mat')
% test 1: on the ground going down the fence
% test 2: tried holding vertical?
% test 3: went down fence, holding vertical :3
disp('Data:')

disp([databuf.rssi' databuf.snr'])

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

figure, hold on
grid on
plot(databuf.panel, databuf.snr, 'bo', 'linewidth', 2)
xlabel('Panels Between Transceivers')
ylabel('SNR (>7 is good)')
xlim([-1 23])

% plot mean at each panel
for p=min(databuf.panel):max(databuf.panel)
    acc = 0;
    n = 0;
    for m = 1:length(databuf.panel)
        if(databuf.panel(m) == p)
            acc = acc + databuf.snr(m);
            n = n + 1;
        end
    end
    if(n > 0)
        plot(p, acc/n, 'k*', 'linewidth', 2)
    end
end

