# Matrix Binarization — Parallel Computing Project

**Author:** Gregorio Bussolari

---

## Overview

This project implements the **binarization of a square matrix** using two parallel programming paradigms: **MPI** (Message Passing Interface) and **OpenMP** (Open Multi-Processing).

## Implementations

### MPI Implementation

- The **master node** (rank = 0) distributes portions of matrix `A` to all processes.
- Each process computes its corresponding rows of result matrix `T`.
- At the end, the master node aggregates all partial results using collective communication.

**Key design choices:**
- Matrices are allocated using **Row Major** order, initialized with random values, with `N ≥ 2000`.
- Work is distributed using `MPI_Scatterv` (variable-block scatter), which requires `sendcounts` and `displs` arrays.
- **Ghost rows** are exchanged between adjacent processes via `MPI_Sendrecv` (combined point-to-point, blocking, buffer-safe, deadlock-free).
- Partial results are gathered using `MPI_Gatherv` (variable-block gather).
- Border handling uses a **Mirror Index** strategy for both rows and columns.

### OpenMP Implementation

- Uses a **shared memory** model: matrices `A` and `T` are shared among all threads.
- Parallelism is declared via `#pragma omp parallel` with `num_threads(p)`.
- The workload is parallelized with `#pragma omp parallel for` and `schedule(static)` (round-robin chunk assignment).
- Each thread computes its assigned portion of `T` independently.
- No ghost row management needed due to shared memory access.
- Border handling also uses the **Mirror Index** strategy.

---

## Performance Analysis

Performance was measured via **strong** and **weak scalability** analysis for both implementations.

- **Strong scalability**: fixed `N = 10000`, increasing number of processors `p`.
- **Weak scalability**: starting from `N = 5000` at `p = 1`, problem size scaled as `N = 5000 × √p`.

### MPI — Strong Scalability

Speedup plateaus around `~4.3×` due to MPI communication overhead.

### OpenMP — Strong Scalability

Near-ideal speedup across all thread counts — efficiency consistently above 96%.

### MPI — Weak Scalability

Efficiency drops from `1.0` (p=1) to `~0.55` (p=48) as communication overhead grows with problem size.

### OpenMP — Weak Scalability

Efficiency remains extremely high (above 97%) across all configurations, demonstrating excellent scalability on shared memory.

---

## Conclusions

| Feature              | MPI                                      | OpenMP                            |
|----------------------|------------------------------------------|-----------------------------------|
| Memory model         | Distributed (message passing)            | Shared memory                     |
| Load balancing       | Manual (programmer's responsibility)     | Automatic (`schedule(static)`)    |
| Speedup              | Limited (~4× peak on single node)        | Near-ideal (up to ~46× at p=48)   |
| Efficiency           | Decreasing with more processes           | Consistently very high (>96%)     |
| Scalability          | High — runs on multi-node clusters       | Limited to cores of a single node |
| Portability          | Runs on distributed and shared memory    | Shared memory systems only        |

OpenMP achieves near-ideal speedup and efficiency within a single node, while MPI enables scaling across multiple nodes at the cost of communication overhead.

---

## Infrastructure

This project was developed and benchmarked on the **HPC Galileo 100** cluster at **[CINECA](https://www.cineca.it/)**.

- **Job scheduler:** [SLURM](https://slurm.schedmd.com/)
- **MPI tests:** single physical node, up to `--ntasks-per-node=48`
- **OpenMP tests:** single node, single task, up to `-c 48` CPU cores

---

## Additional Documentation

Further details on the implementation, algorithms, and benchmarks are available in:

```
docs/Presentation-1-1.pdf
```

---

## Requirements

- MPI library (e.g. Intel MPI, OpenMPI)
- OpenMP-compatible C compiler (e.g. `gcc`, `icc`)
- `N ≥ 2000` for meaningful benchmarking

---

## Build & Run (example)

```bash
# MPI
mpicc -O2 -o binarize_mpi binarize_mpi.c
mpirun -n <P> ./binarize_mpi <N>

# OpenMP
gcc -O2 -fopenmp -o binarize_omp binarize_omp.c
OMP_NUM_THREADS=<P> ./binarize_omp <N>
```

> For SLURM job submission scripts, refer to the examples in `docs/Presentation-1-1.pdf`.
