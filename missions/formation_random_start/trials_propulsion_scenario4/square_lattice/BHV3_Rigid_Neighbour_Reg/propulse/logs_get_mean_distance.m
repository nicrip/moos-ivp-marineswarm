% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV1_Attraction_Repulsion/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV2_Pairwise_Neighbour_Ref/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV3_Rigid_Neighbour_Reg/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV4_Assignment_Registration/node_logs/';
dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/trials_propulsion_scenario4/square_lattice/BHV3_Rigid_Neighbour_Reg/propulse/log_propulse_2/node_logs/';

file_names = strcat(dir_name, '*.csv');
files = dir(file_names);

time_series_x = [];
time_series_y = [];
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    csv = csvread(file_name, 2, 0);
    start_row = find(csv(:,22), 1, 'first');
    x_pos = csv(start_row:end, 3);
    y_pos = csv(start_row:end, 4);
    time = csv(start_row:end, 2);
    time = time - time(1);
    x_ts = timeseries(x_pos, time);
    x_ts = resample(x_ts, 0:1:round(time(end)));
    y_ts = timeseries(y_pos, time);
    y_ts = resample(y_ts, 0:1:round(time(end)));
    time_series_x = [time_series_x, x_ts];
    time_series_y = [time_series_y, y_ts];
end

max_len = 0;
for i = 1:size(time_series_x, 2)
    if (size(time_series_x(i).Data,1) > max_len)
        max_len = size(time_series_x(i).Data,1);
        max_time = time_series_x(i).Time(end);
    end
end

max_time = 18000;
for i = 1:size(time_series_x, 2)
    time_series_x(i) = resample(time_series_x(i), 0:1:max_time);
    time_series_y(i) = resample(time_series_y(i), 0:1:max_time);
end

sum_ts_data_x = time_series_x(1).Data;
for i = 2:size(time_series_x, 2)
    sum_ts_data_x = sum_ts_data_x + time_series_x(i).Data;
end

sum_ts_data_y = time_series_y(1).Data;
for i = 2:size(time_series_y, 2)
    sum_ts_data_y = sum_ts_data_y + time_series_y(i).Data;
end

avg_ts_data_x = sum_ts_data_x./size(time_series_x, 2);
mean_x_ts = timeseries(avg_ts_data_x, 0:1:max_time);

avg_ts_data_y = sum_ts_data_y./size(time_series_y, 2);
mean_y_ts = timeseries(avg_ts_data_y, 0:1:max_time);

cmap = hsv(size(files,1));
figure; hold on; axis equal;
count = 1;
for i = 1:size(time_series_x, 2)
    plot(time_series_x(i).Data, time_series_y(i).Data, 'Color', cmap(count,:), 'LineWidth', 2);
    plot(time_series_x(i).Data(end), time_series_y(i).Data(end), 'o', 'MarkerSize', 10, 'MarkerEdgeColor', 'r', 'MarkerFaceColor', 'r', 'LineWidth', 2);
    plot(time_series_x(i).Data(1), time_series_y(i).Data(1), 'x', 'MarkerSize', 12, 'MarkerEdgeColor', 'k', 'LineWidth', 2);
    count = count+1;
end
plot(mean_x_ts.Data, mean_y_ts.Data, '--', 'Color', [0,0,0], 'LineWidth', 3);
xlabel('X Position (m)');
ylabel('Y Position (m)');

dist_travelled = [0; cumsum(sqrt(diff(mean_x_ts.Data(:)).^2 + diff(mean_y_ts.Data(:)).^2))];

%%% speedup %%%
for i = 1:size(time_series_x, 2)
    time_series_x(i) = resample(time_series_x(i), 0:3:max_time);
    time_series_y(i) = resample(time_series_y(i), 0:3:max_time);
end
%%%%%%%%%%%%%%%

X = [];
Y = [];
for i = 1:size(time_series_x, 2);
    X = [X, time_series_x(i).Data];
    Y = [Y, time_series_y(i).Data];
end
figure('units','normalized','outerposition',[0 0 1 1]); axis equal;
disp('Press Key to Start Animation')
k = waitforbuttonpress;
multicomet(X, Y);

% save(strcat(dir_name,'time_v_mean_energy'),'mean_energy_ts', 'time_series')