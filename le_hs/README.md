## Hirschberg Sinclair algorithm for leader election

The algorithm runs on networks based on **ring communication topology**.


Compile with

```bash
mpic++ -o prog leader_el_HS.cpp
```

Run with

```bash
mpirun -np 7 --oversubscribe prog
```

### Pseudocode

- pi is the current process unit;
- ph is the current phase;
- counter is the iterator that goes from 1 to 2<sup>ph</sup> ;
- id is the identificator for the current process unit;

```pseudo
Leader_Election_Hirschberg_Sinclair(pi) { 
while true do switch 
    case of receiving no message: 
        if asleep == false then 
            send <probe, id, 0, 1> to left and right // ph = 0, counter = 1
    
    case of receiving <probe, id_j, ph, counter> from left (respectively, right): 
        if id_j == id then 
            send <leader, id_j> to left

        if id_j > id and counter < 2^ph then /* forward the message */ 
            send <probe, id_j, ph, counter+1> to right (respectively, left) 
        
        if id_j > id and counter >= 2^ph then /* reply to the message */ 
            send <reply, id_j, ph> to left (respectively, right) 
        
        /* if id_j < id, the message is swallowed */
    
    case of receiving <reply, id_j, ph> from left (respectively, right): 
        if id_j != id then /* forward the reply */ 
            send <reply, id_j, ph> to right (respectively, left)
        
        else /* reply is for own probe */ 
            if already received <reply, id_j, ph> from right (respectively, left) then 
                send <probe, id, ph+1, 1> /* phase ph winner */ 
    
    case of receiving <leader, id_j> from right: 
        if id_j != id then 
            send <leader, id_j> to left 
            terminate
}
```
