-------------------- MODULE TLCSmoke --------------------
EXTENDS Naturals

VARIABLE n

Init == n = 0

Next == \/ \/ n < 2 /\ n' = n + 1
        \/ \/ n = 2 /\ n' = n

Spec == Init /\ [][Next]_<<n>>

TypeOK == n \in 0..2

==========================================================
