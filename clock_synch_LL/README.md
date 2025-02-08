##  Lundelius-Lynch algorithm for clock synchronization 

Compile with

```bash
mpic++ -o prog clock_synch_LL.cpp
```

Run with

```bash
mpirun -np 5 --oversubscribe prog
```

### Pseudocode

```pseudo
Notations: 
- difference[0..n-1] an unidimensional array of size n. 
- sender the index of the processing unit from which pi receives the message T. 
Premise: 
- Initially, difference[i] = 0; i=1..n-1 
Results: 
- The differences between the adjusted clocks of pi and pj will be stored in 
difference[j], j=1..n-1. 
- The adjust variable will store the adjust value of pi's clock. 

Pseudocode for processing unit pi, i=0..n-1 
Lundelius Lynch SC(n,d,u,pi) {
    send HC (current hardware clock value) to all processing units 
    for j = 0 to n-1 do 
        wait a message T /*T = hardware clock value sent by a processing unit*/ 
        when T arrive, identify the sender 
            difference[sender] = T + d – u/2 - HC
    
    for j = 0 to n-1 do 
        adjust = adjust + difference[j] 
    adjust = adjust/n 
}
```
