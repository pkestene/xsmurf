#!/usr/bin/env python

"""
Generate 2d fractional Brownian vector fields (fBm) as two numpy arrays.
Provide a routine to save in either matlab or xsmurf file format.

Use Hodge-Helmholtz decomposition to obtain the divergence free part.
"""
__author__ = "Pierre Kestener"
__license__ = "GPL"

# numerical packages
import numpy as np

# Fast Fourier Transform
from scipy.fftpack import fft, ifft, fftn, ifftn

def genFbm2d_scalar(nx,ny,h=0.5):
    """
    Generate 2D fBm scalar field (assume nx and ny are even)
    
    h : Holder exponent in [0, 1]
    if h == 0.5 : regular    Brownian motion
    if h != 0.5 : fractional Brownian motion
    """
    
    # initialize Fourier coef
    fftCoeff = np.zeros((nx,ny)).astype(complex) 

    print fftCoeff.flags
    print fftCoeff.shape
    
    # fill half Fourier, and the other half with complex conjugate
    for i in range(nx):

        # compute kx
        kx = i
        if i>nx/2:
            kx = i - nx
        
        # compute i2 (central symmetry)
        if (i==0):
            i2=0
        elif (i==nx/2):
            i2=nx/2
        else:
            i2=nx-i

        for j in range(ny/2+1):

            # compute ky
            ky = j

            # compute j2 (central symmetry)
            if (j==0):
                j2=0
            elif (j==ny/2):
                j2=ny/2
            else:
                j2=ny-j

            kSquare = 1.0*(kx**2+ky**2)
            if kSquare>0:
                radius = np.power(kSquare, -(2*h+2)/4) * np.random.normal()
                phase = 2 * np.pi * np.random.uniform()
            else:
                radius = 1.0
                phase  = 0.0

            # fill fourier coefficient so that ifft is real (imag = 0)
            fftCoeff[i ,j ] = radius*np.cos(phase) + 1j*radius*np.sin(phase)
            fftCoeff[i2,j2] = radius*np.cos(phase) - 1j*radius*np.sin(phase)

    # make sure that Fourier coef at i=0, j=ny/2 is real
    fftCoeff[0,ny/2] = np.real(fftCoeff[0,ny/2]) + 1j*0

    # make sure that Fourier coef at i=nx/2, j=0 is real
    fftCoeff[nx/2,0] = np.real(fftCoeff[nx/2,0]) + 1j*0

    # make sure that Fourier coef at i=nx/2, j=ny/2 is real
    fftCoeff[nx/2,ny/2] = np.real(fftCoeff[nx/2,ny/2]) + 1j*0

    return ifftn(fftCoeff)

def genFbm3d_scalar(nx,ny,nz,h=0.5):
    """
    Create 3D fBm scalar field (assume nx, ny, nz are even)
    
    h : Holder exponent in [0, 1]
    if h == 0.5 : regular    Brownian motion
    if h != 0.5 : fractional Brownian motion
    """
    
    # initialize Fourier coef
    fftCoeff = np.zeros((nx,ny,nz)).astype(complex) 

    # fill half Fourier space, and the other half using conplex conjugate
    for i in range(nx):

        # compute kx
        kx = i
        if i>nx/2:
            kx = i - nx

        # compute i2
        if (i==0):
            i2=0
        elif (i==nx/2):
            i2=nx/2
        else:
            i2=nx-i

        for j in range(ny):

            # compute ky
            ky = j
            if j>ny/2:
                ky = j - ny

            # compute j2
            if (j==0):
                j2=0
            elif (j==ny/2):
                j2=ny/2
            else:
                j2=ny-j

            for k in range(nz/2+1):

                # compute kz
                kz = k

                # compute k2
                if (k==0):
                    k2=0
                elif (k==nz/2):
                    k2=nz/2
                else:
                    k2=nz-k

                kSquare = 1.0*(kx**2+ky**2+kz**2)
                if kSquare>0:
                    radius = np.power(kSquare, -(2*h+3)/4) * np.random.normal()
                    phase = 2 * np.pi * np.random.uniform()
                else:
                    radius = 1.0
                    phase  = 0.0 

                # fill Fourier coef so that ifft is real
                fftCoeff[i ,j ,k ] = radius*np.cos(phase) + 1j*radius*np.sin(phase)
                fftCoeff[i2,j2,k2] = radius*np.cos(phase) - 1j*radius*np.sin(phase)

    # enforce symmetries for a real valued field
    # make sure that Fourier coef at i=nx/2 ... is real
    fftCoeff[nx/2,0   ,0   ] = np.real(fftCoeff[nx/2,0   ,0   ]) + 1j*0
    fftCoeff[0   ,ny/2,0   ] = np.real(fftCoeff[0   ,ny/2,0   ]) + 1j*0
    fftCoeff[0   ,0   ,nz/2] = np.real(fftCoeff[0   ,0   ,nz/2]) + 1j*0

    fftCoeff[nx/2,ny/2,0   ] = np.real(fftCoeff[nx/2,ny/2,0   ]) + 1j*0
    fftCoeff[nx/2,0   ,nz/2] = np.real(fftCoeff[nx/2,0   ,nz/2]) + 1j*0
    fftCoeff[0   ,ny/2,nz/2] = np.real(fftCoeff[0   ,ny/2,nz/2]) + 1j*0

    fftCoeff[nx/2,ny/2,nz/2] = np.real(fftCoeff[nx/2,ny/2,nz/2]) + 1j*0

    return ifftn(fftCoeff)

