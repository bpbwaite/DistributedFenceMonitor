%/*
%  FILE: severity_analysis.m
%  VERSION: 1.0.1
%  TEST DATE: 14 April 2023
%  DATE: 15 April 2023
%  PROJECT: Distributed Fence Monitor Capstone
%  AUTHORS: Briellyn Braithwaite
%  DESCRIPTION: Collected fence climbing data using dfm_recv.m. Processing
%               and producing graphs in this file.
%*/

% Data from a variety of people and methods, including sneaky and
% on-the-pole

%%
clearvars
close all
format longg

%% Between units
trial = 1:4;
u1 = [7 5 6];
u2 = [3 4 5];

% Build the fence
figure, hold on
xlim([-3.5 3.5])
xticks(sort([-3.5:1:3.5]))
ylim([0 2])
yticks([0 0.25 1.75 2])
grid on
% place the nodes
plot([-2 2], [1 1], 'bo', 'linewidth', 8)
text(-2, 0.9, 'Node 1', 'HorizontalAlignment', 'center')
text(2, 0.9, 'Node 2', 'HorizontalAlignment', 'center')
% place the Intruder
plot(0, 0.7, 'ro', 'linewidth', 10)
text(0, 0.6, 'Intruder', 'HorizontalAlignment', 'center')
% place data
text(-2, 1.5, ["Severity" string(u1)], 'HorizontalAlignment', 'center')
text(2, 1.5, ["Severity" string(u2)], 'HorizontalAlignment', 'center')
% formatting
xticklabels([])
yticklabels([])
title('Intruder Climbs Between Nodes')

id = 1;
for n=-3:3
    text(n, 0.2, num2str(id), 'HorizontalAlignment', 'center')
    id = id + 1;
end
xlabel('Fence Panel')
%% On top of unit
trial = 1:3;
u1 = [15 15 15];
u2 = [0 0 0];

% Build the fence
figure, hold on
xlim([-3.5 3.5])
xticks(sort([-3.5:1:3.5]))
ylim([0 2])
yticks([0 0.25 1.75 2])
grid on
% place the nodes
plot([-2 2], [1 1], 'bo', 'linewidth', 8)
text(-2, 0.9, 'Node 1', 'HorizontalAlignment', 'center')
text(2, 0.9, 'Node 2', 'HorizontalAlignment', 'center')
% place the Intruder
plot(-2, 0.7, 'ro', 'linewidth', 10)
text(-2, 0.6, 'Intruder', 'HorizontalAlignment', 'center')
% place data
text(-2, 1.5, ["Severity" string(u1)], 'HorizontalAlignment', 'center')
text(2, 1.5, ["Severity" string(u2)], 'HorizontalAlignment', 'center')
% formatting
xticklabels([])
yticklabels([])
title('Intruder Climbs Directly on Node')

id = 1;
for n=-3:3
    text(n, 0.2, num2str(id), 'HorizontalAlignment', 'center')
    id = id + 1;
end
xlabel('Fence Panel')
%% One-away from 1, between
trial = 1:3;
u1 = [7 11 12];
u2 = [0 3 2];

% Build the fence
figure, hold on
xlim([-3.5 3.5])
xticks(sort([-3.5:1:3.5]))
ylim([0 2])
yticks([0 0.25 1.75 2])
grid on
% place the nodes
plot([-2 2], [1 1], 'bo', 'linewidth', 8)
text(-2, 0.9, 'Node 1', 'HorizontalAlignment', 'center')
text(2, 0.9, 'Node 2', 'HorizontalAlignment', 'center')
% place the Intruder
plot(-1, 0.7, 'ro', 'linewidth', 10)
text(-1, 0.6, 'Intruder', 'HorizontalAlignment', 'center')
% place data
text(-2, 1.5, ["Severity" string(u1)], 'HorizontalAlignment', 'center')
text(2, 1.5, ["Severity" string(u2)], 'HorizontalAlignment', 'center')
% formatting
xticklabels([])
yticklabels([])
title('Intruder Climbs Closer to Node 1')

id = 1;
for n=-3:3
    text(n, 0.2, num2str(id), 'HorizontalAlignment', 'center')
    id = id + 1;
end
xlabel('Fence Panel')
%% One-away from 2, between
trial = 1:3;
u1 = [3 6 0];
u2 = [4 5 9];

% Build the fence
figure, hold on
xlim([-3.5 3.5])
xticks(sort([-3.5:1:3.5]))
ylim([0 2])
yticks([0 0.25 1.75 2])
grid on
% place the nodes
plot([-2 2], [1 1], 'bo', 'linewidth', 8)
text(-2, 0.9, 'Node 1', 'HorizontalAlignment', 'center')
text(2, 0.9, 'Node 2', 'HorizontalAlignment', 'center')
% place the Intruder
plot(1, 0.7, 'ro', 'linewidth', 10)
text(1, 0.6, 'Intruder', 'HorizontalAlignment', 'center')
% place data
text(-2, 1.5, ["Severity" string(u1)], 'HorizontalAlignment', 'center')
text(2, 1.5, ["Severity" string(u2)], 'HorizontalAlignment', 'center')
% formatting
xticklabels([])
yticklabels([])
title('Intruder Climbs Closer to Node 2')

id = 1;
for n=-3:3
    text(n, 0.2, num2str(id), 'HorizontalAlignment', 'center')
    id = id + 1;
end
xlabel('Fence Panel')