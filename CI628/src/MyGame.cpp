#include "MyGame.h"
#include "GameObjects.h"



GameData game_data;  // define game_data instnace
ProjectileData projectileData; // define projectileData instance

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {

    // PLAYER DISCONNECTED COMMAND
    if (cmd == "PLAYER_DISCONNECTED")
    {
        if (args.size() == 1) {
            int disconnectedID = std::stoi(args.at(0)); // store id
            std::cout << "Player of ID: " << disconnectedID << " has disconnected. Cleaning up game world...\n"; // DEBUG MESSAGE
            disconnectPlayer(disconnectedID); // disconnected the disconncected ID player
            game_data.readyToCreateRemotePlayer = false; // set this flag to false so disconnccted remote player will no longer be visible in game world

        }
    }


    // GAME DATA COMMAND
    if (cmd == "GAME_DATA") {
        // should have 4 arguements
        if (args.size() == 4) {
            game_data.receivedPositionsFromServer = true; // set to true

            // store the values in variables
            int id = stoi(args.at(0));
            int x = stoi(args.at(1));
            int y = stoi(args.at(2));
            int playerHealth = stoi(args.at(3));


            // set the server positions of the correct IDs 
            game_data.serverXPositions[id] = x;
            game_data.serverYPositions[id] = y;

            // set player healths correctly
            if (id == 1)
            {
                game_data.player1Health = playerHealth;
            }

            if (id == 2)
            {
                game_data.player2Health = playerHealth;
            }




            /* this is wrong: if I hardcode ID values then any player with ID 1 will be treated as local (and any player with ID 2 will be treated as remote player).
            * This completely breaks the multi-client functionality since the player 2, on the second client, whilst being the local client uses the spritesheet of player1
            * and even MOVES as player 1 while its own sprite does not move.
            *
            if (stoi(args.at(0)) == 1)
            {
                game_data.playerX = stoi(args.at(1));
                game_data.playerY = stoi(args.at(2));
            }

            else if (stoi(args.at(0)) == 2)
            {
                game_data.player2X = stoi(args.at(1));
                game_data.player2Y = stoi(args.at(2));
            }
            */


            //     game_data.ballX = stoi(args.at(2));
           //      game_data.ballY = stoi(args.at(3));
        }
    }

    // if (cmd == "MOVEMENT_DATA")

     // SPAWN DATA COMMAND
    if (cmd == "CLIENT_SPAWN_DATA")
    {
        if (args.size() == 3)
        {
            int id = stoi(args.at(0));
            int x = stoi(args.at(1));
            int y = stoi(args.at(2));

            // if client id isnt assigned then set it along with start positions
            if (!game_data.hasClientID)
            {
                game_data.clientID = id;
                game_data.hasClientID = true;
                game_data.startX = x; // mixing logic of spawn data such as startX and startY is a bit confusing and needs to be cleaned up later.
                game_data.startY = y;
                return;
            }

        }
    }
    // LEVEL DATA COMMAND
    if (cmd == "LEVEL_DATA")
    {
        std::cout << "Client received level data from server.\n"; // DEBUG MESSAGE

        // "DESERIALIZE" LEVEL DATA
        game_data.mapWidth = std::stoi(args.at(0));  // store map width
        game_data.mapHeight = std::stoi(args.at(1)); // store map height
        std::string levelData = args.at(2);          // create a new string variable for holding the map data



        int rowStart = 0; // row starts at 0
        int rowEnd = levelData.find(';');  // row ends at the ; delimiter
        int row = 0; // set row to 0

        // while loop going through each of the rows
        while (rowEnd != -1 && row < game_data.mapHeight) {
            std::string rowData = levelData.substr(rowStart, rowEnd - rowStart);

            int colStart = 0; // column starts at 0
            int colEnd = rowData.find(':'); // column ends at the : delimiter
            int col = 0; // set col to 0

            // while loop going through each of the columns
            while (colEnd != -1 && col < game_data.mapWidth) {
                game_data.levelMap[row][col++] = std::stoi(rowData.substr(colStart, colEnd - colStart));
                colStart = colEnd + 1;
                colEnd = rowData.find(':', colStart); // in each column find where the next column is by looking for the : column delimiter using find()
            }

            // handling the last column of a row
            if (col < game_data.mapWidth) {
                game_data.levelMap[row][col] = std::stoi(rowData.substr(colStart));
            }

            // after handling the last column of a row,, move on to the next row and find the ; row delimiter
            rowStart = rowEnd + 1;
            rowEnd = levelData.find(';', rowStart);
            row++;
        }

        // fthe last row and column dooes not get parsed as data so an additional if statement checks for the last row and all columns in it
        if (row < game_data.mapHeight && rowStart < levelData.size()) {
            std::string rowData = levelData.substr(rowStart);
            int colStart = 0;
            int colEnd = rowData.find(':');
            int col = 0;
            while (colEnd != -1 && col < game_data.mapWidth) {
                game_data.levelMap[row][col++] = std::stoi(rowData.substr(colStart, colEnd - colStart));
                colStart = colEnd + 1;
                colEnd = rowData.find(':', colStart);
            }
            if (col < game_data.mapWidth) { // last column of the row
                game_data.levelMap[row][col] = std::stoi(rowData.substr(colStart));
            }
        }

        std::cout << "Map size: " << game_data.mapWidth << " x " << game_data.mapHeight << "\n"; // DEBUG TO MAKE SURE THE CORRECT MAP SIZE IS CREATED

        game_data.mapDataReceived = true; // set this flag to true to ensure map data is received and then in main.cpp the map can be rendered
    }


    // LEVEL UPDATE COMPLETE COMMAND
    if (cmd == "LEVEL_UPDATE_COMPLETE")
    {
        game_data.levelLoaded = true; // just sets this flag to true as a double check ensuring the level is ready to render
    }


    // REMOTE SPAWN DATA COMMAND
    if (cmd == "REMOTE_SPAWN_DATA")
    {
        if (args.size() == 3)
        {
            // stoer values
            int id = stoi(args.at(0));
            int x = stoi(args.at(1));
            int y = stoi(args.at(2));


            // if id is client id then ignore since we already have the client player created; this method should onjly handle remote player creation
            if (id == game_data.clientID) { // this is just a check
                std::cout << "Already have a client ID. Create a remote client instead\n";
                return;
            }

            else
            {
                // remove the id from disconnected players set if its in that set and add it to remoteSpawnList instead
                game_data.disconnectedPlayers.erase(id);
                game_data.remoteSpawnList.push_back({ id, x, y });

                game_data.remotePlayerID = id; // set the remotePlayerID as well 
                //game_data.remoteStartX = x; not needed
                //game_data.remoteStartY = y;
                std::cout << "Remote ID created for remote player: " << game_data.remotePlayerID << "\n"; // DEBUG
                game_data.readyToCreateRemotePlayer = true; // this flag ensures remote player is also ready to render

            }


        }
    }

    // PROJECTILE SPAWN COMMAND
    if (cmd == "PROJECTILE_SPAWN")
    {
        // this command should be received from the server as validation that the projectile should be created and that the right projectile with the correct id is created


        // this command mainly just verifies that everything is correct: clientX,Y, the targetX,Y, and also makes sure the projectile is properly intialised 
        // set all appropriate values
        int projectileId = stoi(args.at(0));
        projectileData.id = projectileId;

        int clientX = stoi(args.at(1));
        int clientY = stoi(args.at(2));

        int targetX = stoi(args.at(3));
        int targetY = stoi(args.at(4));


        projectileData.xPos = clientX;
        projectileData.yPos = clientY;

        projectileData.targetX = targetX;
        projectileData.targetY = targetY;


        // it also makes sure that once everything is validated the projectile is ready to fire and ready to render
        // projectiles will only spawn and render after these flags
        projectileData.readyToFire = true;
        game_data.readyToRenderProjectile = true;

    }

    // PROJECTILE UPDATE COMMAND
    if (cmd == "PROJECTILE_UPDATE")
    {

        // in this statement the actual server X and Y positions of hte projectile are checked against the predicted positions of the projectile which were simulated locally
        // std::cout << "update for projectile has been received\n";

        // get the projectile id and its updated x and y positions

        // store projectile ID
        int projectileId = stoi(args.at(0));
        int projectileServerX = stoi(args.at(1));
        int projectileServerY = stoi(args.at(2));

        // find the correct projectile with the corresponding id
        auto projectile = game_data.worldProjectiles.find(projectileId);
        if (projectile != game_data.worldProjectiles.end()) { // if its found then
            Projectile* predictedProjectile = projectile->second; // retrieve the projectile object

            // use lerp on the instance of the predictedProjectile
            float reconciledXPos = predictedProjectile->lerp(predictedProjectile->getX(), projectileServerX, 0.1f);
            predictedProjectile->setX(reconciledXPos);  // set the new X position

            float reconciledYPos = predictedProjectile->lerp(predictedProjectile->getY(), projectileServerY, 0.1f);
            predictedProjectile->setY(reconciledYPos);  // set the new X position
        }

        // projectile X and Y need to be compared against the predicted (locally simulated X and Y for the projectile).
    }


    // PROJECTILE DEATH COMMAND
    if (cmd == "PROJECTILE_DEATH")
    {
        // find the inactive projectile and remove it from the unordered map by ID since it needs not be rendered annymore.
        // client needs to know when to stop rendering a particular projectile therefore this message is requires else client just sees a "trail" of the projectile they fired

        // the id of the projectile to remove from the world
        int projectileIdToRemove = stoi(args.at(0));

        // erase the projectile from the map so it appears as if it "despawns" on hit.
        game_data.worldProjectiles.erase(projectileIdToRemove);
    }
}

