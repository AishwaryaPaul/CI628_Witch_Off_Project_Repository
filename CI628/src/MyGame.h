#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "SDL.h"
#include "GameObjects.h"

// structs are useful to hold multiple variables and then can be used to pass as a parameter to lists, for example. This is useful since lists cant take more than 2 parameters.
// spawn info just contains the id of the player and the x and y positions theyre supposed to spawn at
struct RemoteSpawnInfo {
    // this struct is mostly useful for keeping info for remote players which spawn along with their positions
    int id;
    int x, y;
};

// contains data for projectiles in the world
struct ProjectileData {
    int id = 0; // projectile id

    // x and y positions
    // ALL POSITION VARIABLES ACROSS CLIENT AND SERVER SHOULD BE USING THE SAME VARIABLE TYPE IDEALLY; THIS NEEDS TO BE CHECKED AND FIXED IN THE FUTURE
    double xPos = 0;
    double yPos = 0;

    // target x and y positions
    double targetX = 0;
    double targetY = 0;
    bool readyToFire = false;
};
extern ProjectileData projectileData;


struct GameData {
    int clientID = -1; // the id of hte client
    int remotePlayerID = -1; // the id of the remote player
    // holding them in separate variables is sufficient since I decided that ultimately my game will only have 2 players. though this solution is not very scalable in the long term.

    //int player1Y = 0;
   // int player1X = 0;
    //int player2Y = 0;
    //int player2X = 0;

    //int playerX = 0;
    //int playerY = 0;
    //int ballX = 0;
    //int ballY = 0;
     //int player1X = 0;
    //int player1Y = 0;
    //int player2X = 0;
    //int player2Y = 0;


    bool hasClientID = false; // used to check if the client id has been received from server
    bool isSpectator = false;
    bool receivedPositionsFromServer = false; // used to check if positions have been recieved by server

    bool localClientCreated = false; // ysed to check if the local client has been created
    bool readyToCreateRemotePlayer = false; // used to determine if ready to create remote players (when remote spawn data message is received from server)

    // HOLDS THE PLAYER HEALTH VARIABLES: THESE VALUES WILL BE RECEIVED FROM THE SERVER TO ENSURE THEY ARE VALID. THE PLAYER CHARACTERS IN THE CLIENT CODE WILL NOT HAVE A HEALTH VARIABLE, ONLY THE PLAYERCHARACTER COMP IN THE SERVER WILL
    int player1Health = 100;
    int player2Health = 100;


    bool readyToRenderProjectile = false; // determines if projectiles are ready to render (this is after server confirms projectile can be spawned and initialised)

    // WORLD PROJECTILES SHOULD BE MOVED INTO PROJECTILE DATA IDEALLY
    std::unordered_map<int, Projectile*> worldProjectiles; // world projectiles will be stored in an unordered map with an integer ID and the Projectile object itself.
    // this is so that i can track what id of projectile is spawned or destroyed and then render or remove from the clients accordingly




    std::unordered_map<int, PlayerCharacter*> players; // using unordered map instead of making player1 and player2 variables.
    std::unordered_set<int> disconnectedPlayers; // holds the disconnected players and enables disconnectedplayers to rejoin the game properly later. Stored in a set since they each disconnected ID must be unique. Putting them in the set ensures they will be unique.
    // THIS SET CAN BE CONVERTED INTO AN UNORDERED MAP LATER SO THE HEALTH OF DISCONNECTED PLAYERS COULD ALSO BE STORED SINCE CURRENTLY
    //IF A PLAYER DISCONNECTS AND REJOINS THEIR HEALTH RETURNS TO 100, WHILE THE OTHER PLAYER'S HEALTH WON'T CHANGE TO WHAT IT WAS BEFORE THE OTHER PLAYER DISCONNECTED.
    // THIS COULD EITHER BE SOLVED BY THIS CHANGE OR BY IMPLEMENTING A LOBBY SYSTEM FOR THE GAME

    // hold the X and Y positions of each client ID in these unordered maps. This makes it easy to access any plauer's X and Y just by using their Id
    std::unordered_map<int, int> serverXPositions;
    std::unordered_map<int, int> serverYPositions;


    std::list<RemoteSpawnInfo> remoteSpawnList; // using a list to keep the list of remote spawn players because without this the remote spawns were actually not rendering for local players for some reaosn




    // ideally these two variables should not be used here in game data but should be moved into spawn info and RemoteSpawnInfo should be renamed to just SpawnInfo. using these teo variables may be inefficient
    int startX = 0;
    int startY = 0;
    // REMOTE SPAWN INFO SHOULD BE USED IN PLACE OF THESE TWO; INTEGRATE THIS CHANGE LATER
    int remoteStartX = 0;
    int remoteStartY = 0;

    // level map width and height which will be set later
    int mapWidth = 0;
    int mapHeight = 0;
    int levelMap[50][50]; // level map as an array. should be max 50x50 tiles though map is still likely to be smaller than this. Not ssetting this to map width and map height size since they could need to be static constants.
    bool mapDataReceived = false; // check for ensuring map data is received
    bool levelLoaded = false; // check for ensuring level data is loaded

    bool mapReadyForRendering = false; // this variable is used as a flag but this might be unncessary since 2 other flags were added

};

extern GameData game_data; // declare external game_data 

class MyGame {

private:
    SDL_Rect player1 = { 0, 0, 20, 60 };


public:
    std::vector<std::string> messages;
    void on_receive(std::string message, std::vector<std::string>& args);
    void send(std::string message);
    void input(SDL_Event& event);
    void update();
    void render(SDL_Renderer* renderer);
    void disconnectPlayer(int disconnectedPlayerID); // CLEANLY DISCONNECT THE PLAYER AND HOLD IT IN THE DISCONNCTED PLAYERS SET AND ASSIGN IT AGAIN UPON RECONNECTION
};



#endif