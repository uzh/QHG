'''
Created on Oct 20, 2014

@author: natalie
'''

from __future__ import division 
from random import random
import numpy
from pylab import *


def function(ratio_value=0.7/0.8,name_of_the_file='file_name.txt'):
    # begin 1.layer
    #K = 10.0;
    #N0 = K;
    out = open(name_of_the_file,'w')

    K_line = [10, 50, 100, 500]   #
        

    time_initial  = 1000;
    #time = 5000;
    time_start = 1;

    #ratio = theta/b0
    ratio = ratio_value #0.7/0.8 #10**(-1)  # look at lower ratio!!
    d0_ratio = 0 #(0.4/0.8) #; d0_initial/b0_initial

    #b0_line = [1.0, 0.9, 0.8,0.7, 0.6, 0.5, 0.1,0.08, 0.04, 0.01]; 
    #b0_line= [1, 0.9, 0.8, 0.7, 0.6, 0.5,0.4, 0.3, 0.2,0.14, 0.1, 0.08, 0.06 ,  0.04, 0.03,0.02, 0.014 ,0.01, 0.008,0.001]
    b0_line = [0.3]
    b0_value = b0_line[0]
    theta_value = ratio_value*b0_value

    #b0_line = [0.8,0.7, 0.6, 0.5, 0.1,0.08, 0.04, 0.01];
    d0_line = list(d0_ratio*array(b0_line));  # d0_initial/b0_initial
    d0_value = d0_line[0]
    theta_line = list(ratio*array(b0_line))
    
    several_means = []
    several_variances = []
  
    several_means_birth_probability = []
    several_means_death_probability = []

    #begin 1. layer , do for every K value in the array K_line
    for item in range(len(K_line)):
    
        b0 = b0_value # b0_line[item]
        theta = theta_value #theta_line[item]
         
        d0 = d0_value   # d0 = 0
        time =  time_initial #time_line[item]
    
        K = K_line[item]
        N0 = K

        print 'b0 = ' + str(b0)
        #print 'time = ' +  str(time)    
        print 'K = ' + str(K)
        print 'N0 = ' + str(N0)    

        number_repetition = 100;
        count_extinction = 0;

        mean_value_repetition = [];
        var_deviation_repetition = [];

        mean_value_birth_probability_repetition = []
        mean_value_death_probability_repetition = []     
    
        current_repetition = 1;
        # begin 2. layer, perform repetitions
        while current_repetition <= number_repetition:
        
            a = current_repetition
    
            # set initial conditions
            number_particles = [];   # array of particles
            number_particles.append(N0);
            
            birth_probability_line = []
            death_probability_line = []
            
            print 'ratio = ' + str(ratio) + '; b0 = ' + str(b0)  
            print 'repetition = ' + str(a) 
            #print 'number_particles = ' + str(number_particles)
        
            # begin 3. layer , perform for each time step
            for i in range(1, time):
    
                previous_number_particles = number_particles[i-1]
                #print previous_number_particles
                
                number_birth = 0;  # number of new individuals
                number_death = 0;  # number of died individuals
    
                birth_probability = b0 - (b0 -theta)*(previous_number_particles/K);  # calculate birth probability at the time step
                death_probability = d0 + (theta -d0)*(previous_number_particles/K);  # calculate death probability at the time step
    
                if birth_probability < 0:    # correction of birth probability
                    death_probability = death_probability - birth_probability 
            
                # begin 4. layer
                # look over all the individuals
                for j in range(1, int(previous_number_particles)+1):
                  
                    b_rand = random();
                    d_rand = random();
        
                    # assure that the interaval is open. is that needed?
                    while b_rand == float(0):
                        b_rand = random();
                            
                    while d_rand == float(0):
                        d_rand = random();
            
                    # decide if the agent will give a birth
                    if d_rand < death_probability:
                        number_death = number_death +1;
        
                    # decide if the agent will die
                    if b_rand < birth_probability:
                        number_birth = number_birth +1;
                    
                # end 4. layer
                # update the number of particles
                new_number_particles = previous_number_particles - number_death + number_birth;
                number_particles.append(new_number_particles);
            
                # write birth and death probability
                birth_probability_line.append(birth_probability)
                death_probability_line.append(death_probability)
        
            # end 3. layer
        
            # verification if the population went extinct
            if number_particles[time-1] == 0:
                number_repetition = number_repetition + 1;
                count_extinction = count_extinction +1;
                print 'extinction'
               
            #print 'number repetition ' + str(number_repetition)
            # consider non-extinct populations
            if number_particles[time-1] != 0:
                # calculate mean value and variance of simulation
                number_particles_array = numpy.array(number_particles) 
                mean_value = number_particles_array.mean()
                print 'K = ' + str(K) + '; theta = ' + str(theta) 
                print 'mean_value ' + str(mean_value)
                
                    
                var_deviation = number_particles_array.var() 
                    
                # write in an array for all simulations
                mean_value_repetition.append(mean_value) 
                var_deviation_repetition.append(var_deviation)   
            
                #
                birth_probability_line_array = array(birth_probability_line)
                death_probability_line_array = array(death_probability_line)
                #
                mean_value_birth_probability_repetition.append(birth_probability_line_array.mean())
                mean_value_death_probability_repetition.append(death_probability_line_array.mean())
            # update the repetition
            current_repetition = current_repetition + 1;
        
        # end 2. layer     
        print mean_value_repetition
        
     
        mean_value_repetition_array = array(mean_value_repetition)
        var_deviation_repetition_array = array(var_deviation_repetition)
            
        # mean of means
        mean_end = mean_value_repetition_array.mean()   
         
        # variance of means
        variance_of_mean_end = mean_value_repetition_array.var() 
        variance_of_mean_end_procent = (variance_of_mean_end/mean_end)*100.0

        # save into the file K, b0, theta, mean_number, mean_end, variance_of_mean_end 
        out.write('K = ' + str(K) + '\n')
        out.write('b0 = ' + str(b0) + '; theta = ' + str(theta) + '\n')
        out.write('mean_value_repetition = ' + str(mean_value_repetition) + '\n')
        out.write('mean_end = ' + str(mean_end) + '\n')
        out.write('variance_of_mean_end= ' + str(variance_of_mean_end) + '\n')
        out.write('\n') 

        # mean variance at the end
        mean_of_variance_deviation_repetition = var_deviation_repetition_array.mean();
        mean_of_variance_deviation_repetition_procent = (mean_of_variance_deviation_repetition/mean_end)*100

        print 'mean_end = ' + str(mean_end)

        print 'variance_of_mean_end = ' + str(variance_of_mean_end)
        print 'variance_of_mean_end_procent = ' + str(variance_of_mean_end_procent)

        print 'mean_of_variance_deviation_repetition = ' + str(mean_of_variance_deviation_repetition)
        print 'mean_of_variance_deviation_repetition_procent = ' + str(mean_of_variance_deviation_repetition_procent)

        print 'count_extinction = ' + str(count_extinction)

        print 'mean_end = ' + str(mean_end)
    
        several_means.append(mean_end)
        several_variances.append(variance_of_mean_end)
    
        # 
        mean_value_birth_probability_repetition_array = array(mean_value_birth_probability_repetition)
        mean_value_death_probability_repetition_array = array(mean_value_death_probability_repetition)
        #
        several_means_birth_probability.append(mean_value_birth_probability_repetition_array.mean())     
        several_means_death_probability.append(mean_value_death_probability_repetition_array.mean())

    # end 1. layer    
    print 'several_means = ' + str(several_means)
    print 'several_variances = ' + str(several_variances)
    
    print 'several_means_birth_probability = ' + str(several_means_birth_probability)
    print 'several_means_death_probability = ' + str(several_means_death_probability)
    
    # save several means and several variances
    out.write('K_line = ' + str(K_line) + '\n')
    out.write('several_means = ' + str(several_means) + '\n') 
    out.write('several_variances = ' + str(several_variances) + '\n')
    out.write('\n')
    out.close()

    return [several_means, several_variances]


#all_ratios= [(0.7/0.8), 10**(-1), 10**(-2)]
all_ratios = [0.5]   # array with ratios theta/b0 

for i in all_ratios:
    file_name = 'mean_var_several_means_b0_0_3_K10_500_t1000_repetition100_ratio_' + str(i) + '_.txt'
    function(ratio_value=i,name_of_the_file=file_name)
    print file_name
    
    