def compute_div_free_2d(vx,vy):
    """
    Compute Hodge-Helmholtz decomposition of the vector field (vx,vy) using a Fourier method.

    Hodge-Helmholtz theorem is
    V = -nabla(phi) + nabla ^ A

    So here, we compute W=nabla^A, using the Leray projector W=PV
    with P defined by P_kj = \delta_kj + (ksi_k ksi_j) / |ksi|^2

    return (wx,wy) : the div-free velocity projection
    """

    # retrieve array sizes
    nx,ny = vx.shape

    # compute forward FFT
    vx_fft = fftn(vx)
    vy_fft = fftn(vy)

    # initialize to zero the div free velocity field
    wx_fft = np.zeros((nx,ny)).astype(complex) 
    wy_fft = np.zeros((nx,ny)).astype(complex) 

    # fill half Fourier, and the other half with complex conjugate
    for i in range(nx):

        # compute kx
        kx = i
        if i>nx/2:
            kx = i - nx
        
        # compute i2
        if (i==0):
            i2=0
        elif (i==nx/2):
            i2=nx/2
        else:
            i2=nx-i

        for j in range(ny/2+1):

            # compute ky
            ky = j

            # compute j2
            if (j==0):
                j2=0
            elif (j==ny/2):
                j2=ny/2
            else:
                j2=ny-j

            kSquare = 1.0*(kx**2+ky**2)
            # don't do anything about the continuous component
            if kx==0 and ky==0:
                kSquare = 1.0

            # Leray tensor:
            m_00 =  ky**2/kSquare
            m_01 = -(kx*ky)/kSquare
            m_10 = -(kx*ky)/kSquare
            m_11 =  kx**2/kSquare
                
            # fill fourier coefficient so that inverse FFT is real
            tmp0 = m_00 * vx_fft[i,j] + m_01 * vy_fft[i,j]
            tmp1 = m_10 * vx_fft[i,j] + m_11 * vy_fft[i,j]

            wx_fft[i,  j ] = tmp0
            wx_fft[i2, j2] = np.conj(tmp0)
            wy_fft[i,  j ] = tmp1
            wy_fft[i2, j2] = np.conj(tmp1)
            
    # make sure that Fourier coef at i=0, j=ny/2 is real
    wx_fft[0,ny/2] = np.real(wx_fft[0,ny/2]) + 1j*0
    wy_fft[0,ny/2] = np.real(wy_fft[0,ny/2]) + 1j*0

    # make sure that Fourier coef at i=nx/2, j=0 is real
    wx_fft[nx/2,0] = np.real(wx_fft[nx/2,0]) + 1j*0
    wy_fft[nx/2,0] = np.real(wy_fft[nx/2,0]) + 1j*0

    # make sure that Fourier coef at i=nx/2, j=ny/2 is real
    wx_fft[nx/2,ny/2] = np.real(wx_fft[nx/2,ny/2]) + 1j*0
    wy_fft[nx/2,ny/2] = np.real(wy_fft[nx/2,ny/2]) + 1j*0

    # return the result div free velocity field
    return ifftn(wx_fft),ifftn(wy_fft)
    
def saveMatlab(filename_prefix,data):
    """
    Save data array in a file using Matlab file format.
    
    filename_prefix (no .mat)
    """

    # import routine for matlab file format
    import scipy.io as sio

    # save data
    sio.savemat(filename_prefix+'.mat', {filename_prefix:data})

