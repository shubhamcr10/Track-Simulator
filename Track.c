#include "Track.h"

#include <ucdr/microcdr.h>
#include <string.h>

bool TrackType_serialize_topic(ucdrBuffer* writer, const TrackType* topic)
{
    bool success = true;

        success &= ucdr_serialize_array_char(writer, topic->shipID, sizeof(topic->shipID) / sizeof(char));

        success &= ucdr_serialize_float(writer, topic->range);

        success &= ucdr_serialize_float(writer, topic->bearing);

        success &= ucdr_serialize_float(writer, topic->sensor_ID);

        success &= ucdr_serialize_float(writer, topic->speed);

        success &= ucdr_serialize_double(writer, topic->sensor_timestamp);

    return success && !writer->error;
}

bool TrackType_deserialize_topic(ucdrBuffer* reader, TrackType* topic)
{
    bool success = true;

        success &= ucdr_deserialize_array_char(reader, topic->shipID, sizeof(topic->shipID) / sizeof(char));

        success &= ucdr_deserialize_float(reader, &topic->range);

        success &= ucdr_deserialize_float(reader, &topic->bearing);

        success &= ucdr_deserialize_float(reader, &topic->sensor_ID);

        success &= ucdr_deserialize_float(reader, &topic->speed);

        success &= ucdr_deserialize_double(reader, &topic->sensor_timestamp);

    return success && !reader->error;
}

uint32_t TrackType_size_of_topic(const TrackType* topic, uint32_t size)
{
    uint32_t previousSize = size;
        size += ucdr_alignment(size, 1) + sizeof(topic->shipID);

        size += ucdr_alignment(size, 4) + 4;

        size += ucdr_alignment(size, 4) + 4;

        size += ucdr_alignment(size, 4) + 4;

        size += ucdr_alignment(size, 4) + 4;

        size += ucdr_alignment(size, 8) + 8;

    return size - previousSize;
}
