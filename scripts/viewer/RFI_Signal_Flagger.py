
# This code is known as as a RFI and Signal Flagger. Its main purpose is to process a FITS file after the original data has been merged after FREDDA or PRESTO. 
# The first half of the code takes the FITS file and sums the pixel values along the y-dimension and effectively allows for the total power vs time plot to be generated. 
# Any values that exceed a certain RFI threshold (default = 5*IQR_rfi_noise) will be identified as RFI and can be further classified as single spikes or continuous blocks. 
# The second half, allows for transient signals to be detected using de-dispersion and by adapting the same method to find times where the total_power vs time exceeds a
# certian threshold (default = 5*IQR_signal_noise). 

# %%
#!/usr/bin/env python3
# coding: utf-8

import matplotlib
matplotlib.use('Qt5Agg')

# %%
import astropy.io.fits as fits 
import math 
import numpy as np 
import astropy
from astropy.table import QTable
from astropy.table import Table
import pandas as pd
import matplotlib.pyplot as plt
import os 
import sys
from scipy import ndimage
from decimal import Decimal, getcontext
import copy

# %%
#fits_file = "out_genfrb0011.fits.gz"
fits_file = sys.argv[1]
DM = float(sys.argv[2])

# %%
# Global Parameters 

# IQR_multiplier (for detecting RFI) and IQR_multiplier (for detecting a signal) is the multipler which allows for a specified S/N ratio. 
IQR_multiplier = 5              
IQR_multiplier_signal = 5
max_width = 50 # Max width is used in the function threshold_signal function as a parameter to characterise a RFI signal in a de-dispersed total power vs time FITS file. 
dedis_width = 10 # This is the width that the signal spike spans across. It is used in the threshold_signal function. 


# %%
# Processing FITS Files
# This block allows for the fits file being fed into the function to be processed. 

def process_fits_files(fits_file):

    fits_open = None
 
    try: 

        fits_open = fits.open(fits_file) 
        primary_hdu = fits_open[0] 
        headers = primary_hdu.header 
        data = primary_hdu.data.astype(primary_hdu.data.dtype.newbyteorder("=")) #This line allow data with different byte orders to be processed. 
        pixel_data = pd.DataFrame(data) 
        x_dimension = float(headers['NAXIS1'])
        y_dimension = float(headers['NAXIS2'])
        reference_x_pixel = int(headers['CRPIX1'])
        delta_x = float(headers['CDELT1'])*1000 # Multiplied by 100 to convert to ms
        starting_frequency = 199.60214170 # Starting frequency used by PRESTO 
        delta_y = float(headers['CDELT2'])
        reference_y_pixel = int(headers['CRPIX2']) 
        

        return  x_dimension, y_dimension, delta_x, starting_frequency, delta_y, reference_y_pixel, pixel_data, delta_y, headers, data 
        
    except IndexError:
        print("Index was not found")
    except KeyError:
        print("Key was not found")
    except FileNotFoundError:
        print("File was not found")
    except Exception as e:
        print(f"An error occurred when processing the file: {type(e).__name__}: {str(e)}")
              
    finally:
        if fits_open:
            fits_open.close()
            
    return None, None, None, None, None, None , None, None, None, None  

x_dimension, y_dimension, delta_x, starting_frequency, delta_y, reference_x_pixel, pixel_data, delta_y, headers, data = process_fits_files(fits_file)

# %%
# Power vs Time RFI 
# This block allows for the summation of all the pixel values along each column to be calculated, essentially achieving the total power in each column. 
# This block is done for the purposes of searching for RFI. 

def power_vs_time(delta_x,pixel_data):

    total_power_vs_time = []
    time_xaxis = []
    
    for column in pixel_data:
        total_power = sum(pixel_data[column])
        total_power_vs_time.append(total_power)
        time = (pixel_data.columns.get_loc(column))*delta_x
        time_xaxis.append(time)

    sorted_total_power_vs_time = np.sort(total_power_vs_time)
    median = np.median(sorted_total_power_vs_time)

    return total_power_vs_time, time_xaxis, sorted_total_power_vs_time, median
    

total_power_vs_time, time_xaxis, sorted_total_power_vs_time, median = power_vs_time(delta_x, pixel_data)


# %%
# Thresholding RFI
# This block allows for the thresholds for RFI to be established.

