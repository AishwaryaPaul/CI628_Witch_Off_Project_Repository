
#include <SDL_net.h> // include the SDL_net Header file 
#include "MyGame.h" // include the MyGame Header file
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "GameObjects.h"
#include <unordered_map>

using namespace std;
SDL_Renderer* gameRenderer = nullptr;
SDL_Window* window = nullptr;

TTF_Font* gameFont = nullptr;

const char* IP_NAME = "localhost"; // create a constant char variable IP_Name
const Uint16 PORT = 55555;

bool is_running = true; // set a new boolean is_running variable to true

MyGame* game = new MyGame(); // create a new instance of the game 

Uint32 lastTick = SDL_GetTicks();

bool isGameOver = false;
bool musicStarted = false;

static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr; // create TCP socket

    const int message_length = 1024;

    char message[message_length];
    int received;

    // TODO: while(), rather than do
    do {

        received = SDLNet_TCP_Recv(socket, message, message_length); // recieve messages over TCP socket 

        // check if received is less than or equal to 0; shutdown the program if it is
        if (received <= 0) {
            is_running = false;
            break;
        };
        message[received] = '\0';

        // pch = pointer to token (substring of message)
        char* pch = strtok(message, ","); // split message into tokens and separates them with a comma.

        // get the command, which is the first string (token) in the message. The other tokens are the arguments of the command
        string cmd(pch);


        vector<string> args; // these are the arguments to the command, stored as a vector of strings

        // --- Parsing the arguements of the token

        while (pch != NULL) {
            pch = strtok(NULL, ","); // get next token/ substring of message
            if (pch != NULL) {
                args.push_back(string(pch)); // save the arguments in the args vector 
            }
        }
        // ---

        // process the command and arguments
        game->on_receive(cmd, args);


        // if cmd is Exit then shut connection.
        if (cmd == "exit") {
            break;
        }


    } while (received > 0 && is_running);


    return 0;
}

static int on_send(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    while (is_running) {
        if (game->messages.size() > 0) {
            string message = "CLIENT_DATA";

            for (auto m : game->messages) {
                message += "," + m;
            }

            game->messages.clear();

            cout << "Sending_TCP: " << message << endl;

            SDLNet_TCP_Send(socket, message.c_str(), message.length());
        }

        SDL_Delay(1);
    }

    return 0;
}


// LOOP()
/*
void loop() {
    SDL_Event event;

    while (is_running) {
        // input
        while (SDL_PollEvent(&event)) {
            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                game->input(event);

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;

                    default:
                        break;
                }
            }

            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        SDL_SetRenderDrawColor(globalRenderer, 0, 0, 0, 255);
        SDL_RenderClear(globalRenderer);

        game->update();

        game->render(globalRenderer);

        SDL_RenderPresent(globalRenderer);

        SDL_Delay(17);
    }
}
*/



// ----- RUNNING THE GAME -----

// INT RUN_GAME()
/*
int run_game() {
    // ---
    // CREATE THE SDL_WINDOW
    SDL_Window* window = SDL_CreateWindow(
        "Multiplayer Pong Client", // Name the Window
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, // Set the position of the Window
        800, 600, // Set the resolution of the Window
        SDL_WINDOW_SHOWN // Set whether the window is visible or not
    );
    // If window could not be created, log a message to the console that window creation was failed and output which error occurred
    if (nullptr == window) {
        std::cout << "Failed to create window" << SDL_GetError() << std::endl;
        return -1;
    }
    // ---


    // ---
    // CREATE THE SDL RENDERER
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Create an instance of the rendererr with the earlier created window in the window parameter

    if (nullptr == renderer) {
        std::cout << "Failed to create renderer" << SDL_GetError() << std::endl;
        return -1;
    }

    loop(); // loop the renderer so that the game object rects are constantly being rendered/displayed

    return 0;
}
*/

// CREATE REMOTE PLAYER METHOD
void createRemotePlayer(int playerId, int x, int y) {

    if (playerId == game_data.clientID) return; // ignore if its theclient ID

    // if the rmeote playeris not already in players then create a new PlayerCharacter object with that player id
    if (game_data.players.find(playerId) == game_data.players.end()) {
        game_data.players[playerId] = new PlayerCharacter(playerId, gameRenderer, x, y); // remoet players created at x and y positions passed from parameters unlike for local client creation. this is somewhat messy and should be cleaned up, they should both be having similar spawning methods to cause less confusion.
        std::cout << "Created REMOTE player with ID " << playerId << "\n"; // DEBUG
    }
}

