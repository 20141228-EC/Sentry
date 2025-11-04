clc;
clear;

data = readtable("chassis_lb.csv");

power_other = 5.157132992481203 / 4 * 3;

x = data.matlab_x;
y = data.matlab_y;
z = data.matlab_z;
p = data.matlab_p;  % 单电机静态

% 检查数据长度是否一致
if length(x) ~= length(y) || length(x) ~= length(z) || length(x) ~= length(p)
    error('所有数据向量的长度必须一致');
end

% 创建匿名函数，描述拟合目标
fitfunc = @(params) abs(params(1)*x + params(2)*y + params(3)*z + params(4) + power_other) - p;

% 初始参数估计
initial_params = [0, 0, 0, 0];

% 使用非线性最小二乘法求解
options = optimoptions('lsqnonlin', 'Display', 'iter');
params = lsqnonlin(fitfunc, initial_params, [], [], options);

% 提取拟合参数
a = params(1);
b = params(2);
c = params(3);
d = params(4);

% 计算拟合值
p_fit = abs(a*x + b*y + c*z + d + power_other);

% 计算拟合效果R平方
SStot = sum((p - mean(p)).^2);
SSres = sum((p - p_fit).^2);
R_squared = 1 - SSres / SStot;

% 输出拟合参数和R平方
fprintf('拟合参数:\n');
fprintf('a = %.8f\n', a);
fprintf('b = %.8f\n', b);
fprintf('c = %.8f\n', c);
fprintf('拟合效果 R² = %.8f\n', R_squared);