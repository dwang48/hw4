HW=4
hw:  hw$(HW)
hw4: compile


clean:
	rm -f int_ring jacobi-mpi pingpong


compile:
	mpic++ int_ring.c -o int_ring
	mpic++ jacobi-mpi.cpp -o jacobi-mpi
	mpic++ pingpong.cpp -o pingpong



