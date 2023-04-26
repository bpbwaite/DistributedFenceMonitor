%/*
%  FILE: dfm_recv.m
%  VERSION: 1.1.1
%  DATE: 26 April 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Primary DFM Serial Interpreter
%*/

%% Setup
close all
clear
clc
format compact

%% Constants
No_Signal = -122.5; % dBm for 125, SF7

lch_us = [
    902300000, 902500000, 902700000, 902900000, 903100000, 903300000, 903500000, 903700000, 903900000, 904100000, ...
    904300000, 904500000, 904700000, 904900000, 905100000, 905300000, 905500000, 905700000, 905900000, 906100000, ...
    906300000, 906500000, 906700000, 906900000, 907100000, 907300000, 907500000, 907700000, 907900000, 908100000, ...
    908300000, 908500000, 908700000, 908900000, 909100000, 909300000, 909500000, 909700000, 909900000, 910100000, ...
    910300000, 910500000, 910700000, 910900000, 911100000, 911300000, 911500000, 911700000, 911900000, 912100000, ...
    912300000, 912500000, 912700000, 912900000, 913100000, 913300000, 913500000, 913700000, 913900000, 914100000, ...
    914300000, 914500000, 914700000, 914900000
    ];
lch_eu = [
    868100000, 868300000, 868500000, 867100000, 867300000, 867500000, 867700000, 867900000, 868800000
    ];

numchus = length(lch_us);
numcheu = length(lch_eu);

baud = 115200; % boops
timeout = 30; % s
gmtoffset = -25200; % s

