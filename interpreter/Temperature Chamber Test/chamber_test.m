%/*
%  FILE: chamber_test.m
%  VERSION: 1.0.1
%  TEST DATE: 12 April 2023
%  DATE: 12 April 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite, Zachary Couch
%  DESCRIPTION: Plots data collected at thermal chamber
%*/
%% Information

% Agilent E3647A Power supply
% Fluke 189 chamber temp sensor
% Fluke 52-II CPU temp sensor

% Max temp 50 C
% Min temp 0 C
% Max voltage 4.2V
% Min voltage 3.3V

%% Setup
close all

%% Plotting Data
figure(1),

t1 = 0:5:60;
chamber = [20.4, -1.4, -0.8, -0.6, -0.4, -0.3, -0.3, -0.1, -0.1, -0.2, -0.2, -0.1, -0.2];
cpu = [20.6, 1.5, 1.1, 1.0, 0.9, 0.8, 0.9, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8];

subplot(2,2,1)
plot(t1, chamber, 'b', t1, cpu, 'r', 'linewidth', 2)
grid on
xlabel('t (min)')
ylabel('Temp (\circC)')
title('Low Voltage, Cold')
legend('Chamber', 'Transmitter')
ylim([-5 25])

t2 = 0:5:30;
chamber = [-0.2, -0.2, -0.1, -0.2, -0.2, -0.1, -0.2];
cpu = [0.9, 0.8, 0.9, 0.8, 0.8, 0.8, 0.8];

subplot(2,2,2)
plot(t2, chamber, 'b', t2, cpu, 'r', 'linewidth', 2)
grid on
xlabel('t (min)')
ylabel('Temp (\circC)')
title('High Voltage, Cold')
legend('Chamber', 'Transmitter')
ylim([-5 25])

t3 = 0:5:30;
chamber = [50.3, 50.4, 50.4, 50.2, 50.4, 50.4, 50.3];
cpu = [49.0, 49.7, 49.9, 50.0, 50.0, 50.1, 50.1];

subplot(2,2,3)
plot(t3, chamber, 'b', t3, cpu, 'r', 'linewidth', 2)
grid on
xlabel('t (min)')
ylabel('Temp (\circC)')
title('Low Voltage, Hot')
legend('Chamber', 'Transmitter')
ylim([25 60])

t4 = 0:5:30;
chamber = [50.4, 50.4, 50.3, 50.4, 50.4, 50.4, 50.4];
cpu = [50.2, 50.2, 50.3, 50.3, 50.2, 50.4, 50.4];

subplot(2,2,4)
plot(t4, chamber, 'b', t4, cpu, 'r', 'linewidth', 2)
grid on
xlabel('t (min)')
ylabel('Temp (\circC)')
title('High Voltage, Hot')
legend('Chamber', 'Transmitter')
ylim([25 60])