def threshold_median(time_xaxis, total_power_vs_time, IQR_multiplier):

    Q1 = np.quantile(sorted_total_power_vs_time, 0.25)
    Q3 = np.quantile(sorted_total_power_vs_time, 0.75)
    IQR = Q3 - Q1
    IQR_rms = IQR/1.35
    rfi_threshold_median_above = Q3 + IQR_multiplier*IQR_rms 
    rfi_threshold_median_below = Q1 - IQR_multiplier*IQR_rms
    
    rfi_events_spikes = [] # RFI may present as single spike at one time resolution accross multiple channels. 
    rfi_events_block = [] # Or RFI may continue over multiple time resolutions and multiple channels, appearing as a block. 

    i = 0
    while i < len(time_xaxis):
        if (total_power_vs_time[i] >= rfi_threshold_median_above) or (total_power_vs_time[i] <= rfi_threshold_median_below): # Checks whether element in total power exceeds RFI threshold 
            start_time = time_xaxis[i]
            start_power = total_power_vs_time[i]
            count = 0
            
            while (i + count < len(time_xaxis) and 
                   (total_power_vs_time[i + count] >= rfi_threshold_median_above or total_power_vs_time[i + count] <= rfi_threshold_median_below)): # For given element checks whether the consecutive element also exceeds RFI threshold. 
                count += 1 
            
            if count == 1: # If RFI is only found in one time stamp/resolution then this will be classified as a RFI spike. 
                rfi_events_spikes.append((start_time, start_power))
                #print(f"RFI spikes occur from a start time {start_time}")
            elif count > 1: # If continuous then it is a block. 
                end_time = time_xaxis[i + count - 1]
                end_power = total_power_vs_time[i + count - 1]
                rfi_events_block.append((start_time, end_time, start_power, end_power))
               #print(f"RFI spikes occur from a start time {start_time} and end at {end_time}")
        
            i += max(1,count)
            
        else:
            i += 1
    
    return rfi_events_spikes, rfi_events_block, rfi_threshold_median_above, rfi_threshold_median_below, median

rfi_events_spikes, rfi_events_block, rfi_threshold_median_above, rfi_threshold_median_below, median = threshold_median(time_xaxis, total_power_vs_time, IQR_multiplier)


# %%
# De-dispersing 
# This blocks is used to incoherently de-disperse the same FITS for the purposes of identifying whether a signal is present in the FITS file. This is useful if the FITS
# does not present anything obvious and could have a signal. This de-dispersing is depends on the DM of the source being detected. 

getcontext().prec = 50

def dedispersing(DM):
    
    DM = Decimal(str(DM))
    
    frequencies = np.arange(starting_frequency, ((starting_frequency + (y_dimension - 1)*delta_y)), delta_y)

    new_table = []
    tdelay_arr = []
    shift_arr = []
    freq_arr = []
    
    for i in range(len(frequencies)):
        freq = (Decimal(starting_frequency) + Decimal((y_dimension - 1)*delta_y))  # This is the signal highest frequency in the FITS File (the very top of the FITS file)
        fch1 = Decimal(starting_frequency) + (i * Decimal(delta_y)) # This the bottom frequency which is at the very bottom of the FITS file.

        tdelay = Decimal(Decimal(4.15)* 10**6 * DM * ((fch1**-2) - (freq**-2)))
        tdelay_arr.append(tdelay)

        shift = (tdelay / Decimal(delta_x))   
        shift_arr.append(shift) 
        shift = int(round(float(shift))) #Rounded, integer shift in number of pixels. 

        rolled_pixel_data_row = np.roll(pixel_data.iloc[i].values, shift = -shift, axis = 0) # Rolls each row of the original pixel data by the calculated shift for that the channel. 
        new_table.append(rolled_pixel_data_row) # Adds all the newly shifted rows to another array 
        #print("delay(%d) = %.4f , shift = %.4f for freq = %.4f MHz -> %.4f MHz " % (i,tdelay,shift,freq,fch1))    
    new_pixel_data = pd.DataFrame(new_table)
      
    return new_pixel_data, shift_arr, tdelay_arr, freq_arr 
    
new_pixel_data, shift_arr, tdelay_arr, freq_arr = dedispersing(DM) 


# %%
#Power vs Time Signal 
# This block allows for the summation of all the pixel values along each column to be calculated, essentially achieving the total power in each column. 
# This block is done for the purposes of searching for signal. 

def power_vs_time_signal(delta_x,new_pixel_data):

    total_power_vs_time_signal = []
    time_xaxis_signal = []
    
    for column in new_pixel_data:
        total_power_signal = sum(new_pixel_data[column])
        total_power_vs_time_signal.append(total_power_signal)
        time_signal = (new_pixel_data.columns.get_loc(column)) * delta_x
        time_xaxis_signal.append(time_signal)
        
    return total_power_vs_time_signal, time_xaxis_signal

