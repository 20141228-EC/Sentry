clc;
clear;

% 导入数据
data = readtable('static.csv');

chassis_power = data.matlab_p;

power_all = mean(chassis_power);
power = power_all / 4;
