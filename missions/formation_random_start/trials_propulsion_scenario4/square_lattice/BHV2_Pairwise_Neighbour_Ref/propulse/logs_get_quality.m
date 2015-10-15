% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV1_Attraction_Repulsion/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV2_Pairwise_Neighbour_Ref/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV3_Rigid_Neighbour_Reg/node_logs/';
% dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/hexagonal_lattice/BHV4_Assignment_Registration/node_logs/';
dir_name = '/home/rypkema/Workspace/moos-ivp-moossafir/missions/formation_random/trials_propulsion_scenario4/square_lattice/BHV2_Pairwise_Neighbour_Ref/propulse/log_propulse_1/';
file_names = strcat(dir_name, '*.csv');
files = dir(file_names);

% get the launch row
start_row = 0;
for file = files'
    disp(['CURRENT FILE: ', file.name]);
    file_name = strcat(dir_name, file.name);
    csv = csvread(file_name, 2, 0);
    start_row = find(csv(:,5), 1, 'first');
end

time = csv(start_row:end, 2);
time = time - time(1);
form_qual = csv(start_row:end, 4);
quality_ts = timeseries(form_qual, time);
quality_ts = resample(quality_ts, 0:1:round(time(end)));

plot(quality_ts)

save(strcat(dir_name,'time_v_form_qual'),'quality_ts')