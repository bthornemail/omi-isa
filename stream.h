#ifndef OMI_STREAM_H
#define OMI_STREAM_H

#include "omienv.h"

typedef enum {
    STREAM_STATE_WAITING,
    STREAM_STATE_HEADER,
    STREAM_STATE_PAYLOAD,
    STREAM_STATE_COMPLETE,
    STREAM_STATE_INVALID
} StreamState;

typedef struct {
    StreamState state;
    uint8_t buffer[OMI_ENV_SIZE];
    size_t filled;
    int gauge_match_count;
    OMI_512_Envelope envelope;
    uint64_t bitboard;
    int validation_result;
} StreamParser;

typedef struct {
    uint64_t bitboard;
    OMI_512_Envelope envelope;
    int valid;
    int cell_index;
} StreamEvent;

void stream_parser_init(StreamParser* sp);
void stream_push_byte(StreamParser* sp, uint8_t byte);
int stream_has_event(const StreamParser* sp);
int stream_pop_event(StreamParser* sp, StreamEvent* evt);

int stream_inject_sensor(StreamParser* sp, int pin, int value);
int stream_inject_event(StreamParser* sp, const char* event_type, uint32_t value);
int stream_inject_hardware(StreamParser* sp, const char* device, const char* action);

#endif
