% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV1_Attraction_Repulsion/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV2_Pairwise_Neighbour_Ref/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV3_Rigid_Neighbour_Reg/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV4_Assignment_Registration/node_logs/';
dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/trials_propulsion_scenario4/square_lattice/BHV2_Pairwise_Neighbour_Ref/propulse/log_propulse_1/node_logs/';

file_names = strcat(dir_name, '*.csv');
files = dir(file_names);

time_series = [];
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    csv = csvread(file_name, 2, 0);
    start_row = find(csv(:,22), 1, 'first');
    consum_total = csv(start_row:end, 13);
    time = csv(start_row:end, 2);
    time = time - time(1);
    consum_total_ts = timeseries(consum_total, time);
    consum_total_ts = resample(consum_total_ts, 0:1:round(time(end)));
    time_series = [time_series, consum_total_ts];
end

max_len = 0;
for i = 1:size(time_series, 2)
    if (size(time_series(i).Data,1) > max_len)
        max_len = size(time_series(i).Data,1);
        max_time = time_series(i).Time(end);
    end
end

for i = 1:size(time_series, 2)
    time_series(i) = resample(time_series(i), 0:1:max_time);
end

sum_ts_data = time_series(1).Data;
for i = 2:size(time_series, 2)
    sum_ts_data = sum_ts_data + time_series(i).Data;
end

avg_ts_data = sum_ts_data./size(time_series, 2);
mean_energy_ts = timeseries(avg_ts_data, 0:1:max_time);

%plot(time_series(1).Data);
joined = time_series(1).Data';
hold on;
for i = 2:size(time_series, 2)
    %plot(time_series(i).Data);
    joined = cat(1, joined, time_series(i).Data');
end
env_max = max(joined, [], 1);
env_min = min(joined, [], 1);
plot(0:1:max_time, env_max, 'r--');
plot(0:1:max_time, env_min, 'r--');
plot(mean_energy_ts, 'r');

title('');
xlabel('Mission Time (s)');
ylabel('Mean Energy Expenditure (Wh)');

save(strcat(dir_name,'time_v_mean_energy'),'mean_energy_ts', 'time_series')