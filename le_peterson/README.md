## Peterson’s algorithm for leader election

The algorithm runs on networks based on **ring communication topology**.

Compile with

```bash
mpic++ -o prog le_peterson.cpp
```

Run with

```bash
mpirun -np 6 --oversubscribe prog
```

### Pseudocode

- pi is the current process unit;
- ph is the current phase;
- counter is the iterator that goes from 1 to 2<sup>ph</sup> ;
- id is the identificator for the current process unit;

```pseudo
Leader_Election_Peterson(pi) { 
while true do switch

    case of receiving no message:
        if asleep == true and relay == false then
            asleep = false
            max_id = i
            send <"new_id"; max_id> to lef

    case of receiving <"new_id"; id> from right:
        if relay = true then
            send <"new_id"; id> to left /* forward the message */

        else if first_id == null then
            first_id = id
            if max_id == first_id then
                send <"leader"; max_id; i> to left /* max_id is leader and pi is the announcer */
            else
                send <"new_id"; first_id> to left
        else
            second_id = id
            if first_id > max(max_id; second_id) then
                max_id  = first_id
                send <"new_id"; max_id; i> to left /* pi continues to be active */
                first_id = null
                second_id = null
            else
                relay = true /* pi becomes relay */
    
    case of receiving <"leader"; id; announcer> from right:
        leader = id
        if announcer != i then
            send <"leader"; leader; announcer> to left /* forward the message */
        terminate
}
```
