#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#define N 10000000
#define MASTER 0
int * merge(int *A, int asize, int *B, int bsize);
void m_sort(int *A, int min, int max);
double startT, stopT; double startTime;
int * merge(int *A, int asize, int *B, int bsize)
{
    int ai, bi, ci, i;
    int * C;
    int csize = asize+bsize;
    ai = 0;
    bi = 0;
    ci = 0;
    C = (int *)malloc(csize*sizeof(int));
    while((ai<asize)&(bi<bsize))
    {
        if(A[ai]<B[bi])
        {
            C[ci] = A[ai]; ai++;ci++;
        }
        else
        {
            C[ci] = B[bi]; bi++;ci++;
        }
    }
    if(ai >= asize)
        for(i=ci;i<csize;i++,bi++)
            C[i] = B[bi];
    else if(bi >= bsize)
        for(i=ci;i < csize; i++, ai++)
            C[i] = A[ai];
    for(i=0; i < asize;i++)
        A[i] = C[i];
    for(i=0; i < bsize; i++)
        B[i] = C[asize + i];
    return C;
}
void m_sort(int *A, int min, int max)
{
    int * C;
    int mid = (min + max )/2;
    int left = mid - min + 1;
    int right = max - mid;
    if(max != min)
    {
        m_sort(A, min, mid);
        m_sort(A, mid+1, max);
        C = merge(A + min, left, A + mid + 1, right);
    }
}
int main(int argc, char* argv[])
{
    int * data;
    int * blk;
    int * temp;
    int m, n = N;
    int id, p;
    int s = 0;
    int i;
    int step;
    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    startT = MPI_Wtime();
    if(id == MASTER)
    {
        int r;
        srandom(MPI_Wtime());
        s = n/p;
        r = n%p;
        data = (int *)malloc((n+s-r)*sizeof(int));
        for(i=0;i<n;i++)
            data[i] = random();
        if(r != 0)
        {
            for(i=n;i<n+s-r;i++)
                data[i]=0; s++;
        }
        MPI_Bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);
        blk = (int *)malloc(s*sizeof(int));
        MPI_Scatter(data,s,MPI_INT,blk,s,MPI_INT,0,MPI_COMM_WORLD);
        m_sort(blk, 0, s-1);
    }
    else
    {
        MPI_Bcast(&s,1,MPI_INT,0,MPI_COMM_WORLD);
        blk = (int *)malloc(s*sizeof(int));
        MPI_Scatter(data,s,MPI_INT,blk,s,MPI_INT,0,MPI_COMM_WORLD);
        m_sort(blk, 0, s-1);
    }
    step = 1;
    while(step < p)
    {
        if(id%(2*step)==0)
        {
            if(id + step < p)
            {
                MPI_Recv(&m,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                temp = (int *)malloc(m*sizeof(int));
                MPI_Recv(temp,m,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                blk = merge(blk,s,temp,m);
                s += m;
            }
        }
        else
        {
            int near = id-step;
            MPI_Send(&s,1,MPI_INT,near,0,MPI_COMM_WORLD);
            MPI_Send(blk,s,MPI_INT,near,0,MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }
    stopT = MPI_Wtime();
    printf(" %d; %d processors; %f secs\n", s, p, (stopT-startT));
    MPI_Finalize();
    return 0;
}
