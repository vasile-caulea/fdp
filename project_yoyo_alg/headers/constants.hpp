#pragma once

#define VOTE_BUFF_SIZE 2
#define INDEX_VOTE_RESULT 0
#define INDEX_PRUNE_RESULT 1
#define UNDEFINED (-1)

enum EdgeState {
    ACTIVE,
    PRUNED,
    PRUNED_WITH_LINK
};

enum NodeType {
    NO_TYPE,
    SOURCE,
    INTERNAL,
    SINK
};

enum MsgTag {
    MSG,
    SETUP,
    VOTE,
    LEADER
};

enum EdgeDirection {
    NO_DIRECTION,
    IN,
    OUT
};

enum Vote {
    YES,
    NO
};

enum PruneAction {
    NO_PRUNE,
    PRUNE,
    PRUNE_WITH_LINK
};
