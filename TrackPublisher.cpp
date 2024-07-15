#include "Track.h"
#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>

#include <cstdlib>
#include <ctime>
#include <random>
#include <unistd.h>

#define NUM_ITERATIONS  1000
#define STREAM_HISTORY  8
#define SHIP_ID_SIZE    50

#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU * STREAM_HISTORY

const std::string participant_xml = "<dds>"
                                      "<participant>"
                                          "<rtps>"
                                              "<name>default_xrce_participant</name>"
                                          "</rtps>"
                                      "</participant>"
                                  "</dds>";
const std::string topic_xml = "<dds>"
                                "<topic>"
                                    "<name>TRACKS_TOPIC</name>"
                                    "<dataType>TrackType</dataType>"
                                "</topic>"
                            "</dds>";
const std::string publisher_xml = "";
const std::string datawriter_xml = "<dds>"
                                     "<data_writer>"
                                         "<topic>"
                                             "<kind>NO_KEY</kind>"
                                             "<name>TRACKS_TOPIC</name>"
                                             "<dataType>TrackType</dataType>"
                                         "</topic>"
                                     "</data_writer>"
                                 "</dds>";


uxrUDPTransport transport;
uxrSession session;
uint32_t max_topics;
uxrStreamId reliable_out;
uxrObjectId datawriter_id;
// std::vector<TrackTypeModified>m_trackVec;
std::vector<TrackTypeModified>m_trackVec;
std::vector<std::thread> threads;
std::mutex mtx;
std::mt19937 rng(std::time(0)); // Seed the random number generator
std::uniform_int_distribution<uint32_t> range_dist(30, 1920); // Range between 30 and 1920
std::uniform_int_distribution<uint32_t> bearing_dist(0, 360);
std::uniform_int_distribution<uint32_t> sensor_ID_dist(1, 3);
std::uniform_int_distribution<uint32_t> speed_dist(1, 22);
std::uniform_int_distribution<uint16_t> flag_dist(0, 1);

bool initializeSession(const std::string& ip, const std::string& port) {
    if (!uxr_init_udp_transport(&transport, UXR_IPv4, ip.c_str(), port.c_str())) {
        std::cerr<<"Error: transport initialization error"<<std::endl;
        return false;
    }

    uxr_init_session(&session, &transport.comm, 0xAAAA0000);
    // uxr_set_topic_callback(&session, onTrackTopic, this);
    // uxr_set_topic_callback(&session, onTopic, this);

    if (!uxr_create_session(&session)) {
        std::cerr<<"Error: session creation error"<<std::endl;
        return false;
    }

    // Streams
    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    // Create entities
    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0, participant_xml.c_str(), UXR_REPLACE);

    uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
    uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml.c_str(), UXR_REPLACE);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id, publisher_xml.c_str(), UXR_REPLACE);

    datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id, datawriter_xml.c_str(), UXR_REPLACE);

    // Send create entities message and wait its status
    uint8_t status[4];
    uint16_t requests[4] = {participant_req, topic_req, publisher_req, datawriter_req};
    if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 4)) {
        std::cerr << "Error at create entities " << " topic: " << int(status[1]) << " publisher: " << int(status[2]) << " darawriter: " << int(status[3]) << std::endl;
        return false;
    }

    return true;
}

