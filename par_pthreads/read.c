#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

double **m1, **m2, **m3;
int N_SIZE = 0;
int NUM_THREADS = 0;

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

void
mm(int id)
{
  int i, j, k;
  double sum;

  // compute bounds for this threads---just algebra
  int startrow = id * N_SIZE / NUM_THREADS;
  int endrow = (id + 1) * (N_SIZE / NUM_THREADS) - 1;

  // matrix mult over the strip of rows for this thread
  for (i = startrow; i <= endrow; i++)
    {
      for (j = 0; j < N_SIZE; j++)
        {
          sum = 0.0;
          for (k = 0; k < N_SIZE; k++)
            {
              sum = sum + m1[i][k] * m2[i][k];
            }
          m3[i][j] = sum;
        }
    }
}

void *
worker(void *arg)
{
  int id = *((int *) arg);
  mm(id);
}

int
main(int argc, char **argv)
{
//  if (argc != 4)
//    {
//      printf(
//          "Usage:%s set two input files and one output, and number of threads \n",
//          argv[0]);
//      exit(1);
//    }
  NUM_THREADS = 100;

  int n = count_lines("m1.txt");
  if (n < NUM_THREADS)
    {
      NUM_THREADS = n;
    }
  pthread_t *threads;
  int *p;
  N_SIZE = n;
  m1 = read_matrix("m1.txt", n, 0);
  m2 = read_matrix("m2.txt", n, 1);

  m3 = alloc_matrix(n);

  // Allocate thread handles
  threads = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
  for (int i = 0; i < NUM_THREADS; i++)
    {
      p = (int *) malloc(sizeof(int));  // yes, memory leak, don't worry for now
      *p = i;
      pthread_create(&threads[i], NULL, worker, (void *) (p));
    }

  for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_join(threads[i], NULL);
    }

  print_matrix(m3, n);
  return 0;
}

