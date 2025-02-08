##  Polinomial algorithm for consensus problem in presence of byzantine defects, in a system with n processing units


Compile with

```bash
mpic++ -o prog pol_con_byz.cpp
```

Run with

```bash
mpirun -np 5 --oversubscribe prog
```

### Pseudocode

- pi is the current process unit;

Precondition: n>4f, f number of defects \
Initially d[i] = x // i's decision for its inputs\
d[j] = V (implicit value), for any j!=i

```pseudo
Byzantine Consensus(pi, D, n, f, v) {
    Round 2k-1, 1<=k<=f+1 // first round of phase k 
    1. Send <d[i]> to all CPUs 
    2. Receive <vj> from pj, for any j!=i, and set d[j]=vj 
    3. Compute maj = majority of values from the set D={d[o],…,d[n-1]}; if it does not exist then maj=v (implicit value) 
    4. Compute mult = the number of occurrences of maj in D set

    Round 2k, 1<=k<=f+1 // the second round of phase k 
    1. If i==k then send <maj> to all CPUs 
    2. Receive <king-maj> from pk; if it does not exists then king-maj=v (implicit value) 
    3.  If mult>n/2+f then 
            d[i]=maj 
        else 
            d[i]=king-maj 
    4. If k==f+1 then 
        y=d[i]
}
```