// sending messages to server
void MyGame::send(std::string message) {
    messages.push_back(message);
}

// input method: used for sending messages about player inputs
void MyGame::input(SDL_Event& event) {

    // receive the event from the main game loop and check which type of event it was, and then send a message accordingly about player inputs
    // client one inputs should be W,A,S,D for movement and F key for firing projectile
    if (game_data.clientID == 1)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            send(event.type == SDL_KEYDOWN ? "W_DOWN" : "W_UP");
            break;

        case SDLK_a:
            send(event.type == SDL_KEYDOWN ? "A_DOWN" : "A_UP");
            break;

        case SDLK_s:
            send(event.type == SDL_KEYDOWN ? "S_DOWN" : "S_UP");
            break;

        case SDLK_d:
            send(event.type == SDL_KEYDOWN ? "D_DOWN" : "D_UP");
            break;

        case SDLK_f: // F IS THE FIRE BUTTON FOR PLAYER 1
            // FOR THE PROJECTILE FIRING WE DONT NEED TO SEND F_DOWN AND F_UP BOTH, JUST NEED TO SEND THAT F FIRE KEY WAS PRESSED
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                static int nextProjectileId = 0; // STATIC INT TO MAKE SURE THE ID VALUE PERSISTS
                int projectileId = nextProjectileId++; // update id everytime the fire key is pressed
                projectileData.id = projectileId;

                // bullets were being fired from incorrect positions because the wrong X and Y positions were being accessed; this is not how you acccess the positions in unordered maps
                //int clientX = game_data.serverXPositions[game_data.clientID];
               //int clientY = game_data.serverYPositions[game_data.clientID];

                // this is the correct way to access: the correct position must be FOUND using .find and the correct id
                // also these onyl find the correct pair
                auto clientX = game_data.serverXPositions.find(game_data.clientID);
                auto clientY = game_data.serverYPositions.find(game_data.clientID);

                // now the values must be retrieved from them
                // 
                double clientXPos = clientX->second; // second means the actual X value in server positions (first is the client Id value)
                double clientYPos = clientY->second;

                // set projecitleData x and y positions; this will be starting positions for projectile
                projectileData.xPos = clientXPos;
                projectileData.yPos = clientYPos;


                std::cout << "Player is at X: " << clientXPos << " and Y: " << clientYPos << std::endl; // DEBUG

                // LOG THE MOUSE X AND Y POSITIONS USING SDL MOUSE STATE; THIS WILL BE THE TARGET POSITIONS
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                projectileData.targetX = mouseX;
                projectileData.targetY = mouseY;


                // SEND THE FIRE MESSAGE
                std::string msg = "FIRE";
                // without converting the "FIRE" text to string it kept citing a "expression must have integral or unscoped enum type" since it wouldnt recognise it as string concatenation 
                send(msg + "," + std::to_string(mouseX) + "," + std::to_string(mouseY) + "," + std::to_string(projectileId));

            }
            break;
        }



    }

    // client two inputs should be I, K, L, J for movement and G key for firing projectile
    else if (game_data.clientID == 2)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_i:
            send(event.type == SDL_KEYDOWN ? "I_DOWN" : "I_UP");
            break;
        case SDLK_k:
            send(event.type == SDL_KEYDOWN ? "K_DOWN" : "K_UP");
            break;
        case SDLK_l:
            send(event.type == SDL_KEYDOWN ? "L_DOWN" : "L_UP");
            break;
        case SDLK_j:
            send(event.type == SDL_KEYDOWN ? "J_DOWN" : "J_UP");
            break;


        case SDLK_g: // G IS THE FIRE BUTTON
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                    if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                        // SAME CODE AS THE ONE USED FOR PLAYER 1'S F KEY PRESS. IT MIGHT BE BETTER TO ALLOW BOTH CLIENTS TO USE THE SAME FIRE BUTTON (F)
                        static int nextProjectileId = 0;
                        int projectileId = nextProjectileId++;
                        projectileData.id = projectileId;

                        auto clientX = game_data.serverXPositions.find(game_data.clientID);
                        auto clientY = game_data.serverYPositions.find(game_data.clientID);


                        double clientXPos = clientX->second;
                        double clientYPos = clientY->second;


                        projectileData.xPos = clientXPos;
                        projectileData.yPos = clientYPos;


                        std::cout << "Player is at X: " << clientXPos << " and Y: " << clientYPos << std::endl; // DEBUG

                        // LOG THE MOUSE X AND Y POSITIONS USING SDL MOUSE STATE; THIS WILL BE THE TARGET POSITIONS
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);

                        projectileData.targetX = mouseX;
                        projectileData.targetY = mouseY;


                        // SEND THE FIRE MESSAGE
                        std::string msg = "FIRE";
                        send(msg + "," + std::to_string(mouseX) + "," + std::to_string(mouseY) + "," + std::to_string(projectileId));
                    }

                }


            }
            break;


        }
    }
}

void MyGame::update() {

    // player1.y = game_data.player1Y;
     // player1.x = game_data.player1X;
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
}

// DISCONNECT PLAYER METHOD
void MyGame::disconnectPlayer(int disconnectedPlayerID)
{
    auto disconnectedPlayer = game_data.players.find(disconnectedPlayerID);
    if (disconnectedPlayer != game_data.players.end()) {
        delete disconnectedPlayer->second; // this is important to do as memory needs to be freed up
        game_data.players.erase(disconnectedPlayer); // erase disconnected player from players map
        std::cout << "Remote player " << disconnectedPlayerID << " removed.\n"; // cleanly remove the remote player
    }
    game_data.disconnectedPlayers.insert(disconnectedPlayerID); // also mark the disconnected player as disconnected by keep it in this set
}