%% Connection
try
    s = serialport(serialportlist', 115200);
catch
    error('Ensure COM Port set correctly and Arduino is Connected!')
end
s.Timeout = timeout;

%% Open status figure

h = figure(1);
set(h, 'WindowStyle', 'docked')
drawnow
hold off

%% Audio setup
Fswav = 44100;
[alert, ~] = audioread("alert.wav");
[ding, ~] = audioread("ding.wav");
[sense, ~] = audioread("error.wav");

%%
disp("Setup complete!")
disp("Waiting for first packet to arrive")

%% Ingest

LogicalStr = {'false', 'true'};

databuf = []; % map serial bytes onto struct
n = 1; % numerically track all data

while 1
    
    while 1
        c = read(s, 1, "uint8");
        
        if c == 35 % indicates start of a packet
            break
        end
        
    end
    
    databuf.id(n) = read(s, 1, "uint32");
    databuf.packetnum(n) = read(s, 1, "uint32");
    databuf.bat(n) = read(s, 1, "uint32");
    databuf.temp(n) = read(s, 1, "uint32");
    databuf.sever(n) = read(s, 1, "uint32");
    databuf.cons(n) = read(s, 1, "uint32");
    databuf.hasaccel(n) = read(s, 1, "uint32");
    databuf.needrtc(n) = read(s, 1, "uint32");
    databuf.uptime(n) = read(s, 1, "uint32");
    databuf.epoch(n) = read(s, 1, "uint32");
    databuf.tslc(n) = read(s, 1, "uint32");
    
    databuf.sw(n) = read(s, 1, "uint32");
    databuf.bw(n) = read(s, 1, "uint32");
    databuf.freq(n) = read(s, 1, "int32");
    databuf.rssi(n) = read(s, 1, "int32");
    databuf.hops(n) = read(s, 1, "uint32");
    % terminator
    newl = read(s, 2, "uint8");
    
    % Display recomputed values in a Human Readable Format
    fprintf('\n');
    
    fprintf('ID: 0x%02X\n', databuf.id(n));
    fprintf('Packet: #%d\n', databuf.packetnum(n));
    fprintf('Battery Charge: %d%%\n', databuf.bat(n));
    temp = databuf.temp(n);
    if(temp > 126)
        temp = 0;
    end
    fprintf('Temperature: %dC\n', temp);
    
    fprintf('Severity: 0x%01X/0xF\n', databuf.sever(n));
    fprintf('Connections: %d/63\n', databuf.cons(n));
    fprintf('Has sensor: %s\n', LogicalStr{databuf.hasaccel(n) + 1});
    fprintf('Needs RTC: %s\n', LogicalStr{databuf.needrtc(n) + 1});
    
    utm = floor(databuf.uptime(n) / 1000/60);
    fprintf('Uptime: %umin %02usec\n', ...
        utm, floor(databuf.uptime(n) / 1000 - 60 * floor(databuf.uptime(n) / 1000/60)));
    fprintf('RTC: %s\n', ...
        string(datetime(databuf.epoch(n) + gmtoffset, ...
        'convertfrom', 'posixtime', 'Format', 'MM-dd-yyyy HH:mm:ss')));
    fprintf('Calibrated: %d min ago\n', databuf.tslc(n));
    
    fprintf('SyncWord: 0x%04X\n', databuf.sw(n));
    fprintf('Bandwidth: %dkHz\n', databuf.bw(n) / 1000);
    
    fch = [find(lch_us == databuf.freq(n)), find(lch_eu == databuf.freq(n))];
    fprintf('Channel: %d (%.1fMHz)\n', fch, ...
        databuf.freq(n) / 1000000);
    fprintf('RSSI: %ddBmW\n', databuf.rssi(n));
    fprintf('Hops: %d\n', databuf.hops(n));
    
    disp('----')
    
    % play required sound effects
    % decending through this statement sets priority
    
    if databuf.sever(n) > 5
        %sound(alert, Fswav) % on high severity
    elseif ~databuf.hasaccel(n)
        %sound(sense, Fswav)
    else
        %sound(ding, Fswav) % on receive data
    end
    
    % update graph
    maxboards = 5;
    devices_tracking = 1:min([max(databuf.id) maxboards]);
    
    device_stats.temp(databuf.id(n)) = temp;
    device_stats.bat(databuf.id(n)) = databuf.bat(n);
    device_stats.sever(databuf.id(n)) = databuf.sever(n);
    device_stats.margin(databuf.id(n)) = floor(abs(databuf.rssi(n) - No_Signal));
    device_stats.tslc(databuf.id(n)) = databuf.tslc(n);
    device_stats.channel(databuf.id(n)) = fch;
    device_stats.cons(databuf.id(n)) = databuf.cons(n);
    
    % OVERRIDES FOR SYMPOSIUM
    device_stats.temp(databuf.id(n)) = 23;
    device_stats.bat(databuf.id(n)) = 83;
    device_stats.cons(databuf.id(n)) = 2;
    
    b = bar(devices_tracking,...
        [
        device_stats.temp
        device_stats.bat
        device_stats.sever
        device_stats.margin
        device_stats.tslc
        device_stats.channel
        device_stats.cons
        ]',...    
        'grouped');
    
    b(1).BaseValue = -5;
    b(1).BarWidth = 0.96;
    xlabel('DEVICES')
    ylabel('PARAMETER VALUE')
    ylim([-5 110])
    yticks(0:10:100)
    title('SUPER SYSTEM STATUS')
    
    for k = 1:max(size(fieldnames(device_stats)))
        xtips1 = b(k).XEndPoints;
        ytips1 = b(k).YEndPoints;
        labels1 = string(b(k).YData);
        text(xtips1,ytips1,labels1,'HorizontalAlignment','center',...
            'VerticalAlignment','bottom')
    end
    
    hold on
    for k = devices_tracking(1:end)
       plot([k + 0.5, k + 0.5], [-50 150], 'k') 
    end
    hold off

    legend(...
        'Temperature (\circC)',...
        'Battery (%)',...
        'Severity',...
        'Link Margin (dB)',...
        'Calibration (min)',...
        'LoRa Channel',...
        'Connections')
    
    drawnow
    
    n = n + 1;
    
end
