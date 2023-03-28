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
s = serialport(serialportlist', 115200);
s.Timeout = timeout;

disp("Setup complete!")
disp("Waiting for first packet to arrive")

%% Ingest

LogicalStr = {'false', 'true'};

databuf = []; % map serial bytes onto struct
hrbuf = []; % human readable databuffer

while 1
    while 1
        c = read(s, 1, "uint8");
        if c == 35 % indicates start of a packet
            break
        end
    end
    
    databuf.id = read(s, 1, "uint32");
    databuf.packetnum = read(s, 1, "uint32");
    databuf.bat = read(s, 1, "uint32");
    databuf.temp = read(s, 1, "uint32");
    databuf.sever = read(s, 1, "uint32");
    databuf.cons = read(s, 1, "uint32");
    databuf.hasaccel = read(s, 1, "uint32");
    databuf.needrtc = read(s, 1, "uint32");
    databuf.uptime = read(s, 1, "uint32");
    databuf.epoch = read(s, 1, "uint32");
    databuf.tslc = read(s, 1, "uint32");
    
    databuf.sw = read(s, 1, "uint32");
    databuf.bw = read(s, 1, "uint32");
    databuf.freq = read(s, 1, "int32");
    databuf.rssi = read(s, 1, "int32");
    databuf.hops = read(s, 1, "uint32");
    % terminator
    newl = read(s, 2, "uint8");
    
    
    % Display recomputed values in a Human Readable Format
    fprintf('\n');
    show_list = true;
if show_list    
    fprintf('ID: 0x%02X\n', databuf.id);
    fprintf('Packet: #%d\n', databuf.packetnum);
    fprintf('Battery Charge: %d%%\n', databuf.bat);
    fprintf('Temperature: %dC\n', databuf.temp);
    
    fprintf('Severity: 0x%01X/0xF\n', databuf.sever);
    fprintf('Connections: %d/63\n', databuf.cons);
    fprintf('Has sensor: %s\n', LogicalStr{databuf.hasaccel + 1});
    fprintf('Needs RTC: %s\n', LogicalStr{databuf.needrtc + 1});
    fprintf('Uptime: %umin %01usec\n',...
        floor(databuf.uptime/1000/60), floor(databuf.uptime/1000 - 60*floor(databuf.uptime/1000/60)));
    fprintf('RTC: %s\n',...
        string(datetime(databuf.epoch + gmtoffset,...
        'convertfrom', 'posixtime', 'Format', 'MM-dd-yyyy HH:mm:ss')));
    fprintf('Calibration: %d min ago', databuf.tslc);
    
    fprintf('SyncWord: 0x%04X\n', databuf.sw);
    fprintf('Bandwidth: %dkHz\n', databuf.bw/1000);
    fprintf('Channel: %d (%.1fMHz)\n',...
        [find(lch_us == databuf.freq), find(lch_eu == databuf.freq)],...
        databuf.freq/1000000);
    fprintf('RSSI: %ddBmW\n', databuf.rssi);
    fprintf('Hops: %d\n', databuf.hops);
else
    ; % show in paragraph form
end
    disp('----')
    
    
    
end

