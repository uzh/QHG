
% imports 'estimated_arrival_time.mat', and finds the arrival times at
% the positions (longitude and latitude) specified in the table lat_long

clear

arrival_time_file = ['estimated_arrival_time.mat'];
load(arrival_time_file,'arrival_time')


% table of latitude and longitude of archeological sites

lat_long  = [ 44.3         -88.4 ; ... % 14.8 -- 14.2    Schaefer, Hebior sites (Wisconsin)              1.
             42.8         -120.7; ...  % 14.5 -- 14        Paisley Caves (Oregon)                        5. 
             40.3         -79.5; ...     %  14.5 -- 14 *    Meadowcroft Shelter (Pennsylvania)       8.
             31.0		 -96.5; ... % 15.5 -- 13.2 *		Buttermilk Creek, Friedkin site (Texas)     7.
           -47.5		 -67.9; ...  % 13.1* -- 12.9*		Piedra Museo (Argentina)                    3. 
           -33.7  	     -71.3; ...  % 13.1*			Quebrada Santa Julia (Chille)                   4.
           -40.5         -72.9; ...  %14.6 -- 14.2 *		Monte Verde (Chile)                         6. 
           -52.0         -70.1; ... %	13.1* -- 12.9* 		Fell’s Cave (Chille )                       2.      
           30.2          -84.0 ]  % 14.5 Page-Ladson (Florida)
 
selected_arrival_time = [];

for i = 1:length(lat_long)
    latitude = lat_long(i,1);
    longitude = lat_long(i,2);
    i
    if i==1  | i ==6 
        x = find(arrival_time(:,2)>=(longitude-0.2) & arrival_time(:,2)<= (longitude+0.2) ...
            & arrival_time(:,3)>= (latitude-0.2) & arrival_time(:,3)<=(latitude+0.2))
    end
    if i==8| i == 7 |i== 4 | i == 5
        x = find(arrival_time(:,2)>=(longitude-0.3) & arrival_time(:,2)<= (longitude+0.3) ...
            & arrival_time(:,3)>= (latitude-0.3) & arrival_time(:,3)<=(latitude+0.3))
    end
    if i ==3 
          x = find(arrival_time(:,2)>=(longitude-0.2) & arrival_time(:,2)<= (longitude+0.2) ...
            & arrival_time(:,3)>= (latitude-0.3) & arrival_time(:,3)<=(latitude+0.3))
    end       
    if i == 2
          x = find(arrival_time(:,2)>=(longitude-0.2) & arrival_time(:,2)<= (longitude+0.2) ...
            & arrival_time(:,3)>= (latitude-0.2) & arrival_time(:,3)<=(latitude+0.3))  
    end
    if i==9
       x = find(arrival_time(:,2)>=(longitude-0.3) & arrival_time(:,2)<= (longitude+0.3) ...
            & arrival_time(:,3)>= (latitude-0.3) & arrival_time(:,3)<=(latitude+0.2))
    end
    
    selected_arrival_time = [selected_arrival_time; arrival_time(x,:)];
end


%------------- estimated arrival times 
time_shift = 18000

selected_arrival_time(:,1) = 5*selected_arrival_time(:,1) - time_shift;
%selected_arrival_time(:,1) = round(selected_arrival_time(:,1)/1000,1);
