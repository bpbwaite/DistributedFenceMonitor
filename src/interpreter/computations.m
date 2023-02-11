%% Battery Life

bat_capacity = 1500; % mAh
efficiency = 0.7; % how much of the battery is usable

% milliAmperes wrt 3.3v rail:
draw_constant = 0.1;    % 0.1 to 0.5
draw_other = 0.027;     % cryptochip, io, flash
draw_cpu = 4.6;         % 4.6 to 5.7
draw_receiver = 9.9;    % 9.9 to 21.5
draw_transmitter = 70;  % up to 128

dc_cpu = 100e-3/5;
dc_receiver = 0;
dc_transmitter = 97e-3/5;


current_draw = draw_constant + draw_other + ...
    draw_cpu*dc_cpu + draw_receiver*dc_receiver + draw_transmitter*dc_transmitter; 
s = efficiency * bat_capacity / current_draw * 3600;
time = sprintf('%01uh %02um %02us',...
    floor(s/60^2),...
    floor(s/60 - 60*floor(s/60^2)),... 
    floor(s - 3600*floor(s/60^2) - 60*floor(s/60 - 60*floor(s/60^2)))...
    );
disp(time)