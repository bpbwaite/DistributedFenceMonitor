function [] = listenTo(bits, sf, sw, crc, plen, symtime)
%/*
%  FILE: listenTo.m
%  VERSION: 1.0.1
%  DATE: 23 February 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  REQUIREMENTS: Communications Toolbox
%  DESCRIPTION: Interpret a LoRa packet as sound
%*/
%% Full Error Handling
if nargin < 1
    error('No Data')
end
if nargin < 2
    sf = 7;
end
if nargin < 3
    sw = 0x12;
end
if nargin < 4
    crc = 1;
end
if nargin < 4
    plen = 8;
end
if nargin < 4
    symtime = 0.1;
end
%% Constants
sf = 7; % force spreadfactor sorry
CR = 5; % coding rate, in 4/?
tone_min = 500; % Hz
tone_max = 900; % Hz
tone_mid = (tone_max+tone_min)/2;
tone_diff = tone_max-tone_min;
sec_per_symbol = symtime; % s

if crc > 0
    crc = 2; % symbols
else
    crc = 0;
end

%% Compute vector
packetLen = ceil(length(bits)/8); % in bytes

% allocate time and output vector
Ts = 1/((2+2)*tone_max);
Fs = floor(1/Ts);

t = 0:Ts:...
    (plen + 4.25 + (ceil((8*packetLen)-(4*sf)+8+crc)/(4.0*sf))*CR+8)*sec_per_symbol;

y = zeros(1, length(t));

%% construct preamble, sw, iq
t_symbol = ceil(sec_per_symbol*Fs);
t_halfsymbol = ceil(sec_per_symbol*Fs / 2) + 1;
t_threefourthsymbol = ceil(3*sec_per_symbol*Fs / 4) + 1;
packet_times = t_symbol + 1;
% construct first upper half
pre_a = [];
pre_a = chirp(t(1:t_halfsymbol), tone_mid, t(t_halfsymbol), tone_max);
for sy=1:(plen-1)
    pre_a = [pre_a, chirp(t(1:packet_times), tone_min, t(packet_times), tone_max)];
end
% construct second lower half
pre_a = [pre_a, chirp(t(1:t_halfsymbol), tone_min, t(t_halfsymbol), tone_mid)];
% add sync word
sw_a = [];
    symbol1 = floor(double(sw) / (2^sf));
    symbol2 = floor(double(sw) - (2^sf) * symbol1);
    maxchip = 2^sf;
    
    freq_start = tone_min + tone_diff * symbol1/maxchip;
    nibbletime = ceil(packet_times*(1.0 - symbol1/maxchip));
    
    sw_a = [sw_a, chirp(t(1:nibbletime), freq_start, t(nibbletime), tone_max)]; 
    sw_a = [sw_a, chirp(t(1:(packet_times-nibbletime)), tone_min, t(packet_times), tone_max)]; 
    
    freq_start = tone_min + tone_diff * symbol2/maxchip;
    nibbletime = ceil(packet_times*(1.0 - symbol2/maxchip));
    
    sw_a = [sw_a, chirp(t(1:nibbletime), freq_start, t(nibbletime), tone_max)]; 
    sw_a = [sw_a, chirp(t(1:(packet_times-nibbletime)), tone_min, t(packet_times), tone_max)]; 
    
% invert IQ to symbolize end of preamble:
inv_a = [];
inv_a = chirp(t(1:t_halfsymbol), tone_mid, t(t_halfsymbol), tone_min);
inv_a = [inv_a, chirp(t(1:packet_times), tone_max, t(packet_times), tone_min)]; 
inv_a = [inv_a, chirp(t(1:t_threefourthsymbol), tone_max, t(packet_times), tone_min)]; 

%% construct data & crc

pay_a = [];
pay_a = 0;
% how does CR work? not implemented :(
maxchip = 2^sf;
for k=1:packetLen
    cursymbol = bits(k:k+sf-1);
    data = 0;
    for n=1:length(cursymbol)
       data = data + cursymbol(n) * 2^(length(cursymbol)-n);
    end
    freq_start = tone_min + tone_diff * data/maxchip;
    nibbletime = ceil(packet_times*(1.0 - double(data)/maxchip))+1;
    pay_a = [pay_a, chirp(t(1:nibbletime), freq_start, t(nibbletime), tone_max)]; 
    pay_a = [pay_a, chirp(t(1:(packet_times-nibbletime)), tone_min, t(packet_times), tone_max)]; 

end

crcgen = comm.CRCGenerator('z^16+z^12+z^5+1', 'ChecksumsPerFrame',2);
bits_with_crc = crcgen(logical(bits'));
crc_alone = double(bits_with_crc(end-15:end)');
crc = 0;
for k=1:length(crc_alone)
   crc = crc + crc_alone(k) * 2^(length(crc_alone)-k);
end
% currently only works for SF=7
symbolcrc(1) = double(min(bitand(crc, 0b111111100000000000000)/(2^(2*sf)), 2^sf-1));
symbolcrc(2) = double(min(bitand(crc, 0b000000011111110000000)/ (2^sf), 2^sf-1));
symbolcrc(3) = double(min(bitand(crc, 0b000000000000001111111), 2^sf-1));
maxchip = 2^sf;

crc_a = [];
for k=1:3
    freq_start = tone_min + tone_diff * symbolcrc(k)/maxchip;
    nibbletime = ceil(packet_times*(1.0 - double(symbolcrc(k))/maxchip))+1;
    
    crc_a = [crc_a, chirp(t(1:nibbletime), freq_start, t(nibbletime), tone_max)]; 
    crc_a = [crc_a, chirp(t(1:(packet_times-nibbletime)), tone_min, t(packet_times), tone_max)]; 
end
%% Playback
y = [pre_a sw_a inv_a pay_a crc_a];
close all
vol = 0.1;
sound(y.*vol, Fs)
%pspectrum(y,Fs,'spectrogram','TimeResolution',0.08,'OverlapPercent',99,'Leakage',0.85)

end