// CREATE PROJECTILES METHOD
void creatingProjectiles()
{
    // if projectile is ready to be rendered then create an instance of it
    if (game_data.readyToRenderProjectile)
    {
        //game_data.projectiles.push_back(Projectile(gameRenderer, game_data.projectileSpawnX, game_data.projectileSpawnY)); this wont work

        // once projectile is ready to render, create a projectile object at the correct location

        Projectile* newProjectile = new Projectile(gameRenderer, projectileData.xPos, projectileData.yPos);
        std::cout << "New projectile created at: " << projectileData.xPos << "," << projectileData.yPos << endl;
        game_data.worldProjectiles[projectileData.id] = newProjectile;

        game_data.readyToRenderProjectile = false; // set the flag back to false else it'll keep rendering multiple times
    }
}

// CREATE LOCAL PLAYER CLIENT
void createLocalPlayer()
{
    // check whether a client ID has been assigned yet
    if (!game_data.hasClientID) {
        std::cout << "Cannot create local player: no clientID yet.\n";
        return;
    }

    int id = game_data.clientID; // set client id 

    // check if the player exists; local player only should be created once, other players should be considered as remote players
    if (game_data.players.count(game_data.clientID)) {
        std::cout << "Local player already exists.\n";
        return;
    }

    // create the player object and add it to players
    PlayerCharacter* player = new PlayerCharacter(game_data.clientID, gameRenderer, game_data.startX, game_data.startY);
    game_data.players[game_data.clientID] = player;

    std::cout << "Created LOCAL player with ID " << game_data.clientID << "\n"; // DEBUG

}

// RENDER PLAYERS
void renderPlayers(SDL_Renderer* renderer)
{
    // iterlate through players and retrieve the PlayerCharacter object of each player client and render them
    for (auto& client : game_data.players) {
        PlayerCharacter* player = client.second;
        if (!player) continue;
        player->render(renderer); // call the renderer method
    }
}

// HANDLE PROJECTILES
void handleProjectiles(SDL_Renderer* renderer, float dt)
{
    // iterate through the world projectiles, get each projectile and its Projectile Object, call its fire method and render it and also update it.
    for (auto& projectileObject : game_data.worldProjectiles) {
        Projectile* projectile = projectileObject.second;
        if (!projectile) continue;

        projectile->fireAtTarget(projectileData.xPos, projectileData.yPos, projectileData.targetX, projectileData.targetY);
        // it would be cleaner to separate these into further methods which contain the appropriate logic separately especially since this method name is rather ambiguous 
        projectile->render(renderer);
        projectile->update(dt);
    }
}


// UPDATE PLAYERS
void updatePlayers(float dt, const Uint8* keys)
{
    for (auto& client : game_data.players) { // for each player client in the players unordered map in the game data struct
        int id = client.first; // obtain the id of hte player client
        PlayerCharacter* player = client.second; // and obtain the player object of the client

        if (!player) continue;  // return if no player as a safety check

        // store the old x y positions for animation calculation
        int oldX = static_cast<int>(player->getX());
        int oldY = static_cast<int>(player->getY());

        // if position updates received from server then set the server X and Y positions  and also set the player's position to those authoriative positions
        if (game_data.receivedPositionsFromServer)
        {
            if (game_data.serverXPositions.count(id) && game_data.serverYPositions.count(id))
            {
                // this is actually incorrect for client-side prediction; movement needs to be applied by client and then corrected by
                // the server, rather than snapping onto server set positions.
                // this is also probably the source of the jittery movement since this is run every frame in the main game loop
                // for client side prediction for players; simulate movement locally first, and then check if these server authoritative positions and local positions match up. if not, correct the differences.
                player->setPos(game_data.serverXPositions[id], game_data.serverYPositions[id]);
            }
        }

        /* OLD CODE
        // -
        if (game_data.receivedPositionsFromServer) {
            if (game_data.clientID == id) {
                // local player server positions
                player->setPos(game_data.playerX, game_data.playerY);
            }
            else {
                // remote player server positions
                player->setPos(game_data.player2X, game_data.player2Y);
            }
        }
        */

        // update animations for the local player
        if (id == game_data.clientID) {

            player->handleAnimations(keys); // handle animation method for local player

        }
        else {

            player->handleRemoteAnimations(oldX, oldY, dt); // remote animations for remote player

        }


        // update animations method for both players to play the correct animation frames
        player->updateAnimations(dt);

    }
}



