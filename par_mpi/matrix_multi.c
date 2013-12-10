// m3Sc 522
// MPI matrix multiplication example

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define Tm1G 13

double **m1, **m2, **m3;

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
          printf("%f ", m[i][j]);
        }
      printf("\n");
    }
}

int
main(int argc, char *argv[])
{

  double **m1, **m2, **m3;
  int n = count_lines("m1.txt");
  m1 = read_matrix("m1.txt", n, 0);
  m2 = read_matrix("m2.txt", n, 0);

  m3 = alloc_matrix(n);

  double startTime, endTime;
  int numElements, offset, stripSize, myrank, numnodes, N, i, j, k;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &numnodes);

  N = n;

  // allocate m1, m2, and m3 --- note that you want these to be
  // contiguously allocated.  Workers need less memory allocated.


  // start timer
  if (myrank == 0)
    {
      startTime = MPI_Wtime();
    }

  stripSize = N / numnodes;

  // send each node its piece of m1 -- note could be done via MPI_Scatter
  if (myrank == 0)
    {
      offset = stripSize;
      numElements = stripSize * N;
      for (i = 1; i < numnodes; i++)
        {
          MPI_Send(m1[offset], numElements, MPI_DOUBLE, i, Tm1G, MPI_COMM_WORLD);
          offset += stripSize;
        }
    }
  else
    {  // receive my part of m1
      MPI_Recv(m1[0], stripSize * N, MPI_DOUBLE, 0, Tm1G, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
    }

  // everyone gets m2
  MPI_Bcast(m2[0], N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Let each process initialize m3 to zero
  for (i = 0; i < stripSize; i++)
    {
      for (j = 0; j < N; j++)
        {
          m3[i][j] = 0.0;
        }
    }

  // do the work
  for (i = 0; i < stripSize; i++)
    {
      for (j = 0; j < N; j++)
        {
          for (k = 0; k < N; k++)
            {
              m3[i][j] += m1[i][k] * m2[k][j];
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
          MPI_Recv(m3[offset], numElements, MPI_DOUBLE, i, Tm1G, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
          offset += stripSize;
        }
    }
  else
    { // send my contribution to m3
      MPI_Send(m3[0], stripSize * N, MPI_DOUBLE, 0, Tm1G, MPI_COMM_WORLD);
    }

  // stop timer
  if (myrank == 0)
    {
      endTime = MPI_Wtime();
      printf("Time is %f\n", endTime - startTime);
    }

  // print out matrix here, if I'm the master
  if (myrank == 0 && N < 10)
    {
      for (i = 0; i < N; i++)
        {
          for (j = 0; j < N; j++)
            {
              printf("%f ", m3[i][j]);
            }
          printf("\n");
        }
    }

  MPI_Finalize();
  return 0;
}
