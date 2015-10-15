% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV1_Attraction_Repulsion/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV2_Pairwise_Neighbour_Ref/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV3_Rigid_Neighbour_Reg/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV4_Assignment_Registration/node_logs/';
dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/trials_propulsion_scenario4/square_lattice/BHV2_Pairwise_Neighbour_Ref/propulse/';

file_names = strcat(dir_name, 'time_v_mean_energy*');
files = dir(file_names);
mean_energies = [];
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    mean_energies = [mean_energies, load(file_name)];
end

max_len = 0;
for i = 1:size(mean_energies, 2)
    if (size(mean_energies(i).mean_energy_ts.Data,1) > max_len)
        max_len = size(mean_energies(i).mean_energy_ts.Data,1);
        max_time = mean_energies(i).mean_energy_ts.Time(end);
    end
end

for i = 1:size(mean_energies, 2)
    mean_energies(i).mean_energy_ts = resample(mean_energies(i).mean_energy_ts, 0:1:max_time);
end

sum_ts_data = mean_energies(1).mean_energy_ts.Data;
for i = 2:size(mean_energies, 2)
    sum_ts_data = sum_ts_data + mean_energies(i).mean_energy_ts.Data;
end

avg_ts_data = sum_ts_data./size(mean_energies, 2);
trial_averaged_energy_ts = timeseries(avg_ts_data, 0:1:max_time);

%plot(mean_energies(1).mean_energy_ts);
joined = mean_energies(1).mean_energy_ts.Data';
hold on;
for i = 2:size(mean_energies, 2)
    %plot(mean_energies(i).mean_energy_ts);
    joined = cat(1, joined, mean_energies(i).mean_energy_ts.Data');
end
env_max = max(joined, [], 1);
env_min = min(joined, [], 1);
plot(0:1:max_time, env_max, 'k--');
plot(0:1:max_time, env_min, 'k--');
plot(trial_averaged_energy_ts, 'r');
%title(sprintf('Trial-Averaged Mean Energy Expenditure of 25 AUVs During\nHexagonal Formation Construction with no Ocean Current'));
xlabel('Mission Time (s)');
ylabel('Trial-Averaged Mean Energy Expenditure (Wh)');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

file_names = strcat(dir_name, 'time_v_form_qual*');
files = dir(file_names);
qualities = [];
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    load(file_name)
    qualities = [qualities, quality_ts];
end

max_len = 0;
for i = 1:size(qualities, 2)
    if (size(qualities(i).Data,1) > max_len)
        max_len = size(qualities(i).Data,1);
        max_time = qualities(i).Time(end);
    end
end

for i = 1:size(qualities, 2)
    qualities(i) = resample(qualities(i), 0:1:max_time);
end

sum_ts_data = qualities(1).Data;
for i = 2:size(qualities, 2)
    sum_ts_data = sum_ts_data + qualities(i).Data;
end

avg_ts_data = sum_ts_data./size(qualities, 2);
trial_averaged_quality_ts = timeseries(avg_ts_data, 0:1:max_time);

figure;
%plot(qualities(1));
joined = qualities(1).Data';
hold on;
for i = 2:size(qualities, 2)
    %plot(qualities(i));
    joined = cat(1, joined, qualities(i).Data');
end
env_max = max(joined, [], 1);
env_min = min(joined, [], 1);
plot(0:1:max_time, env_max, 'k--');
plot(0:1:max_time, env_min, 'k--');
plot(trial_averaged_quality_ts, 'r');
%title(sprintf('Trial-Averaged Formation Quality Metric of 25 AUVs During\nHexagonal Formation Construction with no Ocean Current'));
xlabel('Mission Time (s)');
ylabel('Trial-Averaged Formation Quality Metric');