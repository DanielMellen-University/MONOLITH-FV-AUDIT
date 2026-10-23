-------------------- MODULE AtomicWriters --------------------
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS Writers, None
ASSUME None \notin Writers
ASSUME Cardinality(Writers) = 2

VARIABLES Bytes, Synced, LockOwner, Target, CommitOrder
vars == <<Bytes, Synced, LockOwner, Target, CommitOrder>>

SetOfSeq(sequence) ==
    {sequence[index] : index \in 1..Len(sequence)}

Init ==
    /\ Bytes = [writer \in Writers |-> 0]
    /\ Synced = {}
    /\ LockOwner = None
    /\ Target = None
    /\ CommitOrder = <<>>

WriteChunk(writer) ==
    /\ writer \in Writers
    /\ writer \notin SetOfSeq(CommitOrder)
    /\ Bytes[writer] < 2
    /\ Bytes' = [Bytes EXCEPT ![writer] = @ + 1]
    /\ UNCHANGED <<Synced, LockOwner, Target, CommitOrder>>

SyncContent(writer) ==
    /\ writer \in Writers
    /\ writer \notin SetOfSeq(CommitOrder)
    /\ Bytes[writer] = 2
    /\ writer \notin Synced
    /\ Synced' = Synced \cup {writer}
    /\ UNCHANGED <<Bytes, LockOwner, Target, CommitOrder>>

AcquirePublishLock(writer) ==
    /\ writer \in Writers
    /\ Synced = Writers
    /\ writer \notin SetOfSeq(CommitOrder)
    /\ LockOwner = None
    /\ LockOwner' = writer
    /\ UNCHANGED <<Bytes, Synced, Target, CommitOrder>>

Publish(writer) ==
    /\ LockOwner = writer
    /\ writer \in Synced
    /\ Bytes[writer] = 2
    /\ writer \notin SetOfSeq(CommitOrder)
    /\ Target' = writer
    /\ CommitOrder' = Append(CommitOrder, writer)
    /\ UNCHANGED <<Bytes, Synced, LockOwner>>

ReleasePublishLock(writer) ==
    /\ LockOwner = writer
    /\ LockOwner' = None
    /\ UNCHANGED <<Bytes, Synced, Target, CommitOrder>>

Next ==
    \/ \E writer \in Writers: WriteChunk(writer)
    \/ \E writer \in Writers: SyncContent(writer)
    \/ \E writer \in Writers: AcquirePublishLock(writer)
    \/ \E writer \in Writers: Publish(writer)
    \/ \E writer \in Writers: ReleasePublishLock(writer)

Spec == Init /\ [][Next]_vars

TypeOK ==
    /\ Bytes \in [Writers -> 0..2]
    /\ Synced \subseteq Writers
    /\ LockOwner \in Writers \cup {None}
    /\ Target \in Writers \cup {None}
    /\ CommitOrder \in Seq(Writers)
    /\ Len(CommitOrder) <= Cardinality(Writers)
    /\ Cardinality(SetOfSeq(CommitOrder)) = Len(CommitOrder)

LockDiscipline ==
    LockOwner = None
        \/ (LockOwner \in Synced
            /\ Bytes[LockOwner] = 2)

CommittedContentComplete ==
    \A writer \in SetOfSeq(CommitOrder):
        writer \in Synced /\ Bytes[writer] = 2

PublicationOrder ==
    IF Len(CommitOrder) = 0
    THEN Target = None
    ELSE Target = CommitOrder[Len(CommitOrder)]

===============================================================
