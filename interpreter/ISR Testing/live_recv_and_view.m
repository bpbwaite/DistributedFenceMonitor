%/*
%  FILE: Live Viewer for accelerometer
%  VERSION: 1.0.5
%  DATE: 19 April 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Streams 3-axis accelerometer data over serial
%*/

%% Setup
close all
clear
clc
format compact

%% Constants

baud = 115200; % bps
timeout = 10; % s
%% ADXL configuration

lsb_per_g_xy = 265; % you read that correctly. assumes vcc = 3.3v
lsb_per_g_z = 256;
g = 9.81;

Fs = 100;
%% Connection
spla = serialportlist';
spla = spla(1);
s = serialport(spla, 115200);
s.Timeout = timeout;

disp("Setup complete!")
disp("Receiving")

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
h1 = plot(0, 0, 'r', 'linewidth', 2);
h2 = plot(0, 0, 'g', 'linewidth', 2);
h3 = plot(0, 0, 'b', 'linewidth', 2);
ylim([-20 20])

xlower = 0 - 0.5;
xupper = t_window + 0.5;
xlim([xlower, xupper])
t = 0:1 / Fs:t_window;
t = t(1:end - 1);
set(h1, 'XData', t)
set(h2, 'XData', t)
set(h3, 'XData', t)

% show thresholds
th_ms2 = (0x7 * 62.5/1000.0 * 9.81);
plot([t(1) t(end)], [th_ms2 th_ms2], 'k--')
plot([t(1) t(end)], [-th_ms2 -th_ms2], 'k--')

legend('X Accel', 'Y Accel', 'Z Accel', '', '')

configureCallback(s, "terminator", @handleSerialData)
%%
figure(1)
fps = 24;
fperinday = (1 / fps) / 60/60/24;

while 1
    completed_last = now;

    while now - completed_last < fperinday
    end

    set(h1, 'YData', databuf.x * g / lsb_per_g_xy);
    set(h2, 'YData', databuf.y * g / lsb_per_g_xy);
    set(h3, 'YData', databuf.z * g / lsb_per_g_z);

    drawnow

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
