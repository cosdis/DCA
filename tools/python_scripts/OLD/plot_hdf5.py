
import commands
import os
import sys
import time

import matplotlib
matplotlib.use('Agg')

import shutil
import os

import h5py

from pylab import *

import numpy as np
import scipy
from scipy.interpolate import Rbf
import matplotlib.pyplot as plt
from matplotlib import cm

import json
import pylab
import numpy

from pylab import *
from scipy import *

from scipy          import optimize
from scipy.optimize import curve_fit

filename = raw_input('Please enter input-filename : ... \n\t 1 = data.hdf5 \n\t 2 = data_analysis.hdf5 \n\t 3 = data_susceptibilities.hdf5 \n\t 4 = data_spectrum.hdf5 \n\t 5 = output_QMC.hdf5 \n\n   ')

if(filename == '1'):
    filename = 'data.hdf5'

if(filename == '2'):
    filename = 'data_analysis.hdf5'

if(filename == '3'):
    filename = 'data_susceptibilities.hdf5'

if(filename == '4'):
    filename = 'data_spectrum.hdf5'

if(filename == '5'):
    filename = 'output_QMC.hdf5'

dir_name = filename[0:len(filename)-5]
print dir_name


figext = raw_input('Please enter figure-extension : ... \n\t 1 = png \n\t 2 = eps \n\t 3 = pdf \n\n   ')

if(figext == '1'):
    FIGURE_EXTENSION = ".png"

if(figext == '2'):
    FIGURE_EXTENSION = ".eps"

if(figext == '3'):
    FIGURE_EXTENSION = ".pdf"


def my_color(x, N):
    return cm.jet((x+0.0)/(N+0.0))

def my_format(x):
    return ('%.3g' % x)

def my_format6(x):
    return ('%.6e' % x)

def plot_contourf_jet(min, max, x, y, z, function_name, pic_name, pic_title):
    
    import numpy as np
    from scipy.interpolate import griddata
    import matplotlib.pyplot as plt
    import numpy.ma as ma
    from numpy.random import uniform

    # define grid.
    xi = np.linspace(min,max,200)
    yi = np.linspace(min,max,200)

    # grid the data.
    zi = griddata((x, y), z, (xi[None,:], yi[:,None]), method='cubic')
    #zi = griddata((x,y),z,xi,yi)

    figure(num=None)    

    matplotlib.rcParams['contour.negative_linestyle'] = 'solid'

    CS = plt.contour (xi,yi,zi,10,linewidths=1,colors='k')
    CS = plt.contourf(xi,yi,zi,100,cmap=plt.cm.jet)
    plt.colorbar() # draw colorbar

    plt.xlim(min,max)
    plt.ylim(min,max)

    #xticks([min, min/2, 0, max/2, max], [r"$-\pi$", r"$-\pi/2$", r"$0$", r"$\pi/2$", r"$\pi$"])
    #yticks([min, min/2, 0, max/2, max], [r"$-\pi$", r"$-\pi/2$", r"$0$", r"$\pi/2$", r"$\pi$"])

    plt.title(pic_title)

    plot(x, y, "ko", ms=2)
    
    savefig(pic_name)

def configure_figure_1():

    ax = gca()

    ax.aspect = (sqrt(5)-1.0)/2.0
    
    ax.tick_params(direction='in', length=8, width=1.5, pad=10, labelsize=16)

    ax.xaxis.set_ticks_position('bottom')
    ax.yaxis.set_ticks_position('left')
    
    subplots_adjust(left=0.075, bottom=0.075, right=0.975, top=0.975) 

    legend(prop={'size':20})

def configure_figure_2(x_label, y_label):

    ax = gca()

    ax.aspect = (sqrt(5)-1.0)/2.0
    
    ax.tick_params(direction='in', length=8, width=1.5, pad=10, labelsize=16)

    ax.xaxis.set_ticks_position('bottom')
    ax.yaxis.set_ticks_position('left')

    xlabel(x_label, fontsize=20, labelpad=12)
    ylabel(y_label, fontsize=20, labelpad=14)
    
    subplots_adjust(left=0.15, bottom=0.15, right=0.97, top=0.95) 

    legend(prop={'size':20})