def saveXsm(filename_prefix,data):
    """
    Save numpy 2d data array using xsmurf file format.

    Suffix .xsm will be added.
    """

    filename = filename_prefix + '.xsm'

    # get data shape
    nx,ny = data.shape

    # open file
    f = open(filename, 'w')

    # write the one line xsmurf header
    f.write("Binary 1 {0}x{1} {2}(4 byte reals)\n".format(nx,ny,nx*ny))
    
    # write heavy data
    data.astype(np.float32).tofile(f)

    # close file
    f.close()

def demo_plot(vx, vy, wx, wy):

    import matplotlib.pyplot as plt
    #Y,X = np.mgrid[0:nx, 0:ny]
    x = np.linspace(0,nx-1,nx)
    y = np.linspace(0,ny-1,ny)
    X,Y = np.meshgrid(x, y, indexing='ij')
    
    # for colormap
    v = np.sqrt(vx**2+vy**2)    
    w = np.sqrt(wx**2+wy**2)    

    # plot vx,vy
    plt.subplot(231)
    plt.imshow(vx)
    plt.title('vx')
    
    plt.subplot(232)
    plt.imshow(vy)
    plt.title('vy')

    plt.subplot(233)
    #plt.streamplot(X,Y,vx,vy,color=v,density=1.5)
    plt.streamplot(Y,X,vy,vx,color=v,density=1.5)
    plt.colorbar()
    plt.title('(vx,vy)')


    # plot wx,wy
    plt.subplot(234)
    plt.imshow(wx)
    plt.title('wx')
    
    plt.subplot(235)
    plt.imshow(wy)
    plt.title('wy')

    plt.subplot(236)
    #plt.streamplot(X,Y,wx,wy,color=w,density=1.5)
    plt.streamplot(Y,X,wy,wx,color=w,density=1.5)
    plt.colorbar()
    plt.title('(wx,wy)')

    plt.show()
    
def check_divergence(vx, vy, wx, wy):
    """
    Compute divergence of vector fields (vx,vy) and (wx,wy)
    and plot.
    """

    # compute divergence of (vx,vy)
    dxx,dxy=np.gradient(vx)
    dyx,dyy=np.gradient(vy)
    div_v = dxx+dyy
    
    # check that wx,wy is really divergence-free
    dxx,dxy=np.gradient(wx)
    dyx,dyy=np.gradient(wy)
    div_w = dxx+dyy

    L2_div_v = np.sum(div_v**2)**(0.5)
    L2_div_w = np.sum(div_w**2)**(0.5)
    print("average L2 norm of div(v) {}".format(L2_div_v))
    print("average L2 norm of div(w) {}".format(L2_div_w))
    
    # dxx+dyy should zero everywhere
    print("compute divergence")
    import matplotlib.pyplot as plt

    plt.subplot(211)
    plt.imshow(div_v)
    plt.colorbar()
    plt.title('divergence(v)')
    
    plt.subplot(212)
    plt.imshow(div_w)
    plt.colorbar()
    plt.title('divergence(w)')

    plt.show()
    
#
# main
#
if __name__ == '__main__':

    # parse command line
    from optparse import OptionParser

    parser = OptionParser()
    parser.add_option("-f", "--file", dest="filename",
                      default="fBm",
                      help="write output files with given prefix", metavar="FILE")
    parser.add_option("-s", "--size", dest="size",
                      default=64,
                      help="linear size of 3D data", type="int")
    parser.add_option("-H", "--hurst", dest="H",
                      default=0.5,
                      help="Hurst exponent of the fractional Broniaw motion", type="float")

    (options, args) = parser.parse_args()

    #print options.filename
    print 'Program run with args:\nfilename prefix={0}\nsize={1}\nH={2}'.format(options.filename, options.size, options.H)

    size = options.size
    nx = options.size
    ny = options.size
    H  = options.H
    
    # 2d test
    vx = np.real(genFbm2d_scalar(nx,ny))
    vy = np.real(genFbm2d_scalar(nx,ny))
    v = np.sqrt(vx**2+vy**2)
    
    # compute divergence free 
    wx,wy = compute_div_free_2d(vx,vy)
    wx = np.real(wx)
    wy = np.real(wy)
    #w = np.sqrt(wx**2+wy**2)

    # check that wx,wy is really divergence-free
    #check_divergence(vx,vy, wx,wy)
    
    # plot (vx,vy) and (wx,wy)
    #demo_plot(vx, vy, wx, wy)

    # save data (before divergence cleaning)
    saveXsm(options.filename+'_vx', vx)
    saveXsm(options.filename+'_vy', vy)

    # save data (after divergence cleaning)
    saveXsm(options.filename+'_wx', wx)
    saveXsm(options.filename+'_wy', wy)
