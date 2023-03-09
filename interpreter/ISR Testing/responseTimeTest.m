%/*
%  FILE: responseTimeTest.m
%  VERSION: 1.0.0
%  TEST DATE: 7 March 2023
%  DATE: 7 March 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION:
%*/

%% Load Data
cd(fileparts(matlab.desktop.editor.getActiveFilename));
close all 
clc
load('inttolora.mat')

%% Plots
t = inttolora(:, 1); 
ch_interrupt = inttolora(:, 2);
ch_sending = inttolora(:, 3);

% filter out noise, this is digital
ch_interrupt(ch_interrupt < 2.5) = 0;
ch_interrupt(ch_interrupt >= 2.5) = 3.3;
ch_sending(ch_sending < 2.5) = 0;
ch_sending(ch_sending >= 2.5) = 3.3;

figure,

%subplot(2,1,1)
grid on
hold on
plot(1000000*t, ch_interrupt, 'r'); % convert to microseconds
plot(1000000*t, ch_sending, 'b'); % convert to microseconds
xlabel('t (us)')
ylabel('Status')
xlim([-50 250])
ylim([-1 6])
title('Interrupt-to-Response', 'Interpreter', 'none')

text(0, 3.45, 'Accel Senses', 'Rotation', 45, 'HorizontalAlignment', 'left')
text(85.2, 3.45, 'MCU Awakens', 'Rotation', 45, 'HorizontalAlignment', 'left')
text(99, .1, 'Interrupt Serviced', 'Rotation', 45, 'HorizontalAlignment', 'left')
text(160, 3.5, '---------Begin Processing---------', 'HorizontalAlignment', 'center')
%%
% subplot(2,1,2)
% grid on
% hold on
% plot(1000*t, ch_interrupt, 'r'); % convert to milliseconds
% plot(1000*t, ch_sending, 'b'); % convert to milliseconds
% xlabel('t (ms)')
% ylabel('Status')
% xlim([-5 100])
% ylim([-1 5])
% title('Interrupt-to-Send Time','Interpreter','none')