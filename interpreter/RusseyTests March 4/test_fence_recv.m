%/*
%  FILE: test_fence_recv.m
%  VERSION: 0.0.2
%  DATE: 
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Serial Ingest program for tence_fence_view.m and
%  test_fence_analysis.m
%*/

%% Setup
close all
clear
clc
format compact

%% Constants

baud = 115200; % bps
timeout = 120; % s

%% Connection
spla = "COM5";
s = serialport(spla, 115200);
s.Timeout = timeout;
    
disp("Setup complete!")
disp("Waiting for first packet to arrive")

%% Ingest

n = 1;
while 1
    while 1
       c = read(s, 1, "uint8");
       if c == 35 % start of a packet
           break
       end
    end
        databuf.panel(n) = read(s, 1, "int32");
        databuf.rssi(n) = read(s, 1, "int32");
        databuf.snr(n) = read(s, 1, "single");
        
        % terminator
        newl = read(s, 2, "uint8");
        
        fprintf("Point %d: %d dB at Panel %d (%.1f SNR)\n",...
            n,databuf.rssi(n), databuf.panel(n), databuf.snr(n))
        
        n = n+1;
       
        
end
%% CTRL-C to break from data collection
save('fence_N.mat', 'databuf');

