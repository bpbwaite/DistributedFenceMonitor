%% Load Data
clearvars
close all
load('fence_0.mat')
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