// DISPLAYING GAME UI
void displayGameUI()
{
    if (!gameRenderer)
    {
        SDL_Log("Renderer doesnt exist"); // DEBUG IF RENDERER DOESNT EXIST
        return;
    }

    if (!gameFont)
    {
        SDL_Log("Font doesnt exist"); // DEBUG IF FONT DOESNT EXIST
        return;
    }

    // retrieve health values to display as game info
    int health1 = game_data.player1Health;
    int health2 = game_data.player2Health;

    // hold the text
    std::string text1 = "Player 1 Health: " + std::to_string(health1);

    std::string text2 = "Player 2 Health: " + std::to_string(health2);

    // text color
    SDL_Color textColour = { 255, 255, 255, 255 };

    // surface needs to be created for both texts
    SDL_Surface* surface1 = TTF_RenderText_Solid(gameFont, text1.c_str(), textColour);
    // surface needs to be created
    SDL_Surface* surface2 = TTF_RenderText_Solid(gameFont, text2.c_str(), textColour);

    // log mesage if surfaces dont exist
    if (!surface1)
    {
        SDL_Log("TTF_RenderText failed: %s", TTF_GetError());
        return;
    }
    if (!surface2)
    {
        SDL_Log("TTF_RenderText failed: %s", TTF_GetError());
        return;
    }

    // create textures from surfaces
    SDL_Texture* texture1 = SDL_CreateTextureFromSurface(gameRenderer, surface1);
    SDL_Texture* texture2 = SDL_CreateTextureFromSurface(gameRenderer, surface2);

    SDL_FreeSurface(surface1); // ALWAYS FREE SURFACES
    SDL_FreeSurface(surface2);

    // log message if textures dont exist
    if (!texture1)
    {
        SDL_Log("CreateTextureFromSurface failed: %s", SDL_GetError());
        return;
    }

    if (!texture2)
    {
        SDL_Log("CreateTextureFromSurface failed: %s", SDL_GetError());
        return;
    }

    // DISPLAY THE TEXTS FOR BOTH USING DIFFERENT DEST RECTS TO POSITION THEM 
    SDL_Rect destRect1 = { 10, 10, 0, 0 };
    SDL_Rect destRect2 = { 570, 10, 0, 0 };
    SDL_QueryTexture(texture1, nullptr, nullptr, &destRect1.w, &destRect1.h);
    SDL_QueryTexture(texture2, nullptr, nullptr, &destRect2.w, &destRect2.h);

    SDL_RenderCopy(gameRenderer, texture1, nullptr, &destRect1);
    SDL_RenderCopy(gameRenderer, texture2, nullptr, &destRect2);
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
}


// DISPLAYING GAME OVER SCREEN
void displayGameOverScreen()
{
    // display just the black screen
    SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gameRenderer);

    // display the victory message
    std::string text;

    if (game_data.player1Health > 0 && game_data.player2Health <= 0)
    {
        // player 1 wins as player 2 will die 
        text = "Player 1 wins!";
    }

    else if (game_data.player2Health > 0 && game_data.player1Health <= 0)
    {
        // player 2 wins if player 1 dies
        text = "Player 2 wins!";
    }

    else if (game_data.player1Health <= 0 && game_data.player2Health <= 0)
    {
        // if both players manage to die then its a draw
        text = "The game is a draw as both players died";
    }

    // text color is white
    SDL_Color textColor = { 255, 255, 255, 255 };

    // create surface, texture
    SDL_Surface* surface = TTF_RenderText_Blended(gameFont, text.c_str(), textColor);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(gameRenderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    // display the text at dest rect
    SDL_Rect destRect;
    SDL_QueryTexture(texture, nullptr, nullptr, &destRect.w, &destRect.h);
    destRect.x = (800 - destRect.w) / 2; // centre of screen as game window is 800 x 600
    destRect.y = (600 - destRect.h) / 2;

    SDL_RenderCopy(gameRenderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);

    // display the text
    SDL_RenderPresent(gameRenderer);
}


// CHECK GAME STATE
void checkGameState() {
    // game over is true if either player dies
    if (game_data.player1Health <= 0 || game_data.player2Health <= 0)
    {
        isGameOver = true;
    }
}



