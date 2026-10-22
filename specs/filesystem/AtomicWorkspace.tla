-------------------- MODULE AtomicWorkspace --------------------

EXTENDS Naturals

VARIABLES Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
          ContentSynced, EntriesKnown, ParentLock, Phase,
          PublishedComplete, LastReclaimedOwner, LastReclaimedLease
vars == <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
         ContentSynced, EntriesKnown, ParentLock, Phase,
         PublishedComplete, LastReclaimedOwner, LastReclaimedLease>>

InitWriter ==
    /\ Target = "old"
    /\ Workspace = "none"
    /\ Owner = "none"
    /\ LeaseHeld = FALSE
    /\ ReadyMarker = FALSE
    /\ Bytes = 0
    /\ ContentSynced = FALSE
    /\ EntriesKnown = TRUE
    /\ ParentLock = "free"
    /\ Phase = "new"
    /\ PublishedComplete = FALSE
    /\ LastReclaimedOwner = "none"
    /\ LastReclaimedLease = FALSE

InitStaleOwned ==
    /\ Target = "old"
    /\ Workspace = "owned"
    /\ Owner = "valid"
    /\ LeaseHeld = FALSE
    /\ ReadyMarker = TRUE
    /\ Bytes = 1
    /\ ContentSynced = FALSE
    /\ EntriesKnown = TRUE
    /\ ParentLock = "free"
    /\ Phase = "stale"
    /\ PublishedComplete = FALSE
    /\ LastReclaimedOwner = "none"
    /\ LastReclaimedLease = FALSE

InitForeign ==
    /\ Target = "old"
    /\ Workspace = "foreign"
    /\ Owner = "invalid"
    /\ LeaseHeld = FALSE
    /\ ReadyMarker = TRUE
    /\ Bytes = 1
    /\ ContentSynced = FALSE
    /\ EntriesKnown = FALSE
    /\ ParentLock = "free"
    /\ Phase = "stale"
    /\ PublishedComplete = FALSE
    /\ LastReclaimedOwner = "none"
    /\ LastReclaimedLease = FALSE

Init == InitWriter \/ InitStaleOwned \/ InitForeign

AcquireSetupLock ==
    /\ Phase = "new"
    /\ ParentLock = "free"
    /\ ParentLock' = "writer"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

CreateWorkspace ==
    /\ Phase = "new"
    /\ ParentLock = "writer"
    /\ Workspace = "none"
    /\ Workspace' = "owned"
    /\ Owner' = "valid"
    /\ LeaseHeld' = TRUE
    /\ ReadyMarker' = TRUE
    /\ Bytes' = 0
    /\ ContentSynced' = FALSE
    /\ EntriesKnown' = TRUE
    /\ Phase' = "writing"
    /\ UNCHANGED <<Target, ParentLock, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

ReleaseSetupLock ==
    /\ Phase = "writing"
    /\ ParentLock = "writer"
    /\ ParentLock' = "free"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

WriteChunk ==
    /\ Phase = "writing"
    /\ Bytes < 2
    /\ Bytes' = Bytes + 1
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker,
                    ContentSynced, EntriesKnown, ParentLock, Phase,
                    PublishedComplete, LastReclaimedOwner, LastReclaimedLease>>

FinishStream ==
    /\ Phase = "writing"
    /\ Bytes = 2
    /\ Phase' = "prepared"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, ParentLock, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

SyncContent ==
    /\ Phase = "prepared"
    /\ ContentSynced' = TRUE
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    EntriesKnown, ParentLock, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

AcquirePublishLock ==
    /\ Phase = "prepared"
    /\ ContentSynced
    /\ ParentLock = "free"
    /\ ParentLock' = "writer"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

Publish ==
    /\ Phase = "prepared"
    /\ ParentLock = "writer"
    /\ Workspace = "owned"
    /\ Owner = "valid"
    /\ LeaseHeld
    /\ ReadyMarker
    /\ Bytes = 2
    /\ ContentSynced
    /\ Target = "old"
    /\ Target' = "new"
    /\ PublishedComplete' = TRUE
    /\ Phase' = "published"
    /\ UNCHANGED <<Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, ParentLock,
                    LastReclaimedOwner, LastReclaimedLease>>

