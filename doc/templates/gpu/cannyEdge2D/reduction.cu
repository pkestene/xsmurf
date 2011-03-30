#include <reduction_kernel.cu>
////////////////////////////////////////////////////////////////////////////////
// Wrapper function for kernel launch
////////////////////////////////////////////////////////////////////////////////
void reduce(int size, int threads, int blocks, int whichKernel, int *d_idata, int *d_odata)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);
    int smemSize = threads * sizeof(int);

	// choose which of the optimized versions of reduction to launch
    switch (whichKernel)
    {
    case 0:
        reduce0<<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata);
        break;
    case 1:
        reduce1<<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata);
        break;
    case 2:
        reduce2<<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata);
        break;
    case 3:
        reduce3<<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata);
        break;
    case 4:
        reduce4<<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata);
        break;
    case 5:
    default:
        switch (threads)
        {
        case 512:
            reduce5<512><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case 256:
            reduce5<256><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case 128:
            reduce5<128><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case 64:
            reduce5< 64><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case 32:
            reduce5< 32><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case 16:
            reduce5< 16><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case  8:
            reduce5<  8><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case  4:
            reduce5<  4><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case  2:
            reduce5<  2><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        case  1:
            reduce5<  1><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata); break;
        }
        break;       
    case 6:
        switch (threads)
        {
        case 512:
            reduce6<512><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 256:
            reduce6<256><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 128:
            reduce6<128><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 64:
            reduce6< 64><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 32:
            reduce6< 32><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 16:
            reduce6< 16><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  8:
            reduce6<  8><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  4:
            reduce6<  4><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  2:
            reduce6<  2><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  1:
            reduce6<  1><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        }
        break;       
    }
}

////////////////////////////////////////////////////////////////////////////////
// Compute the number of threads and blocks to use for the given reduction kernel
// For the kernels >= 3, we set threads / block to the minimum of maxThreads and
// n/2. For kernels < 3, we set to the minimum of maxThreads and n.  For kernel 
// 6, we observe the maximum specified number of blocks, because each thread in 
// that kernel can process a variable number of elements.
////////////////////////////////////////////////////////////////////////////////
void getNumBlocksAndThreads(int whichKernel, int n, int maxBlocks, int maxThreads, int &blocks, int &threads)
{
    if (whichKernel < 3)
    {
        threads = (n < maxThreads) ? n : maxThreads;
        blocks = n / threads;
    }
    else
    {
        if (n == 1) 
            threads = 1;
        else
            threads = (n < maxThreads*2) ? n / 2 : maxThreads;
        blocks = n / (threads * 2);

        if (whichKernel == 6)
            blocks = min(maxBlocks, blocks);
    }
}



////////////////////////////////////////////////////////////////////////////////
// This function performs a reduction of the input data multiple times and 
// measures the average reduction time.
////////////////////////////////////////////////////////////////////////////////
int computeSum(int  n, 
	       int  numThreads,
	       int  numBlocks,
	       int  maxThreads,
	       int  maxBlocks,
	       int  whichKernel, 
	       int* d_idata)
{
    int gpu_result = 0.0f;
    int *d_odata;

    CUDA_SAFE_CALL( cudaMalloc((void**) &d_odata, numBlocks*sizeof(int)) );

    gpu_result = 0.0f;
    
    // execute the kernel
    reduce(n, numThreads, numBlocks, whichKernel, d_idata, d_odata);
    
    // check if kernel execution generated an error
    CUT_CHECK_ERROR("Kernel execution failed");
    
    // sum partial block sums on GPU
    int s=numBlocks;
    int kernel = (whichKernel == 6) ? 5 : whichKernel;
    while(s > 1) 
      {
	int threads = 0, blocks = 0;
	getNumBlocksAndThreads(kernel, s, maxBlocks, maxThreads, blocks, threads);
	reduce(s, threads, blocks, kernel, d_odata, d_odata);
	if (kernel < 3)
	  s = s / threads;
	else
	  s = s / (threads*2);
      }
    CUDA_SAFE_CALL( cudaMemcpy( &gpu_result, d_odata, sizeof(int), cudaMemcpyDeviceToHost) );
    //gpu_result = d_odata[0];

    CUDA_SAFE_CALL(cudaFree(d_odata));

    return gpu_result;
}
