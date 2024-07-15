## Track Simulator
The Track Simulator is a program designed to simulate the behavior of tracks in a naval-like combat display system. It sends track data to the Combat Display System application over Micro XRCE-DDS.

## Usage
# Running the Track Simulator
To run the Track Simulator, navigate to its build directory and execute the following command:

```bash
./TrackPublisher <IP_address> <port> <shipNumbers>
```
Replace <IP_address> and <port> with the IP address and port number where the Micro XRCE Agent is running. <shipNumbers> denotes the number of ships/tracks to simulate.

# Simulator Functionality
The Track Simulator provides the following functionality:

1. Update Track: Allows updating the properties of an existing track.
2. Add Track: Adds a new track (currently not fully implemented).
3. Remove Track: Removes a track (currently not implemented).

# Example Interaction
Upon running the simulator, you can interact with it via a console menu:

```bash
-----------------------------------------------------
1. Update Track
2. Add Track
3. Remove Track
Enter number: 2
Adding track functionality not working...
```

# Track Update
When updating a track, you'll be prompted to enter the track ID and update its properties such as range, bearing, speed, and movement flag:

```bash
-----------------------------------------------------
1. Update Track
2. Add Track
3. Remove Track
Enter number: 1
Enter track Id (ex 1,2,3 etc): 1
Enter Range: 2
Range Updated for track: T1
Enter Bearing: 3
Bearing Updated for track: T1
Enter speed: 4
Speed Updated for track: T1
Enter 1 for move away from OwnShip and 0 for move track near to OwnShip: 1
Flag Updated for track: T1
```
## Notes
- Ensure that the Micro XRCE Agent is running before starting the Track Simulator.
- After building the Micro XRCE Agent, you can start it with the following command:

```bash
./MicroXRCEAgent udp4 -p 8888
```

This command assumes the agent is built and installed according to the instructions provided by eProsima.

## Contact
For any questions or issues, please contact at shubhamcr10@gmail.com .