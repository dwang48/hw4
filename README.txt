---pingpong.cpp---
command: mpiexec ./a.out 0 3
pingpong latency: 1.812190e-04 ms
pingpong bandwidth: 1.362704e+01 GB/s


---int_ring.c---
command: mpiexec ./a.out 48000 8
Message size: 0.000031 MB
Loops: 48000
Time  0.190488 seconds
Latency 0.000001 seconds
----------------
command: mpiexec ./a.out 96 524288
Message size: 2MB
Loops:96
Time  2.872636 seconds
Latency 0.000623 seconds
Estimated Bandwidth: 3.13301 GB/s
---jacobi-mpi.cpp---
command: mpiexec ./a.out 48 100
Number of Iterations:0
Residual:6.0208
Number of Iterations:10
Residual:2.51233
Number of Iterations:20
Residual:1.09513
Number of Iterations:30
Residual:0.477424
Number of Iterations:40
Residual:0.208134
Number of Iterations:50
Residual:0.0907367
Number of Iterations:60
Residual:0.0395569
Number of Iterations:70
Residual:0.0172449
Number of Iterations:80
Residual:0.00751798
Number of Iterations:90
Residual:0.00327748
Time elapsed is 0.001464 seconds.

---Plan---
Final Project Plan: I plan to work on the k-means clustering for the final project. I would like to find some commercial data or financial data and applies k-means clustering with CUDA to the datal. I am currently planning to work on the project on my own. 

