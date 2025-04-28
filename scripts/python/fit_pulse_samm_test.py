import numpy as np
from scipy.special import erf, erfc, erfcx
import astropy.units as u
from astropy.io import fits
from astropy.time import Time
from astropy.coordinates import SkyCoord, EarthLocation

π = np.pi

def gaussian_cdf(t, A, μ, σ):
    '''
    ... not *quite* CDF of a Gaussian, since this is scaled up by A.
    '''
    # return -np.sqrt(π/2)*A*σ*erf((μ - t)/np.sqrt(2)/σ)
    
    # 2025-04-28 updated :
    return A*0.5*(1 + erf((t - μ)/np.sqrt(2)/σ))

def discrete_gaussian(t, A, μ, σ):
    #return A*np.exp(-0.5*(x - μ)**2 / σ**2)
    # Integrating over a bin width lets us preserve fluence when fitting

    dt = t[1] - t[0]
    return (gaussian_cdf(t + dt/2, A, μ, σ) - gaussian_cdf(t - dt/2, A, μ, σ)) / dt

# 2025-04-28 updated :
def emg_cdf(t, A, μ, σ, τ):
    '''
    ... not *quite* CDF of an exponentially modified Gaussian, since this is scaled up by A.
    '''
    λ = 1/τ
    arg1 = (μ + λ*σ**2 - t)/(np.sqrt(2)*σ)
    arg2 = λ/2 * (2*μ + λ*σ**2 - 2*t)

    # The following two are mathematically equivalent, but computationally different in
    # how they are subject to problems of overflow
    #return A*(gaussian_cdf(t, 1, μ, σ) - 0.5*np.exp(arg2) * erfc(arg1))
    return A*(gaussian_cdf(t, 1, μ, σ) - 0.5*np.exp(arg2 - arg1**2) * erfcx(arg1))

def discrete_emg(t, A, μ, σ, τ):
    '''
    Exponentially modified gaussian
    '''
    dt = t[1] - t[0]
    pdf_value = (emg_cdf(t + dt/2, A, μ, σ, τ) - emg_cdf(t - dt/2, A, μ, σ, τ)) / dt
    #print(f'{A = }, {μ = }, {σ = }, {τ = }, {pdf_value = }')
    return pdf_value
