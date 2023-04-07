%/*
%  FILE: dfm_recv.m
%  VERSION: 1.0.1
%  DATE: 27 March 2023
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
lch_us = [
    902300000, 902500000, 902700000, 902900000, 903100000, 903300000, 903500000, 903700000, 903900000, 904100000,...
    904300000, 904500000, 904700000, 904900000, 905100000, 905300000, 905500000, 905700000, 905900000, 906100000,...
    906300000, 906500000, 906700000, 906900000, 907100000, 907300000, 907500000, 907700000, 907900000, 908100000,...
    908300000, 908500000, 908700000, 908900000, 909100000, 909300000, 909500000, 909700000, 909900000, 910100000,...
    910300000, 910500000, 910700000, 910900000, 911100000, 911300000, 911500000, 911700000, 911900000, 912100000,...
    912300000, 912500000, 912700000, 912900000, 913100000, 913300000, 913500000, 913700000, 913900000, 914100000,...
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
s = serialport("COM5", 115200);
s.Timeout = timeout;

disp("Setup complete!")
disp("Waiting for first packet to arrive")


%% Audio setup

[alert, Fs] = audioread("alert.wav");
%% Ingest

LogicalStr = {'false', 'true'};

databuf = []; % map serial bytes onto struct
n = 1;        % numerically track all data
while 1
    while 1
        c = read(s, 1, "uint8");
        if c == 35 % indicates start of a packet
            break
        end
    end
    
    databuf(n).id = read(s, 1, "uint32");
    databuf(n).packetnum = read(s, 1, "uint32");
    databuf(n).bat = read(s, 1, "uint32");
    databuf(n).temp = read(s, 1, "uint32");
    databuf(n).sever = read(s, 1, "uint32");
    databuf(n).cons = read(s, 1, "uint32");
    databuf(n).hasaccel = read(s, 1, "uint32");
    databuf(n).needrtc = read(s, 1, "uint32");
    databuf(n).uptime = read(s, 1, "uint32");
    databuf(n).epoch = read(s, 1, "uint32");
    databuf(n).tslc = read(s, 1, "uint32");
    
    databuf(n).sw = read(s, 1, "uint32");
    databuf(n).bw = read(s, 1, "uint32");
    databuf(n).freq = read(s, 1, "int32");
    databuf(n).rssi = read(s, 1, "int32");
    databuf(n).hops = read(s, 1, "uint32");
    % terminator
    newl = read(s, 2, "uint8");
    
    
    % Display recomputed values in a Human Readable Format
    fprintf('\n');

    fprintf('ID: 0x%02X\n', databuf(n).id);
    fprintf('Packet: #%d\n', databuf(n).packetnum);
    fprintf('Battery Charge: %d%%\n', databuf(n).bat);
    fprintf('Temperature: %dC\n', databuf(n).temp);
    
    fprintf('Severity: 0x%01X/0xF\n', databuf(n).sever);
    fprintf('Connections: %d/63\n', databuf(n).cons);
    fprintf('Has sensor: %s\n', LogicalStr{databuf(n).hasaccel + 1});
    fprintf('Needs RTC: %s\n', LogicalStr{databuf(n).needrtc + 1});
    fprintf('Uptime: %umin %02usec\n',...
        floor(databuf(n).uptime/1000/60), floor(databuf(n).uptime/1000 - 60*floor(databuf(n).uptime/1000/60)));
    fprintf('RTC: %s\n',...
        string(datetime(databuf(n).epoch + gmtoffset,...
        'convertfrom', 'posixtime', 'Format', 'MM-dd-yyyy HH:mm:ss')));
    fprintf('Calibration: %d min ago\n', databuf(n).tslc);
    
    fprintf('SyncWord: 0x%04X\n', databuf(n).sw);
    fprintf('Bandwidth: %dkHz\n', databuf(n).bw/1000);
    fprintf('Channel: %d (%.1fMHz)\n',...
        [find(lch_us == databuf(n).freq), find(lch_eu == databuf(n).freq)],...
        databuf(n).freq/1000000);
    fprintf('RSSI: %ddBmW\n', databuf(n).rssi);
    fprintf('Hops: %d\n', databuf(n).hops);
    
    disp('----')
    
    
    
    % play any sound effects
    % on receive data
    % on high severity
    if databuf(n).sever > 5
        sound(alert, Fs)
    end
    
    
    n = n + 1;
    
end

