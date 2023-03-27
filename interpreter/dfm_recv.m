%/*
%  FILE: dfm_recv.m
%  VERSION: 0.0.1
%  DATE: 11 February 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Serial Interpreter & Plotter
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

baud = 115200; % bps
timeout = 30; % s
gmtoffset = -25200; % s

% battery voltage divider values
vbat_max = 4.2; % volts at 100% charge
vbat_min = 3.3; % volts when dead
vref = 3.3; % volts
adc_res = 12; % bits
R6 = 680000; % upper
R7 = 330000; % lower
mV_per_pt = vref / (2^adc_res) / (R7/(R6+R7));
%% Connection
s = serialport(serialportlist', 115200);
s.Timeout = timeout;
    
disp("Setup complete!")
disp("Waiting for first packet to arrive")

%% Ingest
databuf = []; % map serial bytes onto struct
hrbuf = []; % human readable databuffer

while 1
    while 1
       c = read(s, 1, "uint8");
       if c == 35 % start of a packet
           break
       end
    end
        databuf.packetnum = read(s, 1, "uint32");
        databuf.id = read(s, 1, "uint16");
        databuf.cons = read(s, 1, "uint16");
        databuf.stat = read(s, 1, "uint16");
        databuf.sw = read(s, 1, "uint16");
        databuf.bat = read(s, 1, "uint32");
        databuf.freq = read(s, 1, "int32");
        databuf.uptime = read(s, 1, "uint32");
        databuf.toa = read(s, 1, "uint32");
        databuf.temp = read(s, 1, "single");
        databuf.epoch = read(s, 1, "uint32");
        % extra-struct values
        % terminator
        newl = read(s, 2, "uint8");
        
        % computed values
        hrbuf.id = sprintf('0x%02X',...
            databuf.id);
        hrbuf.packet = sprintf('#%d',...
            databuf.packetnum);
        hrbuf.status = sprintf('0x%02X',...
            databuf.stat);
        hrbuf.connections = sprintf('%d',...
            databuf.cons);
        hrbuf.batt = sprintf('%0.2fV (%d%%)',...
            databuf.bat * mV_per_pt,...
            max(0,floor(((databuf.bat*mV_per_pt-vbat_min)*100)/(vbat_max-vbat_min))));
        hrbuf.freq = sprintf('%.1fMHz (Ch.%d)',...
            databuf.freq/1000000,...
            [find(lch_us == databuf.freq), find(lch_eu == databuf.freq)]);
        hrbuf.syncword = sprintf('0x%04X',...
            databuf.sw);
        
        hrbuf.uptime = sprintf('%01um%02us',...
            floor(databuf.uptime/1000/60),...
            floor(databuf.uptime/1000 - 60*floor(databuf.uptime/1000/60)));
        hrbuf.toa = sprintf('%d ms',...
            databuf.toa);
        hrbuf.temperature = sprintf('%.1fC',...
            databuf.temp);
        hrbuf.epoch = sprintf('%s',...
            string(datetime(databuf.epoch + gmtoffset,...
            'convertfrom', 'posixtime', 'Format', 'MM-dd-yyyy HH:mm:ss')));
        
        disp(hrbuf)
        disp('----')
        
        
        
end

