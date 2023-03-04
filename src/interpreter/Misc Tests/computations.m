%% Battery Life
% configuration
format compact, format longg
bat_capacity = 1500;    % mAh
efficiency = 0.8;       % how much of the battery is usable

%% Draw in milliAmperes wrt 3.3v rail:
% constants:
draw_constant = 0.1;    % 0.1 to 0.5
draw_other = 0.027;     % cryptochip, io, flash

% draw_cpu: 4.6 to 5.7
% draw_receiver: 9.9 to 21.5. typ 12.5ma on 868, 14.5ma on 915
% draw_transmitter: 28 to 128

%% optimistic values:
draw_cpu = 4.8;        
draw_receiver = 12.5;   
draw_transmitter = 80; 
%% pessimistic values:
draw_cpu = 5.5;         
draw_receiver = 15.0;   
draw_transmitter = 125;
%% best case:
draw_cpu = 4.6;         
draw_receiver = 9.9;    
draw_transmitter = 28;  
%% worst case scenario:
draw_cpu = 5.7;        
draw_receiver = 20.0;   
draw_transmitter = 128; 

%% Duty Cycles
%% expected value:
dc_cpu = 100e-3/5;
dc_receiver = 0;
dc_transmitter = 97e-3/5;
%% best case: never send, receive, or process anything
dc_cpu = 0;
dc_receiver = 0;
dc_transmitter = 0;
%% worst case scenario: always listening and processing, frequent transmitting
dc_cpu = 1;
dc_receiver = 1;
dc_transmitter = 0.01; % (legal limit)

%% Compute:
current_draw = draw_constant + draw_other + ...
    draw_cpu*dc_cpu + draw_receiver*dc_receiver + draw_transmitter*dc_transmitter; 
s = efficiency * bat_capacity / current_draw * 3600;
time = sprintf('%01uh %02um %02us',...
    floor(s/60^2),...
    floor(s/60 - 60*floor(s/60^2)),... 
    floor(s - 3600*floor(s/60^2) - 60*floor(s/60 - 60*floor(s/60^2)))...
    );
disp([time, '    (', num2str(s/(24*60*60)),'d)'])