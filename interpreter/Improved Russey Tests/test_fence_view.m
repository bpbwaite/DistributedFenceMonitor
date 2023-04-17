%/*
%  FILE: Better Fence Test Viewer
%  VERSION: 1.0.1
%  TEST DATE: 9 March 2023
%  DATE: 9 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Plot relation between distance, tx power, and rssi
%*/

%% Load Data
clearvars
close all

load('fence_Braithwaite.mat')
databuf.panel = databuf.panel - 3;
% My assistant pushed the log button being instructed. subtract those pushes

%% Error Corrections?

samples = 3; % at each TX power
distance_between_panels = 2.67/2; % meters
No_Signal = -122.5; % dBm for 125, SF7

%% Plot: Link Margin

figure;
hold on
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

    plot(distance_between_panels * panels_at_tx + t*.05,...
        -1*No_Signal + rssi_at_tx, 'o', 'Color', [t/18 1-t/18 0]);
end
title('Signal Margin vs. Distance')
subtitle(['Configuration: Ch.1 on SF7BW125, ', num2str(samples), ' Samples'])
xlabel('Distance Between Transceivers (m)')
ylabel('Link Margin (dB)')
xlim([-1 55])
ylim([0 75])
%set(gca,'YScale','log')
%yticks(0:10:120)
lgd = legend();
for L = 1:length(all_tx)
    if all_tx(L) == 18
        lgd.String{L} = 'TX\_PA\_BOOST';
    else
        lgd.String{L} = ['TX', num2str(all_tx(L))];
    end

end
title(lgd,'Transmit Power');
drawnow

%% Plot: % Power Received
figure;
hold on
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

    plot(distance_between_panels * panels_at_tx + t*.05,...
        10.^(rssi_at_tx/10), '*', 'Color', [t/18 1-t/18 0], 'linewidth',2);
end

title('Signal Power vs. Distance')
subtitle(['Configuration: Ch.1 on SF7BW125, ', num2str(samples), ' Samples'])
xlabel('Distance Between Transceivers (m)')
ylabel('Power (mW)')
xlim([-1 55])
%ylim([1 120])
set(gca,'YScale','log')
%yticks(0:10:120)

lgd = legend();
for L = 1:length(all_tx)
    if all_tx(L) == 18
        lgd.String{L} = 'TX\_PA\_BOOST';
    else
        lgd.String{L} = ['TX', num2str(all_tx(L))];
    end

end
title(lgd,'Transmit Power');
drawnow
%% Plot: % Power Received wrt Power Usage

power_draw =[
    146.6, 153.2, 160.7, 169.8, 175.8, 189.0, 193.3, 206.8, 224.0, 238.0, 259.1, 282.0, 305.2, 345.0, 373.1, 417.8, 497.8,
]; % result copied from power test

figure;
hold on
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

    plot(distance_between_panels * panels_at_tx + t*.05-.45,...
        100 * (10.^(rssi_at_tx/10)) ./ power_draw(t-1),...
        'o', 'Color', [t/18 1-t/18 0]);
    
    greatest_efficiency = 0;
    best_power = 0;
    contender = mean(100 * (10.^(rssi_at_tx/10)) ./ power_draw(t-1));
    if contender > greatest_efficiency
        greatest_efficiency = contender;
        best_power = t;
    end
end

title('Relative Efficiency vs. Distance')
subtitle(['Configuration: Ch.1 on SF7BW125, ', num2str(samples), ' Samples'])
xlabel('Distance Between Transceivers (m)')
ylabel('Efficiency (%)')
xlim([-1 55])
%ylim([1 120])
set(gca,'YScale','log')
%yticks(0:10:120)

lgd = legend();
for L = 1:length(all_tx)
    if all_tx(L) == 18
        lgd.String{L} = 'TX\_PA\_BOOST';
    else
        lgd.String{L} = ['TX', num2str(all_tx(L))];
    end

end
title(lgd,'Transmit Power');
drawnow


%% Mesh Plot
% reduce rssi datapoints to mean rssi only
% must be divisible by samples (no missed data)
h=figure;

n = 1;
R = [];

for d = min(databuf.panel): max(databuf.panel)
    for t = 1:17 % maps onto 2-17, 20
        x = t+1;
        if(x == 18)
            x = 20;
        end
        R(t, d+1) = mean(databuf.rssi(databuf.panel == d & databuf.tx == x));
        n = n + 1;
    end
end


[D, T] = meshgrid(min(databuf.panel):max(databuf.panel), power_draw(1:17));
CO(:,:,1) = T/500; % red
CO(:,:,2) = (120+R)/70; % green
CO(:,:,3) = 1-(120+R)/70; % blue
mesh(D*distance_between_panels, T, R, CO, 'FaceAlpha','0.9', 'FaceColor', 'flat')

view(45, 30);
title('RSSI vs. Distance and TX Power')
hxl = xlabel('Distance Between Transceivers (m)',...
    'Rotation', -20, 'HorizontalAlignment', 'center');
xlim([0, 60])
xticks(0:5:60)
hyl = ylabel('TX Power (mW)', 'Rotation', 20, 'HorizontalAlignment', 'center');
ylim([0, 600])
yticks(0:50:600)
hzl = zlabel('RSSI (dBm)');
zlim([-120, -50])
zticks(-120:10:-50)

hxl.Position = [30 -75 -125];
hyl.Position = [67.5 300 -125];