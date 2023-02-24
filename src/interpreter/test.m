

%% testing

clearvars
close all

crc = 1;
sf = 7;
sw = 0x12;
plen = 8;



data = [
(randi(2,1,32)-1)'
(randi(2,1,16)-1)'
(randi(2,1,16)-1)'
(randi(2,1,16)-1)'
(randi(2,1,16)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,32)-1)'
(randi(2,1,8)-1)'
]';

symbolTime = 0.15;

listenTo(data, sf, sw, crc, plen, symbolTime);