def plot_imag_w_function(function_name, domains, function, functions):

    if os.path.exists('./'+dir_name+'/' + function_name):
        shutil.rmtree('./'+dir_name+'/' + function_name)
    os.makedirs('./'+dir_name+'/' + function_name)

    freq_dmn = domains["frequency-domain"]["elements"][:]

    shape = function["data"].shape

    print shape

    x0 = len(freq_dmn)/2-32
    x1 = len(freq_dmn)/2+32

    figure(num=None)
    title(filename)
    for b_ind in range(0, shape[3]):

        if(function_name=="Self_Energy"):

            x = freq_dmn[x0:x1]

            y   = function                       ["data"][x0:x1,0, 0,b_ind, 0,b_ind, 0]
            err = functions["Self_Energy-stddev"]["data"][x0:x1,0, 0,b_ind, 0,b_ind, 0]

            gca().errorbar(x, y, err, fmt='-', color='k', ecolor='k', label="Re[ "+function_name+" ]")

            y   = function                       ["data"][x0:x1,0, 0,b_ind, 0,b_ind, 1]
            err = functions["Self_Energy-stddev"]["data"][x0:x1,0, 0,b_ind, 0,b_ind, 1]

            gca().errorbar(x, y, err, fmt='-', color='r', ecolor='r', label="Im[ "+function_name+" ]")
            
        else:
            #subplot(211)
            plot(freq_dmn[x0:x1], function["data"][x0:x1,0, 0,b_ind, 0,b_ind, 0], 'k.-', label="Re[ "+function_name+" ]")
            
            #subplot(212)
            plot(freq_dmn[x0:x1], function["data"][x0:x1,0, 0,b_ind, 0,b_ind, 1], 'r.-', label="Im[ "+function_name+" ]")

    savefig('./'+dir_name+'/' + function_name + '/0_overview_b' + FIGURE_EXTENSION)

    figure(num=None)
    title(filename)

    for k_ind in range(0, shape[1]):
             
        if(function_name=="Self_Energy"):

            x = freq_dmn[x0:x1]

            y   = function                       ["data"][x0:x1,k_ind, 0,0, 0,0, 0]
            err = functions["Self_Energy-stddev"]["data"][x0:x1,k_ind, 0,0, 0,0, 0]

            errorbar(x, y, err, fmt='-', color='k', ecolor='k', label="Re[ "+function_name+" ]")

            y   = function                       ["data"][x0:x1,k_ind, 0,0, 0,0, 1]
            err = functions["Self_Energy-stddev"]["data"][x0:x1,k_ind, 0,0, 0,0, 1]

            errorbar(x, y, err, fmt='-', color='r', ecolor='r', label="Im[ "+function_name+" ]")
            
        else:   
            #subplot(211)
            plot(freq_dmn[x0:x1], function["data"][x0:x1,k_ind, 0,0, 0,0, 0], 'k.-', label="Re[ "+function_name+" ]")
            
            #subplot(212)
            plot(freq_dmn[x0:x1], function["data"][x0:x1,k_ind, 0,0, 0,0, 1], 'r.-', label="Im[ "+function_name+" ]")

    savefig('./'+dir_name+'/' + function_name + '/0_overview_k' + FIGURE_EXTENSION)

    for b_ind in range(0, shape[3]):
        for k_ind in range(0, shape[1]):
        
            figure(num=None)
            title(filename)
            
            if(function_name=="Self_Energy"):

                x = freq_dmn[x0:x1]
                
                y   = function                       ["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 0]
                err = functions["Self_Energy-stddev"]["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 0]
                
                errorbar(x, y, err, fmt='-', color='k', ecolor='k', label="Re[ "+function_name+" ]")
                
                y   = function                       ["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 1]
                err = functions["Self_Energy-stddev"]["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 1]

                errorbar(x, y, err, fmt='-', color='r', ecolor='r', label="Im[ "+function_name+" ]")
            
            else:
                #subplot(211)
                plot(freq_dmn[x0:x1], function["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 0], 'k.-', label="Re[ "+function_name+" ]")

                #subplot(212)
                plot(freq_dmn[x0:x1], function["data"][x0:x1,k_ind, 0,b_ind, 0,b_ind, 1], 'r.-', label="Im[ "+function_name+" ]")

            savefig('./'+dir_name+'/' + function_name + '/' + function_name + '_b='+ str(b_ind)+ '_k='+ str(k_ind) + FIGURE_EXTENSION)


def plot_imag_t_function(function_name, domains, function):

    if os.path.exists('./'+dir_name+'/' + function_name):
        shutil.rmtree('./'+dir_name+'/' + function_name)
    os.makedirs('./'+dir_name+'/' + function_name)

    freq_dmn = domains["time-domain"]["elements"][:]

    shape = function["data"].shape

    figure(num=None)
    for k_ind in range(0, shape[1]):
                
        plot(freq_dmn[:], function["data"][:,k_ind, 0,0, 0,0], 'k.-', label="Re[ "+function_name+" ]")
            
    title(filename)
    savefig('./'+dir_name+'/' + function_name + '/0_overview_k' + FIGURE_EXTENSION)

    for k_ind in range(0, shape[1]):
        
        figure(num=None)
            
        plot(freq_dmn[:], function["data"][:,k_ind, 0,0, 0,0], 'k.-', label="Re[ "+function_name+" ]")
        
        title(filename)
        savefig('./'+dir_name+'/' + function_name + '/' + function_name + '_k='+ str(k_ind) + FIGURE_EXTENSION)

    figure(num=None)
    for b_ind in range(0, shape[3]):
                
        plot(freq_dmn[:], function["data"][:,0, 0,b_ind, 0,b_ind], 'k.-', label="Re[ "+function_name+" ]")

    title(filename)
    savefig('./'+dir_name+'/' + function_name + '/0_overview_b' + FIGURE_EXTENSION)

def plot_real_w_function(function_name, domains, function, functions):

    if os.path.exists('./'+dir_name+'/' + function_name):
        shutil.rmtree('./'+dir_name+'/' + function_name)
    os.makedirs('./'+dir_name+'/' + function_name)

    freq_dmn = domains["frequency-domain-real-axis"]["elements"][:]

    shape = function["data"].shape

    if(len(shape)==1):

        figure(num=None)
        
        if(function_name=="spectral-density"):

            function_name_stddev = function_name+"-stddev"

            x   = freq_dmn
            y   = functions[function_name       ]["data"][:]
            err = functions[function_name_stddev]["data"][:]

            print "max error : " + str(max(abs(err)))

            yp = y+err
            ym = y-err
            
            plot(x, y, 'r-', label=function_name)        

            fill_between(x, y, yp, where=yp>=y,color='white', facecolor='grey', alpha=0.5, interpolate=True)
            fill_between(x, y, ym, where=y>=ym,color='white', facecolor='grey', alpha=0.5, interpolate=True)
            
            #xlim(-20, 10)
        else:

            x   = freq_dmn
            y   = function["data"][:]

            plot(x, y, 'k.-', label=function_name)        

        legend(loc="upper right")

        savefig('./'+dir_name+'/' + function_name + '/' + function_name + FIGURE_EXTENSION)

    if(len(shape)==3):

        figure(num=None)
        for b_ind in range(0, shape[2]):
            if(function_name=="spectral-density-per-orbital"):

                function_name_stddev = function_name+"-stddev"

                x   = freq_dmn
                y   = functions[function_name       ]["data"][:, 0,b_ind]
                err = functions[function_name_stddev]["data"][:, 0,b_ind]

                print "max error : " + str(max(abs(err)))

                yp = y+err
                ym = y-err
            
                if(shape[2]==1):
                    col  = 'red'
                    fcol = 'grey'
                else:
                    col  = my_color(b_ind, shape[2])
                    fcol = col
                
                plot(x, y, '-', color=col, label="orbital " + str(b_ind))        

                fill_between(x, y, yp, where=yp>=y, color='white', facecolor=fcol, alpha=0.5, interpolate=True)
                fill_between(x, y, ym, where=y>=ym, color='white', facecolor=fcol, alpha=0.5, interpolate=True)
            
            else:

                x   = freq_dmn
                y   = function["data"][:, 0,b_ind]

                plot(x, y, '.-', label="orbital " + str(b_ind))        

        legend(loc="upper right")
        savefig('./'+dir_name+'/' + function_name + '/0_overview_b'+ FIGURE_EXTENSION)

        for b_ind in range(0, shape[2]):
            figure(num=None)

            if(function_name=="spectral-density-per-orbital"):

                function_name_stddev = function_name+"-stddev"

                x   = freq_dmn
                y   = functions[function_name       ]["data"][:, 0,b_ind]
                err = functions[function_name_stddev]["data"][:, 0,b_ind]

                print "max error : " + str(max(abs(err)))

                yp = y+err
                ym = y-err
            
                plot(x, y, 'r-', label=function_name)        

                fill_between(x, y, yp, where=yp>=y, color='white', facecolor='grey', alpha=0.5, interpolate=True)
                fill_between(x, y, ym, where=y>=ym, color='white', facecolor='grey', alpha=0.5, interpolate=True)
            
            else:

                x   = freq_dmn
                y   = function["data"][:, 0,b_ind]

                plot(x, y, 'k.-', label=function_name)        

            legend(loc="upper right")

            savefig('./'+dir_name+'/' + function_name + '/' + function_name + '_b='+ str(b_ind) + FIGURE_EXTENSION)

    if(len(shape)==7):

        figure(num=None)
        for k_ind in range(0, shape[1]):

            subplot(211)
            plot(freq_dmn, function["data"][:,k_ind, 0,0, 0,0, 0], 'k.-', label="Re[ "+function_name+" ]")
            
            subplot(212)
            plot(freq_dmn, function["data"][:,k_ind, 0,0, 0,0, 1], 'r.-', label="Im[ "+function_name+" ]")

        xlabel(r"$\omega$")
        ylabel(function_name)

        #legend()

        savefig('./'+dir_name+'/' + function_name + '/0_overview_k' + FIGURE_EXTENSION)

        figure(num=None)
        for b_ind in range(0, shape[3]):
            
            subplot(211)
            plot(freq_dmn, function["data"][:,0, 0,b_ind, 0,b_ind, 0], '.-', label="Re[ b : "+str(b_ind)+" ]")
            #ylabel(r"Re[$\Sigma$]")

            subplot(212)
            plot(freq_dmn, function["data"][:,0, 0,b_ind, 0,b_ind, 1], '.-', label="Im[ b : "+str(b_ind)+" ]")
            #ylabel(r"Im[$\Sigma$]")

            xlabel(r"$\omega$")
            #ylabel(function_name)

            legend(loc="upper right")

        savefig('./'+dir_name+'/' + function_name + '/0_overview_b' + FIGURE_EXTENSION)

        for k_ind in range(0, shape[1]):
            for b_ind in range(0, shape[3]):

                figure(num=None)

                if(function_name == "self-energy-real-axis" or
                   function_name == "cluster-greens-function-G_k_w-real-axis"):
                
                    function_name_stddev = function_name+"-stddev"

                    subplot(211)
                    
                    x   = freq_dmn
                    y   = functions[function_name       ]["data"][:,k_ind, 0,b_ind, 0,b_ind, 0]
                    err = functions[function_name_stddev]["data"][:,k_ind, 0,b_ind, 0,b_ind, 0]
                    
                    yp = y+err
                    ym = y-err
                    
                    plot(x, y, 'r.-', label=function_name)        

                    fill_between(x, y, yp, where=yp>=y, color='white', facecolor='grey', alpha=0.5, interpolate=True)
                    fill_between(x, y, ym, where=y>=ym, color='white', facecolor='grey', alpha=0.5, interpolate=True)
                    
                    subplot(212)
                    
                    x   = freq_dmn
                    y   = functions[function_name       ]["data"][:,k_ind, 0,b_ind, 0,b_ind, 1]
                    err = functions[function_name_stddev]["data"][:,k_ind, 0,b_ind, 0,b_ind, 1]
                    
                    yp = y+err
                    ym = y-err
                    
                    plot(x, y, 'r.-', label=function_name)        
                    
                    fill_between(x, y, yp, where=yp>=y, color='white', facecolor='grey', alpha=0.5, interpolate=True)
                    fill_between(x, y, ym, where=y>=ym, color='white', facecolor='grey', alpha=0.5, interpolate=True)
            
                else:

                    plot(freq_dmn, function["data"][:,k_ind, 0,b_ind, 0,b_ind, 0], '.-', label="Re[ b : "+str(b_ind)+" ]")
                    plot(freq_dmn, function["data"][:,k_ind, 0,b_ind, 0,b_ind, 1], '.-', label="Im[ b : "+str(b_ind)+" ]")
            
                    legend(loc="upper right")

                xlabel(r"$\omega$")
                #ylabel(function_name)

                #legend(loc="upper right")

                savefig('./'+dir_name+'/' + function_name + '/' + function_name + '_k='+ str(k_ind) + '_b='+ str(b_ind) + FIGURE_EXTENSION)

def plot_DCA_iteration_function(function_name, domains, function):

    if not os.path.exists('./'+dir_name+'/various'):
        os.makedirs('./'+dir_name+'/various')

    f = figure(num=None)
    grid(True)
    
    xlabel('DCA-iteration')
    ylabel(function_name)

    if(function_name == 'L2_Sigma_difference'):
        semilogy(function["data"][:], "ko-") 
    else:
        plot(function["data"][:], "ko-") 

    savefig('./'+dir_name+'/various/' + function_name+FIGURE_EXTENSION)

def plot_error_distribution(function_name, domains, function):

    if not os.path.exists('./'+dir_name+'/various'):
        os.makedirs('./'+dir_name+'/various')

    error_dmn = domains["numerical-error-domain"]["elements"][:]

    f = figure(num=None)
    
    #title(filename)

    grid(True)

    data = function["data"][:]

    print sum(function["data"][:])
    
    data = data/sum(function["data"][:])
    
    semilogx(error_dmn, data, "k-") 

    configure_figure("numerical-error", r"$P(L_{\infty})$")

    savefig('./'+dir_name+'/various/' + function_name+FIGURE_EXTENSION)

def read_special_k_point_names(parameters):

    xticks_names = []

    names = parameters["band-structure-cut"]["Brillouin-zone-names"]
    for l0_ind in range(0, names["size"][0]):

        name = ""
        for l1_ind in range(0, len(names["data"][str(l0_ind)][:])):
            name = name + chr(names["data"][str(l0_ind)][l1_ind])

        if(name=="Gamma"):
            name=r"$\Gamma$"

        xticks_names.append(name)
                       
    return xticks_names

def plot_H_band_structure(parameters, domains, function_data):

    if not os.path.exists('./'+dir_name+'/various'):
        os.makedirs('./'+dir_name+'/various')
    
    bands = function_data["band-structure"]["data"]

    shape = bands.shape

    xticks_array = []
    for l_ind in range(0, shape[0]/101+1):
        xticks_array.append(l_ind*101)

    xticks_names = read_special_k_point_names(parameters)

#     names = parameters["band-structure-cut"]["Brillouin-zone-names"]
#     for l0_ind in range(0, names["size"][0]):

#         name = ""
#         for l1_ind in range(0, len(names["data"][str(l0_ind)][:])):
#             name = name + chr(names["data"][str(l0_ind)][l1_ind])

#         if(name=="Gamma"):
#             name=r"$\Gamma$"

#         xticks_names.append(name)
                       
#     print xticks_names

    figure(num=None)

    for b_ind in range(0, shape[2]):
        plot(bands[:,0,b_ind], 'k-')

    xlim(0, shape[0])

    yticks(fontsize=16)
    #xticks([0*shape[0]/4, 1*shape[0]/4, 2*shape[0]/4, 3*shape[0]/4, 4*shape[0]/4], [r'$(0,0)$', r'$(\pi,\pi)$', r'$(\pi,0)$', r'$(0,\pi)$', r'$(0,0)$'], fontsize=16, rotation=45)           
    if(len(xticks_names)==shape[0]/101+1):
        xticks(xticks_array, xticks_names)
    else:
        xticks(xticks_array)
    
    grid(True)
    
    subplots_adjust(top=0.85, left=0.10, right=0.95)

    #legend(loc=(0, 1), prop={'size':12}, ncol=3, columnspacing=0.2 )
       
    configure_figure_1()

    savefig('./'+dir_name+'/band_structure' + FIGURE_EXTENSION)
    
def plot_S_band_structure(parameters, domains, function_data):

    if not os.path.exists('./'+dir_name+'/various'):
        os.makedirs('./'+dir_name+'/various')
    
    Sigma_qmc     = function_data["Sigma-band-structure"]["data"]
    Sigma_cluster = function_data["Sigma-cluster-band-structure"]["data"]

    Sigma_lattice       = function_data["Sigma-lattice-band-structure"]["data"]
    Sigma_interpolated  = function_data["Sigma-band-structure-interpolated"]["data"]
    Sigma_coarsegrained = function_data["Sigma-band-structure-coarsegrained"]["data"]

    shape = Sigma_qmc.shape

    xticks_array = []
    for l_ind in range(0, shape[0]/101+1):
        xticks_array.append(l_ind*101)

    xticks_names = read_special_k_point_names(parameters)

    figure(num=None)

    max_diff_re = max(abs(Sigma_qmc[:,0,0,0]-Sigma_cluster[:,0,0,0]))
    max_diff_im = max(abs(Sigma_qmc[:,0,0,1]-Sigma_cluster[:,0,0,1]))

    plot(Sigma_qmc[:,0,0,0], 'k--', label=r"Re[$\Sigma_{\vec{K}}(\omega_m=\pi\,T)$]")
    plot(Sigma_qmc[:,0,0,1], 'r--', label=r"Im[$\Sigma_{\vec{K}}(\omega_m=\pi\,T)$]")

    plot(Sigma_lattice[:,0,0,0], 'k-', label=r"Re[$\Sigma(\vec{k}, \omega_m=\pi\,T}$]")
    plot(Sigma_lattice[:,0,0,1], 'r-', label=r"Im[$\Sigma(\vec{k}, \omega_m=\pi\,T)$]")

    plot(Sigma_cluster[:,0,0,0], 'k.', label=r"Re[$\bar{\Sigma}_{\vec{K}}(\omega_m=\pi\,T)$]", mew=0, markevery=8)
    plot(Sigma_cluster[:,0,0,1], 'r.', label=r"Im[$\bar{\Sigma}_{\vec{K}}(\omega_m=\pi\,T)$]", mew=0, markevery=8)

    xlim(0, shape[0])

#     yticks(fontsize=16)
#     xticks([0*shape[0]/4, 1*shape[0]/4, 2*shape[0]/4, 3*shape[0]/4, 4*shape[0]/4], [r'$(0,0)$', r'$(\pi,\pi)$', r'$(\pi,0)$', r'$(0,\pi)$', r'$(0,0)$'], fontsize=16, rotation=45)           
            
    yticks(fontsize=16)
    #xticks([0*shape[0]/4, 1*shape[0]/4, 2*shape[0]/4, 3*shape[0]/4, 4*shape[0]/4], [r'$(0,0)$', r'$(\pi,\pi)$', r'$(\pi,0)$', r'$(0,\pi)$', r'$(0,0)$'], fontsize=16, rotation=45)           
    if(len(xticks_names)==shape[0]/101+1):
        xticks(xticks_array, xticks_names)
    else:
        xticks(xticks_array)

    subplots_adjust(top=0.85, left=0.10, right=0.95)

    legend(loc=(0, 1), prop={'size':12}, ncol=3, columnspacing=0.2 )
       
    savefig('./'+dir_name+'/Sigma_overview' + FIGURE_EXTENSION)

def plot_cpe_approximation(domains, function_data):

    if os.path.exists('./'+dir_name+'/f_approx_versus_f_source/'):
        shutil.rmtree('./'+dir_name+'/f_approx_versus_f_source/')
    os.makedirs('./'+dir_name+'/f_approx_versus_f_source/')

    freq_dmn = domains["frequency-domain"]["elements"][:]

    Sigma_orig       = function_data["f-original"]["data"]

    Sigma_cpe        = function_data["f-approx"]       ["data"]
    Sigma_cpe_stddev = function_data["f-approx-stddev"]["data"]

    Sigma_qmc        = function_data["f-measured"]       ["data"]
    Sigma_qmc_stddev = function_data["f-measured-stddev"]["data"]

    shape = Sigma_qmc.shape

    x0 = len(freq_dmn)/2
    x1 = len(freq_dmn)/2+shape[0]

    figure(num=None)
    for k_ind in range(0, shape[1]):

        plot(freq_dmn[x0:x1], Sigma_qmc[:,k_ind, 0,0, 0,0, 1], 'ro' , label=r"Im[$\Sigma_{CPE}$]", mfc='w', mec='r')        
        plot(freq_dmn[x0:x1], Sigma_cpe[:,k_ind, 0,0, 0,0, 1], 'r+-', label=r"Im[$\Sigma_{QMC}$]")

    savefig('./'+dir_name+'/f_approx_versus_f_source/0_overview_k' + FIGURE_EXTENSION)

    figure(num=None)
    for b_ind in range(0, shape[3]):
        
        k_ind = 0

        plot(freq_dmn[x0:x1], Sigma_qmc[:,k_ind, 0,b_ind, 0,b_ind, 0], 'ko' , label=r"Re[$\Sigma_{CPE}$]", mfc='w', mec='k')
        plot(freq_dmn[x0:x1], Sigma_cpe[:,k_ind, 0,b_ind, 0,b_ind, 0], 'k+-', label=r"Re[$\Sigma_{QMC}$]")

        plot(freq_dmn[x0:x1], Sigma_qmc[:,k_ind, 0,b_ind, 0,b_ind, 1], 'ro' , label=r"Im[$\Sigma_{CPE}$]", mfc='w', mec='r')            
        plot(freq_dmn[x0:x1], Sigma_cpe[:,k_ind, 0,b_ind, 0,b_ind, 1], 'r+-', label=r"Im[$\Sigma_{QMC}$]")

    savefig('./'+dir_name+'/f_approx_versus_f_source/0_overview_b' + FIGURE_EXTENSION)

    for b_ind in range(0, shape[3]):
        for k_ind in range(0, shape[1]):
        
            figure(num=None)

            x = freq_dmn[x0:x1]

            ###
            ### Re
            ###

            y_qmc_re        = Sigma_qmc       [:,k_ind, 0,b_ind, 0,b_ind, 0]
            y_qmc_re_stddev = Sigma_qmc_stddev[:,k_ind, 0,b_ind, 0,b_ind, 0]

            y_cpe_re        = Sigma_cpe       [:,k_ind, 0,b_ind, 0,b_ind, 0]
            y_cpe_re_stddev = Sigma_cpe_stddev[:,k_ind, 0,b_ind, 0,b_ind, 0]

            yp_qmc_re = y_qmc_re+y_qmc_re_stddev
            ym_qmc_re = y_qmc_re-y_qmc_re_stddev

            plot(x, y_qmc_re, 'ko' , label=r"Re[$\bar{\Sigma}_{QMC}$]", mfc='w', mec='k')
            plot(x, y_cpe_re, 'k+-', label=r"Re[$\Sigma_{CPE}$]")    
            
            fill_between(x, y_qmc_re, yp_qmc_re, where=yp_qmc_re>=y_qmc_re , color='white', facecolor='grey', alpha=0.5, interpolate=True)
            fill_between(x, y_qmc_re, ym_qmc_re, where=y_qmc_re >=ym_qmc_re, color='white', facecolor='grey', alpha=0.5, interpolate=True)

            plot(x, y_cpe_re+y_cpe_re_stddev, 'k--')#, label=r"Re[$\Sigma_{CPE}$]")            
            plot(x, y_cpe_re-y_cpe_re_stddev, 'k--')#, label=r"Re[$\Sigma_{CPE}$]")            

            ###
            ### Im
            ###

            y_qmc_im        = Sigma_qmc       [:,k_ind, 0,b_ind, 0,b_ind, 1]
            y_qmc_im_stddev = Sigma_qmc_stddev[:,k_ind, 0,b_ind, 0,b_ind, 1]

            y_cpe_im        = Sigma_cpe       [:,k_ind, 0,b_ind, 0,b_ind, 1]
            y_cpe_im_stddev = Sigma_cpe_stddev[:,k_ind, 0,b_ind, 0,b_ind, 1]

            yp_qmc_im = y_qmc_im+y_qmc_im_stddev
            ym_qmc_im = y_qmc_im-y_qmc_im_stddev
                
            plot(x, y_qmc_im                , 'ro' , label=r"Im[$\bar{\Sigma}_{QMC}$]", mfc='w', mec='r')
            plot(x, y_cpe_im                , 'r+-', label=r"Im[$\Sigma_{CPE}$]")    

            fill_between(x, y_qmc_im, yp_qmc_im, where=yp_qmc_im>=y_qmc_im , color='white', facecolor='red', alpha=0.25, interpolate=True)
            fill_between(x, y_qmc_im, ym_qmc_im, where=y_qmc_im >=ym_qmc_im, color='white', facecolor='red', alpha=0.25, interpolate=True)

            plot(x, y_cpe_im+y_cpe_im_stddev, 'r--')#, label=r"Re[$\Sigma_{CPE}$]")            
            plot(x, y_cpe_im-y_cpe_im_stddev, 'r--')#, label=r"Re[$\Sigma_{CPE}$]")            
            
            ###
            ### Original
            ###

            plot(x, Sigma_orig[:,k_ind, 0,b_ind, 0,b_ind, 0], 'k.' , label=r"Re[$\Sigma_{QMC}$]")
            plot(x, Sigma_orig[:,k_ind, 0,b_ind, 0,b_ind, 1], 'r.' , label=r"Im[$\Sigma_{QMC}$]")

            legend(loc="upper right")

            savefig('./'+dir_name+'/f_approx_versus_f_source/f_approx_versus_f_source' + '_k='+ str(k_ind) + '_b='+ str(b_ind) + FIGURE_EXTENSION)

    
def plot_eigenvector(function_name, domains, function):

    if os.path.exists('./'+dir_name+'/' + function_name):
        shutil.rmtree('./'+dir_name+'/' + function_name)
    os.makedirs('./'+dir_name+'/' + function_name)

    k_dmn    = domains["LATTICE_TP"]["MOMENTUM_SPACE"]["elements"]["data"][:,:]
    k_basis  = domains["LATTICE_TP"]["MOMENTUM_SPACE"]["basis"]   ["data"]
    k_super_basis  = domains["LATTICE_TP"]["MOMENTUM_SPACE"]["super-basis"]["data"]
    freq_dmn = domains["vertex-frequency-domain (COMPACT)"]["elements"][:]
    
    print k_basis[:,:]
    print k_super_basis[:,:]

    shape = function["data"].shape

    for l_ind in range(0, 10):
        figure(num=None)
        for k_ind in range(0, shape[1]):

            if(abs(k_dmn[k_ind,0]+k_dmn[k_ind,1]-3.141592)<1.e-3):
                plot(freq_dmn, function["data"][:,k_ind, 0,0, l_ind, 0], 'k.-', label="Re[ "+function_name+" ]")
                plot(freq_dmn, function["data"][:,k_ind, 0,0, l_ind, 1], 'r.-', label="Im[ "+function_name+" ]")
        
        savefig('./'+dir_name+'/' + function_name + '/phi_w_l=' + str(l_ind) + FIGURE_EXTENSION)

    for l_ind in range(0, 10):

        x=[]
        y=[]

        z_re=[]
        z_im=[]

        for k_ind in range(0, shape[1]):

            for l0 in [-2, -1, 0, 1, 2]:
                for l1 in [-2, -1,0, 1, 2]:
            
                    x.append(k_dmn[k_ind,0] + l0*k_super_basis[0,0] + l1*k_super_basis[1,0])
                    y.append(k_dmn[k_ind,1] + l0*k_super_basis[0,1] + l1*k_super_basis[1,1])

                    z_re.append(function["data"][len(freq_dmn)/2,k_ind, 0,0, l_ind, 0])
                    z_im.append(function["data"][len(freq_dmn)/2,k_ind, 0,0, l_ind, 1])


        L = 10

        pic_name = './'+dir_name+'/' + function_name + '/phi_k_l=' + str(l_ind) + '_re' + FIGURE_EXTENSION

        plot_contourf_jet(-L, L, x, y, z_re, function_name, pic_name, 'Re[phi_k_l=' + str(l_ind) + ']')

        pic_name = './'+dir_name+'/' + function_name + '/phi_k_l=' + str(l_ind) + '_im' + FIGURE_EXTENSION

        plot_contourf_jet(-L, L, x, y, z_im, function_name, pic_name, 'Im[phi_k_l=' + str(l_ind) + ']')

def plot_G_tp(function_name, data_domains, data_function):

    print "start plot_G_tp"

def plot_G4_k_k_w_w(function_name, data_domains, data_function):

    print "start plot_G4_k_k_w_w"

    if os.path.exists('./'+dir_name+'/G4_k_k_w_w/'):
        shutil.rmtree('./'+dir_name+'/G4_k_k_w_w/')
    os.makedirs('./'+dir_name+'/G4_k_k_w_w/')

    freq_dmn = data_domains["vertex-frequency-domain (COMPACT)"]["elements"][:]

    G4 = data_function["data"]

    print G4.shape

    print "\t real \n"
    for i in range(0, len(freq_dmn)):
        tmp = str(freq_dmn[i]) + "\t\t"
        for j in range(0, len(freq_dmn)):
            tmp = tmp + my_format6(G4[i,j, 0,0, 0,0, 0,0, 0]) + "\t"
        print tmp
    print "\n"

    print "\t imag \n"
    for i in range(0, len(freq_dmn)):
        tmp = str(freq_dmn[i]) + "\t\t"
        for j in range(0, len(freq_dmn)):
            tmp = tmp + my_format6(G4[i,j, 0,0, 0,0, 0,0, 1]) + "\t"
        print tmp
    print "\n"


    x = []
    y_re = []
    y_im = []
    z_re = []
    z_im = []
    for i in range(0, len(freq_dmn)):
        x.append(freq_dmn[i])
        y_re.append(G4[i,i, 0,0, 0,0, 0,0, 0])
        y_im.append(G4[i,i, 0,0, 0,0, 0,0, 1])
        z_re.append(G4[i,len(freq_dmn)-1-i, 0,0, 0,0, 0,0, 0])
        z_im.append(G4[i,len(freq_dmn)-1-i, 0,0, 0,0, 0,0, 1])

    figure(num=None)
    
    plot(x, y_re, "ks-", label=r"Re[$G^{II}(w_n, w_n)$]")
    plot(x, y_im, "ro-", label=r"Re[$G^{II}(w_n, w_n)$]")

    plot(x, z_re, "k^--", label=r"Re[$G^{II}(w_n, -w_n)$]")
    plot(x, z_im, "rv--", label=r"Re[$G^{II}(w_n, -w_n)$]")

    legend(loc="upper right")

    savefig('./'+dir_name+'/G4_k_k_w_w/G4_k_k_w_w'
            + '_k0='+ str(0) + '_k1='+ str(0) 
            + '_b0='+ str(0) + '_b1='+ str(0) + '_b2='+ str(0) + '_b3='+ str(0) 
            + FIGURE_EXTENSION)






data = h5py.File(filename,'r')

data_domains = data["domains"]

for i in range(0, len(data_domains.keys())):

    key_name = data_domains.keys()[i]

    print key_name

for i in range(0, len(data.keys())):

    key_name = data.keys()[i]
    print key_name

    if(key_name == "functions"          or 
       key_name == "DCA-loop-functions" or 
       key_name == "CPE-functions"      or 
       key_name == "spectral-functions" or
       key_name == "analysis-functions" or
       key_name == "CT-AUX-SOLVER-functions" or
       key_name == "SS-HYB-SOLVER-functions" or
       key_name == "A-versus-G-functions"):

        data_functions = data[key_name]

        for j in range(0, len(data[key_name].keys())):
        
            function_name = data[key_name].keys()[j]

            data_function = data[key_name][function_name]

            print '\treading '+ function_name + " ... "

            if(function_name == "band-structure"):
                plot_H_band_structure(data["parameters"], data_domains, data_functions)

            if(function_name == "Sigma-lattice-band-structure"):
                plot_S_band_structure(data["parameters"], data_domains, data_functions)
                
            #if(function_name == "numerical-error-distribution-of-N-matrices"):
            #plot_error_distribution(function_name, data_domains, data_function)

            if(function_name == "Self_Energy" or
               function_name == "Self-Energy-n-1-iteration" or
               function_name == "Self-Energy-n-0-iteration" or
               function_name == "cluster_excluded_greens_function_G0_k_w" or
               function_name == "cluster_greens_function_G_k_w" or
               function_name == "free_cluster_greens_function_G0_k_w" or
               function_name == "cluster_hybridization_F_k_w"):
                answer = raw_input('\n\tplot '+ function_name + ' ? (y/n) : ')

                if(answer=='y'):
                    plot_imag_w_function(function_name, data_domains, data_function, data_functions)

            if(function_name == "free_cluster_greens_function_G0_r_t" or
               function_name == "cluster_hybridization_F_r_t"):
                answer = raw_input('\n\tplot '+ function_name + ' ? (y/n) : ')

                if(answer=='y'):
                    plot_imag_t_function(function_name, data_domains, data_function)
                    
            if(function_name == "alpha"                   or
               function_name == "self-energy-real-axis"                    or
               function_name == "cluster-greens-function-G_k_w-real-axis"  or
               function_name == "cluster-greens-function-G0_k_w-real-axis" or
               function_name == "spectral-density"                         or
               function_name == "spectral-density-per-orbital"             or
               function_name == "free-spectral-density"                    or
               function_name == "free-spectral-density-per-orbital"        ):
                answer = raw_input('\n\tplot '+ function_name + ' ? (y/n) : ')

                if(answer=='y' ):               
                    plot_real_w_function(function_name, data_domains, data_function, data_functions)

            if(function_name == "f-approx"):
                answer = raw_input('\n\tplot '+ function_name + ' ? (y/n) : ')

                if(answer=='y' ):
                    plot_cpe_approximation(data_domains, data_functions)

            if(function_name == "leading-eigenvectors"):
                answer = raw_input('\n\tplot '+ function_name + ' ? (y/n) : ')

                if(answer=='y' ):
                    plot_eigenvector(function_name, data_domains, data_function)

            if(function_name == 'sign'
               or function_name == 'density'
               or function_name == 'chemical-potential'
               or function_name == 'expansion_order'
               or function_name == 'updates'
               or function_name == 'L2_Sigma_difference'
               or function_name == 'Gflops_per_mpi_task'):
                plot_DCA_iteration_function(function_name, data_domains, data_function)

            if(function_name == 'G_tp_non' or
               function_name == 'G_tp_int'):
                plot_G_tp(function_name, data_domains, data_function)

            if(function_name == 'G4_k_k_w_w'):
                plot_G4_k_k_w_w(function_name, data_domains, data_function)
                

data.close()