void send_track_data(const uint32_t &shipNumber) {
    std::string shipID = "T" + std::to_string(shipNumber+1);

    // Continuously send track information for each ship
    for (int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
                // Get current timestamp in milliseconds
        mtx.lock();
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        m_trackVec[shipNumber].sensor_timestamp = (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
        // while(true){
            
        
        TrackType topic = {{0}, m_trackVec[shipNumber].range, m_trackVec[shipNumber].bearing, m_trackVec[shipNumber].sensor_ID, m_trackVec[shipNumber].speed, m_trackVec[shipNumber].sensor_timestamp};

        strcpy(topic.shipID, shipID.c_str());
        // printf("Iterations ship_%s_%d: \n", shipID, iteration);
        // printf(" Publisher Current Timestamp: %.3f\n", m_trackVec[shipNumber].sensor_timestamp);
        // topic.sensor_timestamp = m_trackVec[shipNumber].sensor_timestamp;
        // printf("ShipID: %s, Range: %f, Bearing: %f, Sensor ID: %f, Speed: %f, Timestamp: %.3f\n", topic.shipID, topic.range, topic.bearing, topic.sensor_ID, topic.speed, topic.sensor_timestamp);
        
        ucdrBuffer mb;
        uint32_t topic_size = TrackType_size_of_topic(&topic, 0);
        uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &mb, topic_size);
        TrackType_serialize_topic(&mb, &topic);

        // Update range based on flag (+/-) and speed
        if (m_trackVec[shipNumber].flag == 1) {
            m_trackVec[shipNumber].range += m_trackVec[shipNumber].speed;
        } else {
            m_trackVec[shipNumber].range -= m_trackVec[shipNumber].speed;
            // Check if range is close to 0
            if (m_trackVec[shipNumber].range <= 30 && m_trackVec[shipNumber].range >= 0) {
                m_trackVec[shipNumber].flag = 1; // Set flag to 1 to start increasing range
                // printf("Range is getting close to 0. Switching flag to 1.\n");
            }
        }
        m_trackVec[shipNumber].bearing++; // Increment bearing
        //send and wait one second
        uxr_run_session_time(&session, 50);
        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    // Delete resources
    // uxr_delete_session(&session);
}

void menu()
{
    std::cout<<"\n-----------------------------------------------------"<<std::endl;
    std::cout<<"1. Update Track"<<std::endl;
    std::cout<<"2. Add Track"<<std::endl;
    std::cout<<"3. Remove Track"<<std::endl;
    std::cout<<"Enter number: ";
}


void updateTrack(int trackId){
    if(trackId > m_trackVec.size()){
        std::cout<<"Track Not found"<<std::endl;
        return;
    }

    std::cout<<"Enter Range: ";
    std::cin>>m_trackVec[trackId-1].range;
    std::cout<<"Range Updated for track: "<<m_trackVec[trackId-1].shipID<<std::endl;
    std::cout<<"Enter Bearing: ";
    std::cin>>m_trackVec[trackId-1].bearing;
    std::cout<<"Bearing Updated for track: "<<m_trackVec[trackId-1].shipID<<std::endl;
    std::cout<<"Enter speed: ";
    std::cin>>m_trackVec[trackId-1].speed;
    std::cout<<"Speed Updated for track: "<<m_trackVec[trackId-1].shipID<<std::endl;
    std::cout<<"Enter 1 for move away from OwnShip and 0 for move track near to OwnShip: ";
    std::cin>>m_trackVec[trackId-1].flag;
    std::cout<<"Flag Updated for track: "<<m_trackVec[trackId-1].shipID<<std::endl;

}

int addTrack()
{
    // Generate random values for inputs
    TrackTypeModified trackTypeObj;
    
    std::string ship_ID = "T" + std::to_string(m_trackVec.size());
    strcpy(trackTypeObj.shipID, ship_ID.c_str());

    trackTypeObj.range = range_dist(rng);
    trackTypeObj.bearing = bearing_dist(rng);
    trackTypeObj.sensor_ID = sensor_ID_dist(rng);
    trackTypeObj.speed = speed_dist(rng);

    // Adjust flag based on range
    trackTypeObj.flag = (trackTypeObj.range <= 100) ? 1 : flag_dist(rng);

    m_trackVec.push_back(trackTypeObj);
    std::cout<<"Added track: "<<ship_ID<<std::endl;

    return (m_trackVec.size()-1);
}

void trackUpdate(){
    
    int option, trackId;
    while (true)
    {
        menu();
        std::cin>>option;
        switch (option)
        {
        case 1:
            std::cout<<"Enter track Id (ex 1,2,3 etc): ";
            std::cin>>trackId;
            updateTrack(trackId);
            break;
        case 2:
            // std::cout<<"Started adding track..."<<std::endl;
            std::cout<<"Adding track functionality not working..."<<std::endl;
            // int lastTrack = addTrack();
            // threads.emplace_back(send_track_data, addTrack());
            // threads[threads.size()-1].join();
            // std::cout<<"Started running thread"<<std::endl;
            break;
        default:
            std::cout<<"Invalid Input: "<<option<<std::endl;
            // break;
        }

        // Sleep for 1000 milliseconds (1 second)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
}




int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "usage: " << argv[0] << " ip port shipNumbers" << std::endl;
        return 0;
    }
    
    std::string ip = argv[1];
    std::string port = argv[2];
    uint32_t shipNumbers = std::atoi(argv[3]);
    initializeSession(ip,port);

    m_trackVec.reserve(shipNumbers);

    for (int i = 1; i <= shipNumbers; ++i) {
        // Generate random values for inputs
        TrackTypeModified trackTypeObj;
        
        std::string ship_ID = "T" + std::to_string(i);
        strcpy(trackTypeObj.shipID, ship_ID.c_str());

        trackTypeObj.range = range_dist(rng);
        trackTypeObj.bearing = bearing_dist(rng);
        trackTypeObj.sensor_ID = sensor_ID_dist(rng);
        trackTypeObj.speed = speed_dist(rng);

        // Adjust flag based on range
        trackTypeObj.flag = (trackTypeObj.range <= 100) ? 1 : flag_dist(rng);

        m_trackVec.push_back(trackTypeObj);
    }

    // Create threads to send track information for each ship

    std::thread updateThread;
    for (uint32_t i = 0; i < shipNumbers; ++i) {
        threads.emplace_back(send_track_data, i);
    }
    updateThread = std::thread(trackUpdate);
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    updateThread.join();
    
    // Delete resources
    uxr_delete_session(&session);
    return 0;
}
