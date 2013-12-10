#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define TAG 13

double**
alloc_matrix(int n)
{
  double **d = (double **) malloc(n * sizeof(double*));
  for (int k = 0; k < n; k++)
    {
      d[k] = (double *) malloc(n * sizeof(double));
    }
  return d;
}

int
count_lines(const char* file_name)
{
  FILE * pFile;
  int c;
  int n = 0;
  pFile = fopen(file_name, "r");
  if (pFile == NULL)
    perror("Error opening file");
  else
    {
      do
        {
          c = fgetc(pFile);
          if (c == '\n')
            n++;
        }
      while (c != EOF);
      fclose(pFile);
    }
  return n + 1;
}

//if dir == 0 then read simple matrix else read as transponded
double**
read_matrix(const char* file_name, int n, int dir)
{
  FILE* file = fopen(file_name, "r");
  char c = 0;
  fscanf(file, "%c", &c);
  double** d = alloc_matrix(n);

  int i = 0;
  int j = 0;
  while (!feof(file))
    {
      switch (c)
        {
      case '\n':
        i++;
        j = 0;
        break;
      case ',':
        j++;
        break;
      default:
        if (dir == 0)
          {
            d[i][j] = atof(&c);
          }
        else
          {
            d[j][i] = atof(&c);
          }
        break;
        }
      fscanf(file, "%c", &c);
    }
  fclose(file);
  return d;
}

void
print_matrix(double **m, int n)
{
  for (int i = 0; i < n; ++i)
    {
      for (int j = 0; j < n; ++j)
        {
          printf("%.1f ", m[i][j]);
        }
      printf("\n");
    }
}

double **A, **B, **C;
MPI_Status status;

int
main(int argc, char *argv[])
{
  int numElements, offset, stripSize, myrank, numnodes, N, i, j, k;
  long long start, end;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &numnodes);
  struct timeval tv;
  if (myrank == 0)
    {
      N = count_lines("m1.txt");

      A = read_matrix("m1.txt", N, 0);
      B = read_matrix("m2.txt", N, 0);

      print_matrix(A, N);
      C = alloc_matrix(N);
    }

  // allocate A, B, and C --- note that you want these to be
  // contiguously allocated.  Workers need less memory allocated.

  // start timer
  if (myrank == 0)
    {
      gettimeofday(&tv, NULL);
      start = tv.tv_usec;
    }

  stripSize = N / numnodes;

  // send each node its piece of A -- note could be done via MPI_Scatter
  if (myrank == 0)
    {
      offset = stripSize;
      numElements = stripSize * N;
      for (i = 1; i < numnodes; i++)
        {
          MPI_Send(A[offset], numElements, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD);
          offset += stripSize;
        }
    }
  else
    {  // receive my part of A
      MPI_Recv(A[0], stripSize * N, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
    }

  // everyone gets B
  MPI_Bcast(B[0], N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Let each process initialize C to zero
  for (i = 0; i < stripSize; i++)
    {
      for (j = 0; j < N; j++)
        {
          C[i][j] = 0.0;
        }
    }

  // do the work
  for (i = 0; i < stripSize; i++)
    {
      for (j = 0; j < N; j++)
        {
          for (k = 0; k < N; k++)
            {
              C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

  // master receives from workers  -- note could be done via MPI_Gather
  if (myrank == 0)
    {
      offset = stripSize;
      numElements = stripSize * N;
      for (i = 1; i < numnodes; i++)
        {
          MPI_Recv(C[offset], numElements, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
          offset += stripSize;
        }
    }
  else
    { // send my contribution to C
      MPI_Send(C[0], stripSize * N, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);
    }

  // stop timer
  if (myrank == 0)
    {
      gettimeofday(&tv, NULL);
      end = tv.tv_usec;
      printf("Time is %lld\n", (end - start));
    }

  // print out matrix here, if I'm the master
  if (myrank == 0 && N < 10)
    {
      for (i = 0; i < N; i++)
        {
          for (j = 0; j < N; j++)
            {
              printf("%f ", C[i][j]);
            }
          printf("\n");
        }
    }

  MPI_Finalize();
  return 0;
}
