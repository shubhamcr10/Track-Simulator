#ifndef _Track_H_
#define _Track_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/*!
 * @brief This struct represents the structure TrackTopic defined by the user in the IDL file.
 * @ingroup Track
 */
typedef struct TrackType
{
    char shipID[50];

    float range;
    float bearing;
    float sensor_ID;
    float speed;
    double sensor_timestamp;
} TrackType;


typedef struct TrackTypeModified
{
    char shipID[50];
    float range;
    float bearing;
    float sensor_ID;
    float speed;
    double sensor_timestamp;
    bool flag;
} TrackTypeModified;

struct ucdrBuffer;

bool TrackType_serialize_topic(struct ucdrBuffer* writer, const TrackType* topic);
bool TrackType_deserialize_topic(struct ucdrBuffer* reader, TrackType* topic);
uint32_t TrackType_size_of_topic(const TrackType* topic, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif // _Track_H_