# Programming Project: Parallelize Particle Simulation

My implementation of the final project for the ID1217 Concurrent Programming course at KTH Royal Institute of Technology.

Acknowledgment: The project is adapted from the CS267 Assignment 2: Parallelize Particle Simulation of the CS267 Applications of Parallel Computers course at U.C. Berkeley, USA.

## Objectives

The main objectives of this assignment can be summarized as follows:

- To understand how to develop parallel applications in shared and distributed memory models;
- To get some practical skills in improving the performance of a parallel application by modifying its algorithm to find and exploit available parallelism or/and to improve the level of parallelism in the application;
- To understand how to implement a distributed multithreaded application in a distributed environment;
- To implement a multithreaded application.

## Task

Your task is to parallelize a toy particle simulator (similar particle simulators are used in mechanics, biology, astronomy, etc.) that reproduces the behavior shown in the following animation:

![animation](animation.gif)

The range of interaction forces is limited, as shown in grey for a selected
particle. Density is set sufficiently low so that given n particles, only $O(n)$
interactions are expected (compared with the time complexity $O(n^2)$ of a complete
gravitational N-body simulation).

Suppose we have a code that runs in time $T = O(n)$ on a single processor. Then
we'd hope to run in time T/p when using p processors. We'd like you to write
parallel codes that approach these expectations. Programs to Develop and
Evaluate

Given four particle simulation programs of $O(n^2)$ time complexity, you are to
improve the performance of the programs, i.e., develop and evaluate the
following four particle simulation programs.

1. A sequential program that runs in time $T = O(n)$, where n is the number of particles.
2. A parallel program using Pthreads that runs in time close to $T/p$ when using $p$ processors.
3. A parallel program using OpenMP (with/without tasks) that runs in time close to $T/p$ when using $p$ processors.
4. A parallel program using MPI that runs in time close to $T/p$ when using $p$ processors.
