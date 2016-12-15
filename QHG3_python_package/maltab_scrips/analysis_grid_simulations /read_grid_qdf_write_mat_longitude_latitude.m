
% read .qdf files and calculate the speed of the wave that propagates along x-Axis.
% the information about the position of individuals  form .qdf files is save into .mat files

clear

d_start = 100;    % start analysis at simulation time 100 
d_finish=1400;    %  end analysis at simulation time 1400
delta_t = 10;    % the frequency at which the .qdf were saved

% parameters of the simulation
K = 10        % carrying capacity K used in the simulation
number_along_y = 10    % how many cells are filed along y axis at t=0, 
                       % i.e, here 10 cells are filed. Note, that for a 2 dim case                    number_along_y should be 100

half_capacity_position_line = []  % position of the K/2 for all times
time_step_line = []
    
% longitude - x axis
longitude_file = ['write_Longitude_grid_1000x100.mat']
load(longitude_file,'longitude')

% latitude - y axis
latitude_file = ['write_Latitude_grid_1000x100.mat']
load(latitude_file,'latitude')

% settings for x axis and y axis
x_length = 1001;
y_length = 101;
values_x = 1:1:x_length;

% colors for the plot
d_ende_cc = d_ende/delta_t +7;
cc=hsv(d_ende_cc);

figure
hold on

for d=d_start:delta_t:d_finish


time_step = d
 
% define the identifier of the data file

if time_step < 100 && time_step >= 10
    textFilename_density = ['output__pop-Dummy__0000' num2str(time_step) '.qdf'];
    textFilename_density_write = ['write_count_output__pop-Dummy__0000' num2str(time_step) '.mat'];
end
if time_step < 1000 && time_step >= 100
    textFilename_density = ['output__pop-Dummy__000' num2str(time_step) '.qdf'];
    textFilename_density_write= ['write_count_output__pop-Dummy__000' num2str(time_step) '.mat'];
end
if time_step >= 1000 && time_step < 10000
    textFilename_density = ['output__pop-Dummy__00' num2str(time_step) '.qdf'];
    textFilename_density_write = ['write_count_output__pop-Dummy__00' num2str(time_step) '.mat'];
end
if time_step >= 10000
    textFilename_density = ['output__pop-Dummy__0' num2str(time_step) '.qdf'];
    textFilename_density_write = ['write_count_output__pop-Dummy__0' num2str(time_step) '.mat'];
end


% read data file
% LifeState / CellIdx / CellID /AgentsID / BirthTime /Gender/ Age

% reading the .qdf (i.e., hdf file)
data = h5read(textFilename_density,'/Populations/Dummy/AgentDataSet');
density = data.('CellIdx');

% count - matrix that includes the number of individuals at each x,y position of the grid
count = zeros(y_length,x_length);

number_individuals = length(density)   % number_individuals is the total number of individuals a a certain time step

for i = 1:number_individuals
    
    cell_id = density(i);  % find the cell id  
    current_x = longitude(cell_id+1);   % find longitude, +1 since in c the arrays start from 0
    current_y = latitude(cell_id+1);   % find latitude
    
    count(current_y+1, current_x+1) = count(current_y+1, current_x+1) + 1;  
end

write_count = [];
     
save(textFilename_density_write,'count');

% sum the values along the y axis
density_along_x = sum(count);
density_along_x = density_along_x/number_along_y;


%----------- calculate the value which is closest to K/2 ---------------------------
% method: calculate the difference density_along_x - K/2. then find the
% minimum value of the difference with the function min.

% flip the wave: the min function starts to search from the beginning of
% array. In order to avoid that data from the bulk of the wave (usually N~K, 
% the flat part of the wave) go into the estimated min, one flips the wave.
% So the estimation of min starts now from the wave tip which is step.

density_along_x_fliped = fliplr(density_along_x);
values_x_fliped = fliplr(values_x);

[half_K_value half_K_index] = min(abs(density_along_x_fliped - K/2)); % find the minimum
half_capacity_position = values_x_fliped(half_K_index);       % find the position                                              

if half_capacity_position > 10

half_capacity_position_line = [half_capacity_position_line, half_capacity_position];
time_step_line = [time_step_line, time_step];

end


%plot density along x
if  mod(time_step, 100) == 0
 plot(values_x,density_along_x,'color',cc(time_step/delta_t,:),...
       'markers',10,'LineWidth',2)
   
end


end

function_fit = polyfit(time_step_line,half_capacity_position_line,1)
speed = function_fit(1)

hold off

% plot position of K/2 versus time
figure
plot(time_step_line, half_capacity_position_line,'*g','markers',10,'LineWidth',2)
xlabel('time')
ylabel('position of min(N-K/2)')