total_power_vs_time_signal, time_xaxis_signal = power_vs_time_signal(delta_x, new_pixel_data)

# %%
# Thresholding Signal 
# This block allows for the thresholds for RFI to be established. A signal, which has been de-dispersed will appear to have spike, however will span across a few time resolutions. 
# And signals which span a max_width or greater are still considered to be RFI blocks which have been carried over to signal processing. 

def threshold_signal(time_xaxis_signal, total_power_vs_time_signal, IQR_multiplier_signal):

    sorted_total_power_vs_time_signal = np.sort(total_power_vs_time_signal)
    median_signal = np.median(sorted_total_power_vs_time_signal)
    Q1_signal = np.quantile(sorted_total_power_vs_time_signal, 0.25)
    Q3_signal = np.quantile(sorted_total_power_vs_time_signal, 0.75)
    IQR_signal = Q3_signal - Q1_signal
    IQR_rms_signal = IQR_signal/1.35
    signal_threshold_median_above = Q3_signal + IQR_multiplier_signal*IQR_rms_signal
    signal_threshold_median_below = Q1_signal - IQR_multiplier_signal*IQR_rms_signal
        
    signal_event = []
    rfi_block_in_signal = []

    i = 0
    while i < len(total_power_vs_time_signal):
        if (signal_threshold_median_above <= total_power_vs_time_signal[i]) or (signal_threshold_median_below >= total_power_vs_time_signal[i]): # Checks whether element in total power exceeds signal threshold 
            count = 0 
            while (i + count < len(total_power_vs_time_signal)) and ((signal_threshold_median_above <= total_power_vs_time_signal[i + count]) or (signal_threshold_median_below >= total_power_vs_time_signal[i + count])): #For given element checks whether the consecutive element also exceeds signal threshold. 
                count += 1 
                
            if count < dedis_width: # If signal is only found in a few time stamps/time resolution then this will be classified as a signal spike. 
                signal_event.append((time_xaxis_signal[i], total_power_vs_time_signal[i]))
            elif count >= max_width: #If continuous then it is a RFI block.
                rfi_block_in_signal.append((time_xaxis_signal[i], total_power_vs_time_signal[i]))

            i += max(1,count)
        else:
            i += 1
           
    return signal_event, median_signal, signal_threshold_median_above, signal_threshold_median_below, rfi_block_in_signal

signal_event, median_signal, signal_threshold_median_above, signal_threshold_median_below, rfi_block_in_signal = threshold_signal(time_xaxis_signal, total_power_vs_time_signal, IQR_multiplier_signal)

# %%
# Categorise RFI 
# This blocks allows for different scenarios of the FITS file has. 

def is_RFI_signal(rfi_events_spikes, rfi_events_block, signal_event, rfi_block_in_signal):
    if (len(rfi_events_spikes) >= 1 or len(rfi_events_block) >= 1 ) and (len(signal_event) >= 1): #len(rfi_block_in_signal) >= 1) 
        return "There is both RFI and a potential signal in this candidate."
    elif (len(rfi_events_spikes) >= 1) or (len(rfi_events_block)) or (len(rfi_block_in_signal) >= 1):
        return "There is RFI in this candidate."
    elif (len(rfi_block_in_signal) >= 1 or (len(rfi_block_in_signal) >= 1)):
        return "There is a RFI block in this FITS file."
    elif (len(rfi_block_in_signal)>= 1):
        return "Although de-dispersion exceeds threshold, this is due to a RFI block."
    elif (len(signal_event) >= 1):
        return "There is a signal in this FITS file!"
    else:
        return "There is no detection of RFI or a signal in this candidate. Please investigate further."

# %%
# Plotting 
# Plots original FITS file, RFI total power vs time, de-dispersed FITS file, Signal total power vs time. 

