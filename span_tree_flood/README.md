## Building a spanning tree using the flooding technique

The algorithm runs on a connex graph that represents the communication topology.

Compile with

```bash
mpic++ -o prog spanning_tree.cpp
```

Run with

```bash
mpirun -np 6 prog
```

### Pseudocode

- pi is the current process unit;
- pr is the root process which contains the initial message M.

```
Flooding_SpanningTree(pi, pr) { 
while true do switch
    case of receiving no message:
        if pi == pr and parent == NULL then
            parent = pi
            send <M> to all neighbors

    case of receiving <M> from pj: 
        if parent == NULL then
            parent = pj
            send <parent> to parent
            send <M> to all neighbors, except parent
        else
            sent <already> to pj
    
    case of receiving <parent> from pj: 
        add pj to children
        if children U other contains all neighbors, except parent then
            terminate
    
    case of receiving <already> from pj: 
        add pj to other
        if children U other contains all neighbors, except parent then
            terminate
}
```
