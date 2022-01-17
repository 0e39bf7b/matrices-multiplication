# Matrices multiplication optimizations

This repo contains the code that shows how optimizations of level 1 data cache access can influence
matrices multiplication performance. I got some ideas from the famous article
[What Every Programmer Should Know About Memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf).

More explanation can be found in my article
[How Much Access to RAM Costs](https://0e39bf7b.blog/posts/how-much-access-to-ram-costs/).

## Compilation and running

```
gcc -DCLS=$(getconf LEVEL1_DCACHE_LINESIZE) -Wall -O2 mem-speed.c
./a.out
```

## Results

The results I got on Ubuntu 20.04 with Intel Core i7-10700K CPU and 3000 MHz DDR4 memory:

`matrix_multiply`:

|Matrix size|execution time|
|-----------|--------------|
|100        |733           |
|300        |21485         |
|500        |101610        |
|1000       |843546        |
|2000       |25787985      |
|3000       |107763896     |

`matrix_multiply_t`:

|Matrix size|execution time|% of `matrix_multiply`|
|-----------|--------------|----------------------|
|100        |719           |98.09%                |
|300        |20885         |97.21%                |
|500        |96832         |95.30%                |
|1000       |788834        |93.51%                |
|2000       |7268999       |28.19%                |
|3000       |24179636      |22.44%                |

`matrix_multiply_fast`:

|Matrix size|execution time|% of `matrix_multiply`|
|-----------|--------------|----------------------|
|100        |606           |82.67%                |
|300        |15503         |72.16%                |
|500        |72558         |71.41%                |
|1000       |600223        |71.15%                |
|2000       |5203588       |20.18%                |
|3000       |17501126      |16.24%                |

`matrix_multiply_seq`:

|Matrix size|execution time|% of `matrix_multiply`|
|-----------|--------------|----------------------|
|100        |459           |62.62%                |
|300        |11773         |54.80%                |
|500        |56441         |55.55%                |
|1000       |480246        |56.93%                |
|2000       |4164429       |16.15%                |
|3000       |14001852      |12.99%                |
