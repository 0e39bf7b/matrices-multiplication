/*
 * gcc -DCLS=$(getconf LEVEL1_DCACHE_LINESIZE) -Wall -O2 mem-speed.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SM (CLS / sizeof (double))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void matrix_multiply(size_t sz, double **mul1, double **mul2, double **res) {
  size_t i, j, k;

  for (i = 0; i < sz; i++) {
    for (j = 0; j < sz; j++) {
      for (k = 0; k < sz; k++) {
        res[i][j] += mul1[i][k] * mul2[k][j];
      }
    }
  }
}

void matrix_multiply_t(size_t sz, double **mul1, double **mul2, double **res) {
  size_t i, j, k;
  double tmp;

  // Transpose mul2 matrix before further computations, so it will be accessed sequentially
  for (i = 0; i < sz; i++) {
    for (j = i; j < sz; j++) {
      tmp = mul2[i][j];
      mul2[i][j] = mul2[j][i];
      mul2[j][i] = tmp;
    }
  }

  for (i = 0; i < sz; i++) {
    for (j = 0; j < sz; j++) {
      for (k = 0; k < sz; k++) {
        res[i][j] += mul1[i][k] * mul2[j][k];
      }
    }
  }
}

void matrix_multiply_fast(size_t sz, double **mul1, double **mul2, double **res) {
  size_t i, i2, j, j2, k, k2;

  for (i = 0; i < sz; i += SM) {
    for (j = 0; j < sz; j += SM) {
      for (k = 0; k < sz; k += SM) {
        size_t i2_max = MIN(SM, sz - i);
        for (i2 = 0; i2 < i2_max; i2++) {
          size_t k2_max = MIN(SM, sz - k);
          for (k2 = 0; k2 < k2_max; k2++) {
            size_t j2_max = MIN(SM, sz - j);
            for (j2 = 0; j2 < j2_max; j2++) {
              res[i + i2][j + j2] += mul1[i + i2][k + k2] * mul2[k + k2][j + j2];
            }
          }
        }
      }
    }
  }
}

void matrix_multiply_seq(size_t sz, double **mul1, double **mul2, double **res) {
  size_t i, i2, j, j2, k, k2;

  double *rres, *rmul1, *rmul2;

  for (i = 0; i < sz; i += SM) {
    for (j = 0; j < sz; j += SM) {
      for (k = 0; k < sz; k += SM) {
        size_t i2_max = MIN(SM, sz - i);
        for (i2 = 0, rres = &res[i][j], rmul1 = &mul1[i][k]; i2 < i2_max; ++i2, rres += sz, rmul1 += sz) {
          size_t k2_max = MIN(SM, sz - k);
          for (k2 = 0, rmul2 = &mul2[k][j]; k2 < k2_max; ++k2, rmul2 += sz) {
            size_t j2_max = MIN(SM, sz - j);
            for (j2 = 0; j2 < j2_max; ++j2) {
              rres[j2] += rmul1[k2] * rmul2[j2];
            }
          }
        }
      }
    }
  }
}

void check_eql(size_t sz, double **m1, double **m2) {
  size_t i, j;

  for (i = 0; i < sz; i++) {
    for (j = 0; j < sz; j++) {
      if (m1[i][j] != m2[i][j]) {
        printf("m1[%ld][%ld] = %f, m2[%ld][%ld] = %f\n", i, j, m1[i][j], i, j, m2[i][j]);
        return;
      }
    }
  }

  printf("m1 = m2\n");
}

double **allocate_matrix(size_t sz) {
  double **res = malloc(sz * sizeof(double*));

  size_t i, j;

  for (i = 0; i < sz; i++) {
    res[i] = malloc(sz * sizeof(double));
    for (j = 0; j < sz; j++) {
      res[i][j] = 0;
    }
  }

  return res;
}

double **allocate_matrix_seq(size_t sz) {
  double *data = malloc(sz * sz * sizeof(double));
  double **res = malloc(sz * sizeof(double*));

  size_t i;

  for (i = 0; i < sz; i++) {
    res[i] = data + i * sz;
  }

  return res;
}

void fill_matrix(size_t sz, double **m) {
  size_t i, j;

  for (i = 0; i < sz; i++) {
    for (j = 0; j < sz; j++) {
      m[i][j] = rand();
    }
  }
}

void copy_matrix(size_t sz, double **dst, double **src) {
  size_t i, j;

  for (i = 0; i < sz; i++) {
    for (j = 0; j < sz; j++) {
      dst[i][j] = src[i][j];
    }
  }
}

void free_matrix(size_t sz, double **m) {
  size_t i;

  for (i = 0; i < sz; i++) {
    free(m[i]);
  }

  free(m);
}

void free_matrix_seq(double **m) {
  free(m[0]);
  free(m);
}

void measure_speed(size_t sz) {
  clock_t start, t1, t2, t3, t4;

  printf("sz = %ld\n", sz);

  printf("allocate_matrix\n");
  double **mul1 = allocate_matrix(sz);
  double **mul2 = allocate_matrix(sz);
  double **res1 = allocate_matrix(sz);
  double **res2 = allocate_matrix(sz);
  double **res3 = allocate_matrix(sz);

  double **mul1_seq = allocate_matrix_seq(sz);
  double **mul2_seq = allocate_matrix_seq(sz);
  double **res_seq = allocate_matrix_seq(sz);

  printf("fill_matrix\n");
  fill_matrix(sz, mul1);
  fill_matrix(sz, mul2);
  copy_matrix(sz, mul1_seq, mul1);
  copy_matrix(sz, mul2_seq, mul2);

  printf("matrix_multiply...\n");
  start = clock();
  matrix_multiply(sz, mul1, mul2, res1);
  t1 = clock() - start;
  printf("finished in %ld\n", t1);

  printf("matrix_multiply_fast...\n");
  start = clock();
  matrix_multiply_fast(sz, mul1, mul2, res2);
  t2 = clock() - start;
  printf("finished in %ld\n", t2);
  check_eql(sz, res1, res2);

  printf("matrix_multiply_seq...\n");
  start = clock();
  matrix_multiply_seq(sz, mul1_seq, mul2_seq, res_seq);
  t3 = clock() - start;
  printf("finished in %ld\n", t3);
  check_eql(sz, res1, res_seq);

  printf("matrix_multiply_t...\n");
  start = clock();
  matrix_multiply_t(sz, mul1, mul2, res3);
  t4 = clock() - start;
  printf("finished in %ld\n", t4);
  check_eql(sz, res1, res3);

  printf("free_matrix\n\n");
  free_matrix(sz, mul1);
  free_matrix(sz, mul2);
  free_matrix(sz, res1);
  free_matrix(sz, res2);
  free_matrix(sz, res3);
  free_matrix_seq(mul1_seq);
  free_matrix_seq(mul2_seq);
  free_matrix_seq(res_seq);
}

int main(int argc, char **argv) {
  srand(time(NULL));

  printf("SM = %ld\n\n", SM);

  measure_speed(100);
  measure_speed(300);
  measure_speed(500);
  measure_speed(1000);
  measure_speed(2000);
  measure_speed(3000);

  return 0;
}