// ----- THE MAIN LOOP -----
int main(int argc, char** argv) {

    cout << "Starting Client... " << endl;
    // PlayerCharacter* player = nullptr;


    IPaddress ip;


    // initialise the SDL Video system; this is for rendering graphics and displaying the game
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Video failed: %s\n", SDL_GetError());
        return 1;
    }

    // initialise SDL_net; this is for handling all network-related information etc
    if (SDLNet_Init() == -1) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    // initialise SDL_image for png loaading; this is for loading the player sprites and other images
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return 2;
    }

    // load up TFF for displaying UI and text
    if (TTF_Init() == -1)
    {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return -1;
    }
    // check if audio works
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("SDL_mixer failed: %s\n", Mix_GetError());
    }
    // load game music
    Mix_Chunk* gameMusic = Mix_LoadWAV("../assets/audio/BitMan_TitleTheme_loop.wav"); // SOURCED FROM OPEN GAME ART AND AVAILABLE IN REFERENCES
    if (!gameMusic)
    {
        SDL_Log("Failed to load background music: %s", Mix_GetError());
    }


    // resolve host (ip name + port) into an IPaddress type
    if (SDLNet_ResolveHost(&ip, IP_NAME, PORT) == -1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(3);
    }

    // open the connection to the server
    TCPsocket socket = SDLNet_TCP_Open(&ip); // game uses TCP socket

    if (!socket) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(4);
    }

    // CREATE THE THREADS
    SDL_CreateThread(on_receive, "ConnectionReceiveThread", (void*)socket);
    SDL_CreateThread(on_send, "ConnectionSendThread", (void*)socket);

    // assign gameFont
    gameFont = TTF_OpenFont("../assets/fonts/Lora-Regular.ttf", 24);
    if (!gameFont)
    {
        SDL_Log("Could not load font: %s", TTF_GetError());
        return -1;
    }


    // -----------------------
   // Create Window and Renderer
   // -----------------------
    SDL_Window* window = SDL_CreateWindow("SDL Player Client Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN);

    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 6;
    }

    // CREATE THE GAME RENDERER
    gameRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!gameRenderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 7;
    }


    // SEND A REQUEST FOR MAP DATA

    std::string msg = "REQUEST_MAP_DATA";
    SDLNet_TCP_Send(socket, msg.c_str(), msg.length());

    // !!!!!!!!!!!!!!!!!!!! CREATE MAP
    const int NUM_TILES = 2; // 0 = floor, 1 = wall. in the future can add more
    SDL_Texture* tileTextures[NUM_TILES]; // create tile textures
    const int TILE_SIZE = 32; // tiel size is 32x32
    // Load textures
    tileTextures[0] = IMG_LoadTexture(gameRenderer, "../assets/sprites/floor.png");
    tileTextures[1] = IMG_LoadTexture(gameRenderer, "../assets/sprites/wall.png");

    // log message if failed to load the textures
    if (!tileTextures[0]) printf("Failed loading floor.png: %s\n", IMG_GetError());
    if (!tileTextures[1]) printf("Failed loading wall.png: %s\n", IMG_GetError());
    game_data.mapReadyForRendering = true; // THIS FLAG IS TO MAKE SURE THAT THE RENDERED IS ACTUALLY READY AND THE MAP IS READY TO BE RENDERED.

    // -----------------------
    // Create Player Characters
    // -----------------------
    // get client ID and spawn positions
    if (game_data.hasClientID)
    {
        cout << "Received Client Id = " << game_data.clientID << endl;
        cout << "Spawn positions for plaayer: " << game_data.startX << " " << game_data.startY << endl;
    }

    else
    {
        cout << "Waiting for ID from server. Current Default ID = " << game_data.clientID << endl;
        game_data.isSpectator = true;
    }
    // PlayerCharacter player2(2, "assets/player2.png", globalRenderer, 600, 300);

    // -----------------------
    // Main loop
    // -----------------------
    SDL_Event event;
    while (is_running) {
        if (!musicStarted) // play the game music if it isnt already playing
        {
            Mix_PlayChannel(-1, gameMusic, -1);
            musicStarted = true;
        }
        // GET THE CURRENT TIME USING GET TICKS
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTick) / 1000.0f; // DETERMINE DELTA TIME
        lastTick = currentTime; // SET LASTTICK TO CURRENT TIME

        while (SDL_PollEvent(&event)) {

            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                game->input(event); // handle input

                // escape key => exit gaem
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    is_running = false;
                    break;

                default:
                    break;
                }


                if (event.type == SDL_QUIT) is_running = false;

            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);

        // if the local client hasnt been created yet and an id has been received by the server create the local player and add that player to the players map
        if (!game_data.localClientCreated && game_data.hasClientID)
        {
            if (game_data.players.find(game_data.clientID) == game_data.players.end())
            {
                createLocalPlayer();
                game_data.localClientCreated = true;
            }



        }

        /* OLD CODE: DOESNT WORK
        for (auto player = game_data.remoteSpawnList.begin(); player != game_data.remoteSpawnList.end(); )
        {
            SpawnInfo& playerSpawnInfo = *player;

            if (game_data.players.find(playerSpawnInfo.id) == game_data.players.end() && game_data.disconnectedPlayers.find(playerSpawnInfo.id) == game_data.disconnectedPlayers.end())
            {
                createRemotePlayer(playerSpawnInfo.id, playerSpawnInfo.x, playerSpawnInfo.y);
                std::cout << "Created a remeote player from list of id: " << playerSpawnInfo.id << "\n";
            }

            player = game_data.remoteSpawnList.erase(player);
        }
        */

        // create remoet player if ready to create it
        if (game_data.readyToCreateRemotePlayer)
        {
            int id = game_data.remotePlayerID;

            if (game_data.players.find(id) == game_data.players.end() && game_data.disconnectedPlayers.find(id) == game_data.disconnectedPlayers.end())
            {
                createRemotePlayer(id, game_data.remoteStartX, game_data.remoteStartY);
            }
        }




        // create player after Id arrives
       // if (!player && game_data.hasClientID)
       // {
          //  cout << "Creating Player with ID = " << game_data.clientID << endl;
          //  player = new PlayerCharacter(game_data.clientID, gameRenderer, game_data.startX, game_data.startY);
       // }

        /** Handle input
        if (player)
        {
            player->handleAnimations(keys);
        }

        player2.handleInput(keys);

        Update
        if (player)
        {
            player->update(1.0f / 60.0f);
        }

        if (player)
        {
            if (game_data.receivedPositionsFromServer)
            {
                player->setPos(game_data.playerX, game_data.playerY);
            }
        }
        / player2.update(1.0f / 60.0f);

         // Render
        SDL_SetRenderDrawColor(globalRenderer, 0, 0, 0, 255);
        SDL_RenderClear(globalRenderer);

        if (player)
        {
            player->render(globalRenderer);
        }

        //player2.render(renderer);
        **/






        SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gameRenderer);


        // crate the map when ready to
        if (game_data.mapDataReceived && game_data.levelLoaded && game_data.mapReadyForRendering)
        {
            // interate through the map rows and columns and create and render the correct tile at the correct positions
            for (int y = 0; y < game_data.mapHeight; ++y) {
                for (int x = 0; x < game_data.mapWidth; ++x) {
                    int tileId = game_data.levelMap[y][x];
                    SDL_Texture* texture = tileTextures[tileId];
                    SDL_Rect destRect;
                    destRect.x = x * TILE_SIZE;
                    destRect.y = y * TILE_SIZE;
                    destRect.w = TILE_SIZE;
                    destRect.h = TILE_SIZE;

                    SDL_RenderCopy(gameRenderer, texture, nullptr, &destRect);
                }

            }
        }
        else // if not ready then log message
        {
            // IF SPECTATOR THEN LOG DIFFERENT MESSAGE
            if (game_data.isSpectator)
            {
                cout << "Spectators cannot retieve map data." << endl;
            }

            else
            {
                cout << "Map data not received yet.\n";
            }
            
        }

        //check game state
        checkGameState();

        // render and update players
        renderPlayers(gameRenderer);
        updatePlayers(deltaTime, keys);

        // manage projectile creation and handling
        creatingProjectiles();
        handleProjectiles(gameRenderer, deltaTime);

        // display UI LAST or else it doesnt show on the screen
        if (!game_data.isSpectator) // only show UI to players not to spectators. Without this if a third player joins they can actually see the UI when they shouldnt be able to
        {
            displayGameUI();
        }
        

        // if game is over then display gameover screen
        if (isGameOver)
        {
            displayGameOverScreen();
        }

        // display game world
        SDL_RenderPresent(gameRenderer);

        SDL_Delay(16);

    }

    // run_game();

    // delete game;

    if (!is_running)
    {
        std::string msg = "CLIENT_DISCONNECT"; // send client disconnection message when game isnt running so disconnection can be handled cleanly
        SDLNet_TCP_Send(socket, msg.c_str(), msg.length());
        // wait briefly to let TCP send buffer flush
        SDL_Delay(50);  // 50 ms is usually enough
    }

    // !!!!!!!!!! LEVEL CLEANUP
    for (int i = 0; i < NUM_TILES; ++i) {
        SDL_DestroyTexture(tileTextures[i]); // essential to clean up level tiles properly
    }
    // ------------------
    // 
    // 
    // close connection to the server
    SDLNet_TCP_Close(socket);

    // shutdown and quit everything
    SDLNet_Quit();
    Mix_HaltChannel(-1);
    Mix_FreeChunk(gameMusic);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;


}

