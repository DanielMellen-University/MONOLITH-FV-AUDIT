-------------------- MODULE WindowLifecycle --------------------
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS Windows, None
ASSUME None \notin Windows

VARIABLES Seen, Live, Order, Focused, Minimized, Pending, CallbackStack
vars == <<Seen, Live, Order, Focused, Minimized, Pending, CallbackStack>>

RemoveFromOrder(order, window) ==
    SelectSeq(order, LAMBDA candidate: candidate # window)

SetOfSeq(sequence) ==
    {sequence[index] : index \in 1..Len(sequence)}

IndexOf(order, window) ==
    CHOOSE index \in 1..Len(order): order[index] = window

FocusOf(order, visible) ==
    IF visible = {} THEN None
    ELSE CHOOSE window \in visible:
        \A other \in visible: IndexOf(order, window) >= IndexOf(order, other)

Init ==
    /\ Seen = {}
    /\ Live = {}
    /\ Order = <<>>
    /\ Focused = None
    /\ Minimized = {}
    /\ Pending = {}
    /\ CallbackStack = <<>>

CreateWindow(window) ==
    /\ window \in Windows \ Seen
    /\ Seen' = Seen \cup {window}
    /\ Live' = Live \cup {window}
    /\ Order' = Append(Order, window)
    /\ Focused' = window
    /\ Minimized' = Minimized
    /\ Pending' = Pending
    /\ CallbackStack' = CallbackStack

BeginCallback(window) ==
    /\ window \in Live
    /\ Len(CallbackStack) < 2
    /\ CallbackStack' = Append(CallbackStack, window)
    /\ UNCHANGED <<Seen, Live, Order, Focused, Minimized, Pending>>

EndCallback ==
    /\ Len(CallbackStack) > 0
    /\ CallbackStack' = Tail(CallbackStack)
    /\ UNCHANGED <<Seen, Live, Order, Focused, Minimized, Pending>>

CloseNow(window) ==
    /\ window \in Live
    /\ LET nextLive == Live \ {window}
           nextOrder == RemoveFromOrder(Order, window)
           nextMinimized == Minimized \ {window}
       IN
           /\ Live' = nextLive
           /\ Order' = nextOrder
           /\ Minimized' = nextMinimized
           /\ Focused' = FocusOf(nextOrder, nextLive \ nextMinimized)
    /\ UNCHANGED <<Seen, Pending, CallbackStack>>

RequestClose(window) ==
    /\ window \in Live
    /\ (Len(CallbackStack) > 0 \/ Pending = {})
    /\ IF Len(CallbackStack) > 0
          THEN
              /\ Pending' = Pending \cup {window}
              /\ UNCHANGED <<Seen, Live, Order, Focused, Minimized, CallbackStack>>
          ELSE CloseNow(window)

ProcessDeferred(window) ==
    /\ Len(CallbackStack) = 0
    /\ window \in Pending
    /\ Pending' = Pending \ {window}
    /\ IF window \in Live
          THEN
              /\ LET nextLive == Live \ {window}
                     nextOrder == RemoveFromOrder(Order, window)
                     nextMinimized == Minimized \ {window}
                 IN
                     /\ Live' = nextLive
                     /\ Order' = nextOrder
                     /\ Minimized' = nextMinimized
                     /\ Focused' = FocusOf(nextOrder, nextLive \ nextMinimized)
              /\ UNCHANGED <<Seen, CallbackStack>>
          ELSE UNCHANGED <<Seen, Live, Order, Focused, Minimized, CallbackStack>>

MinimizeWindow(window) ==
    /\ window \in Live
    /\ LET nextMinimized == Minimized \cup {window}
       IN
           /\ Minimized' = nextMinimized
           /\ Focused' = FocusOf(Order, Live \ nextMinimized)
    /\ UNCHANGED <<Seen, Live, Order, Pending, CallbackStack>>

RestoreAndRaise(window) ==
    /\ window \in Live
    /\ Minimized' = Minimized \ {window}
    /\ Order' = Append(RemoveFromOrder(Order, window), window)
    /\ Focused' = window
    /\ UNCHANGED <<Seen, Live, Pending, CallbackStack>>

Next ==
    \/ \E window \in Windows: CreateWindow(window)
    \/ \E window \in Live: BeginCallback(window)
    \/ EndCallback
    \/ \E window \in Live: RequestClose(window)
    \/ \E window \in Windows: ProcessDeferred(window)
    \/ \E window \in Live: MinimizeWindow(window)
    \/ \E window \in Live: RestoreAndRaise(window)

Spec == Init /\ [][Next]_vars

TypeOK ==
    /\ Seen \subseteq Windows
    /\ Live \subseteq Seen
    /\ Order \in Seq(Windows)
    /\ SetOfSeq(Order) = Live
    /\ Len(Order) = Cardinality(Live)
    /\ Focused \in {None} \cup Live
    /\ Minimized \subseteq Live
    /\ Pending \subseteq Live
    /\ CallbackStack \in Seq(Windows)
    /\ Len(CallbackStack) <= 2
    /\ SetOfSeq(CallbackStack) \subseteq Live

FocusInvariant ==
    /\ Focused = FocusOf(Order, Live \ Minimized)

===============================================================