FinishPublication ==
    /\ Phase = "published"
    /\ ParentLock = "writer"
    /\ Workspace' = "none"
    /\ Owner' = "none"
    /\ LeaseHeld' = FALSE
    /\ ReadyMarker' = FALSE
    /\ EntriesKnown' = TRUE
    /\ ParentLock' = "free"
    /\ Phase' = "done"
    /\ UNCHANGED <<Target, Bytes, ContentSynced, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

Abort ==
    /\ Phase \in {"writing", "prepared"}
    /\ Target = "old"
    /\ Workspace' = "none"
    /\ Owner' = "none"
    /\ LeaseHeld' = FALSE
    /\ ReadyMarker' = FALSE
    /\ Bytes' = 0
    /\ ContentSynced' = FALSE
    /\ EntriesKnown' = TRUE
    /\ ParentLock' = "free"
    /\ Phase' = "aborted"
    /\ UNCHANGED <<Target, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

CrashWriter ==
    /\ Phase \in {"writing", "prepared"}
    /\ Phase' = "stale"
    /\ LeaseHeld' = FALSE
    /\ ParentLock' = "free"
    /\ UNCHANGED <<Target, Workspace, Owner, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

CanReclaim ==
    /\ Workspace = "owned"
    /\ Owner = "valid"
    /\ ReadyMarker
    /\ ~LeaseHeld
    /\ EntriesKnown

AcquireCleanupLock ==
    /\ ParentLock = "free"
    /\ Workspace # "none"
    /\ ParentLock' = "cleanup"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

SkipScavenge ==
    /\ ParentLock = "cleanup"
    /\ Workspace # "none"
    /\ ~CanReclaim
    /\ ParentLock' = "free"
    /\ UNCHANGED <<Target, Workspace, Owner, LeaseHeld, ReadyMarker, Bytes,
                    ContentSynced, EntriesKnown, Phase, PublishedComplete,
                    LastReclaimedOwner, LastReclaimedLease>>

Scavenge ==
    /\ ParentLock = "cleanup"
    /\ CanReclaim
    /\ Workspace' = "none"
    /\ Owner' = "none"
    /\ LeaseHeld' = FALSE
    /\ ReadyMarker' = FALSE
    /\ EntriesKnown' = TRUE
    /\ ParentLock' = "free"
    /\ Phase' = "reclaimed"
    /\ LastReclaimedOwner' = Owner
    /\ LastReclaimedLease' = LeaseHeld
    /\ UNCHANGED <<Target, Bytes, ContentSynced, PublishedComplete>>

Next ==
    \/ AcquireSetupLock
    \/ CreateWorkspace
    \/ ReleaseSetupLock
    \/ WriteChunk
    \/ FinishStream
    \/ SyncContent
    \/ AcquirePublishLock
    \/ Publish
    \/ FinishPublication
    \/ Abort
    \/ CrashWriter
    \/ AcquireCleanupLock
    \/ SkipScavenge
    \/ Scavenge

Spec == Init /\ [][Next]_vars

TypeOK ==
    /\ Target \in {"old", "new"}
    /\ Workspace \in {"none", "owned", "foreign"}
    /\ Owner \in {"none", "valid", "invalid"}
    /\ LeaseHeld \in BOOLEAN
    /\ ReadyMarker \in BOOLEAN
    /\ Bytes \in 0..2
    /\ ContentSynced \in BOOLEAN
    /\ EntriesKnown \in BOOLEAN
    /\ ParentLock \in {"free", "writer", "cleanup"}
    /\ Phase \in {"new", "writing", "prepared", "published", "done",
                    "aborted", "stale", "reclaimed"}
    /\ PublishedComplete \in BOOLEAN
    /\ LastReclaimedOwner \in {"none", "valid", "invalid"}
    /\ LastReclaimedLease \in BOOLEAN

WorkspaceShape ==
    /\ (Workspace = "none" => /\ Owner = "none" /\ ~LeaseHeld)
    /\ (Workspace = "owned" => Owner = "valid")
    /\ (Workspace = "foreign" => Owner = "invalid")
    /\ (LeaseHeld => Workspace = "owned" /\ Owner = "valid" /\ ReadyMarker)

NoPartialPublication ==
    /\ Target \in {"old", "new"}
    /\ (Target = "new" => PublishedComplete)

PublishedOnlyAfterCompleteSync ==
    (PublishedComplete => /\ Target = "new"
                          /\ Bytes = 2
                          /\ ContentSynced)

ReclaimSafety ==
    /\ LastReclaimedOwner # "invalid"
    /\ ~LastReclaimedLease

===============================================================
