%/*
%  FILE: Live Viewer for accelerometer
%  VERSION: 1.0.7
%  DATE: 24 April 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Streams 3-axis accelerometer data over serial
%*/

%% Setup
close all
clear
clc
format compact

%% Constants & Config
show_all_axes = false;
baud = 115200; % bps
timeout = 10; % s
%% ADXL configuration

lsb_per_g_xy = 265; % you read that correctly. assumes vcc = 3.3v
lsb_per_g_z = 256;
g = 9.81;

Fs = 100;

severities = [
    1.4507
    2.1046
    3.0532
    4.4294
    6.4259
    9.3223
    13.5242
    19.6200
    28.4634
    41.2927
    59.9047
    86.9057
    126.0770
    182.9040
    265.3448
    ];
sev_lsbs = sqrt(severities) / g * lsb_per_g_z;

%% Connection
spla = "COM15";
s = serialport(spla, 115200);
s.Timeout = timeout;

disp("Setup complete!")
disp("Receiving")
disp("(you can ignore the error below)")

%% Ingest
global databuf n keepback

n = 1;

t_window = 5; % seconds of data to show
keepback = t_window * Fs;

databuf.x = zeros(1, keepback);
databuf.y = zeros(1, keepback);
databuf.z = zeros(1, keepback);

figure(1), hold on
grid on
shg
xlabel('Time (sec)')
ylabel('Acceration (m/s^2)')
if show_all_axes
    h1 = plot(0, 0, 'r', 'linewidth', 2);
    h2 = plot(0, 0, 'g', 'linewidth', 2);
end
h3 = plot(0, 0, 'b', 'linewidth', 2);

h_pointer = plot([0 0], [-20 20], 'k--');

ylim([-20 20])

xlower = 0 - 0.1;
xupper = t_window + 0.1;
xlim([xlower, xupper])
t = 0:1 / Fs:t_window;
t = t(1:end - 1);
if show_all_axes
    set(h1, 'XData', t)
    set(h2, 'XData', t)
end
set(h3, 'XData', t)

% show thresholds
th_ms2 = (7.0 * 62.5/1000.0 * g);
th_lsbs = th_ms2 / g * lsb_per_g_z;

plot([t(1) t(end)], [th_ms2 th_ms2], 'k--')
plot([t(1) t(end)], [-th_ms2 -th_ms2], 'k--')
if show_all_axes
    legend('X Accel', 'Y Accel', 'Z Accel', '', '')
else
    legend('Z-Axis Acceleration', '','')
end
title('Live Motion Viewer')

txt = text(4.5,-13.2,'0','HorizontalAlignment', 'right');
txt.FontSize = 48;
txt.FontName = "Consolas";


configureCallback(s, "terminator", @handleSerialData)
%%
figure(1)
fps = 24;
fperinday = (1 / fps) / 60/60/24;

while 1
    completed_last = now;
    
    while now - completed_last < fperinday
        % yield
    end
    try
        if show_all_axes
            set(h1, 'YData', databuf.x * g / lsb_per_g_xy);
            set(h2, 'YData', databuf.y * g / lsb_per_g_xy);
        end
        set(h3, 'YData', databuf.z * g / lsb_per_g_z);
        
        cur = abs(databuf.z(max(1, mod(n, keepback))));
        highest = max(abs(databuf.z));
        if  ~show_all_axes && cur >= th_lsbs
            h3.Color = [1 0 0];
        else
            h3.Color = [0 0 1];
        end
        sv = 0;
        svmax = 0;
        for x=1:length(sev_lsbs)
            if cur < sev_lsbs(x)
                sv = x-1;
                break
            end
        end
        for x=1:length(sev_lsbs)
            if highest < sev_lsbs(x)
                svmax = x-1;
                break
            end
        end
        
        
        txt.String = [num2str(sv) + " (" + num2str(svmax) + ")"];
        t_pointer = mod(n, keepback)/Fs;
        set(h_pointer, 'XData', [t_pointer t_pointer])
        drawnow
        
    catch
        % do nothing on fail
    end
    
end

%% Callback Function
function handleSerialData(src, ~)
global databuf n keepback

if n == 1
    src.flush
    n = n + 1;
    return
else
    
    databuf.x(mod(n, keepback) + 1) = read(src, 1, "int32");
    databuf.y(mod(n, keepback) + 1) = read(src, 1, "int32");
    databuf.z(mod(n, keepback) + 1) = read(src, 1, "int32");
    % extract terminator
    read(src, 2, "uint8");
    
    n = n + 1;
end

end
