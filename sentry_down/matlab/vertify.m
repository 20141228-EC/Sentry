clc;
clear;

% 读取数据
data = readtable("vertify.csv");

% 获取预测值和实际值
predictions = data.power_now;
actual = data.chassis_power;

% 计算均方误差 (MSE)
mse = mean((predictions - actual).^2);

% 输出均方误差
fprintf('算法的均方误差 (MSE) 为: %.4f\n', mse);

% 定义一个容差范围
tolerance = 10; % ± 10w

% 计算在容差范围内的正确预测数量
correct_predictions = sum(abs(predictions - actual) <= tolerance);

% 计算总数据量
total_predictions = length(actual);

% 计算准确率
accuracy = correct_predictions / total_predictions;

% 输出准确率
fprintf('算法的准确率为: %.2f%%\n', accuracy * 100);

% 绘制图像
figure;
hold on;

% 绘制实际值
plot(actual, '-o', 'DisplayName', '实际值');

% 绘制预测值
plot(predictions, '-x', 'DisplayName', '预测值');

% 添加图例
legend;

% 添加标题和标签
title('预测值与实际值对比');
xlabel('样本编号');
ylabel('功率值');

% 显示图像
hold off;