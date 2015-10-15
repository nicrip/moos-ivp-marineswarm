% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV1_Attraction_Repulsion/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV2_Pairwise_Neighbour_Ref/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV3_Rigid_Neighbour_Reg/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV4_Assignment_Registration/node_logs/';
dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/trials_propulsion_scenario4/square_lattice/BHV2_Pairwise_Neighbour_Ref/propulse/log_propulse_2/node_logs/';

file_names = strcat(dir_name, '*.csv');
files = dir(file_names);

time_series_thrust = [];
time_series_drift = [];
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    csv = csvread(file_name, 2, 0);
    start_row = find(csv(:,22), 1, 'first');
    time_thrust = csv(start_row:end, 16);
    time_drift = csv(start_row:end, 15);
    time = csv(start_row:end, 2);
    time = time - time(1);
    time_thrust_ts = timeseries(time_thrust, time);
    time_thrust_ts = resample(time_thrust_ts, 0:1:round(time(end)));
    time_drift_ts = timeseries(time_drift, time);
    time_drift_ts = resample(time_drift_ts, 0:1:round(time(end)));
    time_series_thrust = [time_series_thrust, time_thrust_ts];
    time_series_drift = [time_series_drift, time_drift_ts];
end

max_len = 0;
for i = 1:size(time_series_thrust, 2)
    if (size(time_series_thrust(i).Data,1) > max_len)
        max_len = size(time_series_thrust(i).Data,1);
        max_time = time_series_thrust(i).Time(end);
    end
end

time_series_total = [];
for i = 1:size(time_series_thrust, 2)
    time_series_thrust(i) = resample(time_series_thrust(i), 0:1:max_time);
    time_series_drift(i) = resample(time_series_drift(i), 0:1:max_time);
    time_series_total = [time_series_total, time_series_thrust(i)+time_series_drift(i)];
    time_series_thrust(i) = time_series_thrust(i)./time_series_total(i);
    time_series_drift(i) = time_series_drift(i)./time_series_total(i);
end

sum_ts_data_thrust = time_series_thrust(1).Data;
sum_ts_data_drift = time_series_drift(1).Data;
for i = 2:size(time_series_thrust, 2)
    sum_ts_data_thrust = sum_ts_data_thrust + time_series_thrust(i).Data;
    sum_ts_data_drift = sum_ts_data_drift + time_series_drift(i).Data;
end

avg_ts_data_thrust = sum_ts_data_thrust./size(time_series_thrust, 2);
mean_time_thrust_ts = timeseries(avg_ts_data_thrust, 0:1:max_time);
avg_ts_data_drift = sum_ts_data_drift./size(time_series_thrust, 2);
mean_time_drift_ts = timeseries(avg_ts_data_drift, 0:1:max_time);

%plot(time_series_thrust(1).Data, 'r--');
joined = time_series_thrust(1).Data';
hold on;
%plot(time_series_drift(1).Data, 'b--');
for i = 2:size(time_series_thrust, 2)
    %plot(time_series_thrust(i).Data, 'r--');
    joined = cat(1, joined, time_series_thrust(i).Data');
   % plot(time_series_drift(i).Data, 'b--');
end
env_max = max(joined, [], 1);
env_min = min(joined, [], 1);
plot(0:1:max_time, env_max, 'r--', 'LineWidth', 0.5);
plot(0:1:max_time, env_min, 'r--', 'LineWidth', 0.5);
plot(mean_time_thrust_ts, 'r', 'LineWidth', 1.5);
title('');
xlabel('Mission Time (s)');
ylabel('(Mean Time Thrusting)/(Total Mission Time)');
xlim([0 18000]);
%plot(mean_time_drift_ts, 'b');

%save(strcat(dir_name,'time_v_mean_energy'),'mean_energy_ts', 'time_series')