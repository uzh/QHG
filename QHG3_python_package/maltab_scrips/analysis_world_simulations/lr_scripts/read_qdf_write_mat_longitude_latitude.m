
% reads .qdf populations' file and writes arrival time at a cell and the 
% corresponding longitude, latitude, and cell's ID to the estimated_arrival_time.mat


clear

d_anfang = 18000;  % simulation step at which the data should be loaded
d_ende=18000;    % d_anfang should be equalt to d_ende 
delta_t = 100;   % just some dummy value


 
longitude_file = ['write_Longitude_ieq_128.mat']
load(longitude_file,'longitude')


latitude_file = ['write_Latitude_ieq_128.mat']
load(latitude_file,'latitude')



for d=d_anfang:delta_t:d_ende

 
    
 time_step = d
 
% define the identifier of the data file
if time_step <= 10
    textFilename_density = ['output__pop-AltMoverFK_M_00000' num2str(time_step) '.qdf'];
    textFilename_density_write = ['output__pop-AltMoverFK_M_00000' num2str(time_step) '.mat'];
end


if time_step < 100 && time_step >= 10
    textFilename_density = ['output__pop-AltMoverFK_M_0000' num2str(time_step) '.qdf'];
    textFilename_density_write = ['output__pop-AltMoverFK_M_0000' num2str(time_step) '.mat'];
end
if time_step < 1000 && time_step >= 100
    textFilename_density = ['output__pop-AltMoverFK_M_000' num2str(time_step) '.qdf'];
    textFilename_density_write= ['output__pop-AltMoverFK_M_000' num2str(time_step) '.mat'];
end
if time_step >= 1000 && time_step < 10000
    textFilename_density = ['output__pop-AltMoverFK_M_00' num2str(time_step) '.qdf'];
    textFilename_density_write = ['output__pop-AltMoverFK_M_00' num2str(time_step) '.mat'];
end
if time_step >= 10000
    textFilename_density = ['output__pop-AltMoverFK_M_0' num2str(time_step) '.qdf'];
    textFilename_density_write = ['output__pop-AltMoverFK_M_0' num2str(time_step) '.mat'];
end


% read data file
% LifeState / CellIdx / CellID /AgentsID / BirthTime /Gender/ Age

%data = h5read(textFilename_density,'/Populations/Dummy/AgentDataSet');
data_dist = h5read(textFilename_density,'/MoveStatistics/Dist');
data_time = h5read(textFilename_density,'/MoveStatistics/Time');

data_dist_non_zero =data_dist(find(data_dist>0));
my_matrix_dist = [find(data_dist>0), data_dist_non_zero];

data_time_non_zero=data_time(find(data_time>0));
my_matrix_time = [find(data_time>0), data_time_non_zero];
positions_time = find(data_time>0);

non_equal_dist = []; % cell, distance
non_equal_time = [];
find_values = [];

time_non_zero= [];
dist_non_zero =[];

for i = 1:length(data_dist_non_zero)
    wert_dist = my_matrix_dist(i,1);
    my_find = find(positions_time==wert_dist);
    if length(my_find) == 0
      non_equal_dist = [non_equal_dist, wert_dist];
    end
    
end

arrival_time =  [];  % time, longitude (y), latitude (x)
for i = 1:length(my_matrix_time) 
    id = my_matrix_time(i,1);
    time = my_matrix_time(i,2);
    current_longitude = longitude(id);
    current_latitude = latitude(id);
    if current_longitude < -20 && current_longitude > -165 %&& time > 1400
        
     % in the output .mat files, the arrival times in the Americas, the logitude, 
      % latitude of this cell as well as the cell's id are saved. 
       arrival_time = [arrival_time; time, current_longitude, current_latitude, id];
    end
    
end


end

save('estimated_arrival_time.mat', 'arrival_time')