def plotting(time_xaxis, total_power_vs_time, pixel_data, new_pixel_data, time_xaxis_signal, total_power_vs_time_signal, rfi_threshold_median_above,rfi_threshold_median_below, median, median_signal, signal_threshold_median_above, signal_threshold_median_below):
    fig, axs = plt.subplots(4,1, sharex = True, figsize = (10,20))
    fig.subplots_adjust(hspace = 0.2)

    axs[0].imshow(pixel_data, aspect='auto', origin='lower', cmap='twilight', extent=[0, x_dimension*delta_x, 0, y_dimension*delta_y])
    axs[0].title.set_text('Original FITS file')

    axs[1].plot(time_xaxis, total_power_vs_time, label='Total Power')
    axs[1].axhline(y=median, color='r', linestyle='--', label='Median')
    axs[1].axhline(y=rfi_threshold_median_above, color='k', linestyle=':', label='Upper Threshold')
    axs[1].axhline(y=rfi_threshold_median_below, color='k', linestyle=':', label='Lower Threshold')
    axs[1].set_ylabel('Total Power')
    axs[1].title.set_text('Total Power vs Time for Original FITS File For RFI Flagging')


    axs[2].imshow(new_pixel_data, aspect='auto', origin='lower', cmap='twilight', extent=[0, x_dimension*delta_x, 0, y_dimension*delta_y])
    axs[2].title.set_text('De-dispersed FITS File')

    axs[3].plot(time_xaxis_signal, total_power_vs_time_signal)
    axs[3].axhline(y=median_signal, color='r', linestyle='--', label='Median')
    axs[3].axhline(y=signal_threshold_median_above, color='k', linestyle=':', label='Upper Threshold')
    axs[3].axhline(y=signal_threshold_median_below, color='k', linestyle=':', label='Lower Threshold')
    axs[3].title.set_text('Total Power vs Time for De-dispersed FITS File For Signal Flagging')
    axs[3].set_xlabel('Time (ms)')  # Label x-axis only on the last subplot
    axs[3].set_ylabel('Total Power')
    
    # calculate SNR of maximum peak in de-dispersed total power:
    sorted_values=np.array(total_power_vs_time_signal)
    max_value=sorted_values.max()
    arg_max_value=sorted_values.argmax()
    sorted_values.sort()
    Q1 = np.quantile(sorted_values, 0.25)
    Q3 = np.quantile(sorted_values, 0.75)
    IQR = Q3 - Q1
    IQR_rms = IQR/1.35    
    median = sorted_values[int(len(sorted_values)/2)]   
    print("Dedispersed IQR_rms = %.4f, median = %.4f , max = $%.4f" % (IQR_rms,median,max_value))    
    snr = (max_value - median)/IQR_rms
    print("SNR = %.2f" % (snr))
    
    tt=("SNR = %.2f" % (snr))
    plt.text( arg_max_value, max_value*0.75, tt )

    plt.suptitle(f'Candidate:{fits_file}',fontsize=16, y = 0.95, horizontalalignment='center')
    plt.show()
#plotting(time_xaxis, total_power_vs_time, pixel_data, new_pixel_data, time_xaxis_signal, total_power_vs_time_signal, rfi_threshold_median_above,rfi_threshold_median_below, median, median_signal, signal_threshold_median_above, signal_threshold_median_below)

# %%
# Is there RFI?
# Main function which executes all above functions. 

def is_there_RFI(fits_file):
    x_dimension, y_dimension, delta_x, starting_frequency, delta_y, reference_x_pixel, pixel_data, delta_y, headers, data = process_fits_files(fits_file)
    if pixel_data is None or delta_x is None:
        print('There has been an error processing the FITS file')
    total_power_vs_time, time_xaxis, sorted_total_power_vs_time, median = power_vs_time(delta_x, pixel_data)
    rfi_events_spikes, rfi_events_block, rfi_threshold_median_above, rfi_threshold_median_below, median = threshold_median(time_xaxis, total_power_vs_time, 5)
    new_pixel_data, shift_arr, tdelay_arr, freq_arr = dedispersing(DM) 
    total_power_vs_time_signal, time_xaxis_signal = power_vs_time_signal(delta_x, new_pixel_data)
    signal_event, median_signal, signal_threshold_median_above, signal_threshold_median_below, rfi_block_in_signal = threshold_signal(time_xaxis_signal, total_power_vs_time_signal, IQR_multiplier_signal)
    RFI_or_not = is_RFI_signal(rfi_events_spikes, rfi_events_block,signal_event, rfi_block_in_signal)
    plotting(time_xaxis, total_power_vs_time, pixel_data, new_pixel_data, time_xaxis_signal, total_power_vs_time_signal, rfi_threshold_median_above,rfi_threshold_median_below, median, median_signal, signal_threshold_median_above, signal_threshold_median_below)
    return RFI_or_not
    
#is_there_RFI(fits_file)

if __name__ == "__main__":
    result = is_there_RFI(fits_file)
    print(result)

# %%



