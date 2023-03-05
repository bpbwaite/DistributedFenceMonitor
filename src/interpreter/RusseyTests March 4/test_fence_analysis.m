%% Load Data
clearvars
close all
load('fence_3.mat')
% Requirements: Curve Fitting Toolbox

% Conditions: SF7BW125, TX=15

% test 1: on the ground going down the fence
% test 2: tried holding vertical?

% test 3: went down fence, holding antenna vertically (parallel) (best)

%% Error Corrections

fenceWidth = 10; % ft (?)

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
subtitle('Configuration: SF7BW125, Tx = 15')
xlabel('Distance Between Transceivers (Panels)')
ylabel('RSSI (dB mW)')
xticks(0:1:25)
yticks(-120:5:-45)
