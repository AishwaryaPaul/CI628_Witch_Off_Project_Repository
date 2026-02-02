/*
 * The MIT License (MIT)
 *
 * FXGL - JavaFX Game Library
 *
 * Copyright (c) 2015-2017 AlmasB (almaslvl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
package com.almasb.fxglgames.pong;
import com.almasb.fxgl.app.ApplicationMode;
import com.almasb.fxgl.app.GameApplication;
import com.almasb.fxgl.app.GameSettings;
import com.almasb.fxgl.dsl.FXGL;
import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.net.*;
import com.almasb.fxgl.physics.CollisionHandler;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.ui.UI;
import javafx.scene.paint.Color;
import javafx.util.Duration;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import static com.almasb.fxgl.dsl.FXGL.*;


/**
 * A simple clone of Pong.
 * Sounds from https://freesound.org/people/NoiseCollector/sounds/4391/ under CC BY 3.0.
 *
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class PongApp extends GameApplication implements MessageHandler<String> {

    // Game settings: title, version, font UI, application mode.
    @Override
    protected void initSettings(GameSettings settings) {
        settings.setTitle("Witch Off"); // game title
        settings.setVersion("1.0"); // game version
        settings.setFontUI("pong.ttf"); // UI font
        settings.setApplicationMode(ApplicationMode.DEBUG); // debug mode
    }

    /** create an entity for the player client - an entity is like a unique ID
     multiple entities do not need to be created, just one TYPE of entity for the player works; multiple instances
     of this one type of entity can be made **/
    int playerCount = 0;
    Map<Integer, Entity> playerEntities = new HashMap<>(); // all player entities stored in this hashmap for ease of access
    /** keep a hashmap of playerComponents to keep track of the components of the players
     (each connection should have one type of entity, and a PlayerCharacterComponent for itself) **/
    Map<Integer, PlayerCharacterComponent> playerComponents = new HashMap<>();
    Map<Integer, PlayerActionStates> inputStates = new HashMap<>(); // hashmap for keeping movement states for the players


    // hold the server
    private Server<String> server;

    /** this array is used to hold the "player slots": maximum amount of players which could connect
     holding this in an array is better for scalability: max allowed players variable value can be quickly changed
     to allow for more player connections if the scope of the game was to change in the future*/
    private final int maxAllowedPlayers = 2;
    // boolean client1Exists = false;
    // boolean client2Exists = false;

    // PlayerClient array holding a maximum of 2 players (can be increased in the future to accomodate further clients easily)
    public PlayerClient[] players = new PlayerClient[maxAllowedPlayers];

    private Map<Integer, Entity> activeProjectiles = new HashMap<>(); // stores active projectiles in a  hashmap

    // ========================== INITIALISE THE GAME ====================================
    @Override
    protected void initGame() {

        // initialise the player slots
        initPlayerSlots();

        // add the TCP Writer and Reader
        Writers.INSTANCE.addTCPWriter(String.class, outputStream -> new MessageWriterS(outputStream));
        Readers.INSTANCE.addTCPReader(String.class, in -> new MessageReaderS(in));

        // set the server
        server = getNetService().newTCPServer(55555, new ServerConfig<>(String.class));

        // handle incoming connections and call the handlePlayerConnection method, and add a message handler
        server.setOnConnected(connection -> {
            System.out.println("Incoming connection request from client");
            handlePlayerConnection(connection);
            connection.addMessageHandlerFX(this);
        });


        // disconnection should not be handled here; it should be handled when client sends a disconnection message
        // it is not appropriate to handle disconnection during the initialisation method
      //  server.setOnDisconnected(connection -> {
          //  int clientID = connection.getLocalSessionData().getValue("ID");
         //   handlePlayerDisconnection(clientID);

      //  });

        // register entity factory so that entities can be created
        getGameWorld().addEntityFactory(new PongFactory());
        // set the game scene background color
        getGameScene().setBackgroundColor(Color.rgb(0, 0, 5));



        //initScreenBounds();
        // initGameObjects();



        // start a new server thread
        var t = new Thread(server.startTask()::run);
        t.setDaemon(true);
        t.start();
    }

    // ========================  INITIALISE PHYSICS: USES FXGL COLLISION HANDLER =============================
    @Override
    protected void initPhysics() {
        getPhysicsWorld().setGravity(0, 0); // set gravity to 0 else the player is affected by gravity
        // PROJECTILE AND PLAYER COLLISION
        getPhysicsWorld().addCollisionHandler(
                new CollisionHandler(EntityType.PROJECTILE, EntityType.PLAYER) {

                    @Override
                    protected void onCollisionBegin(Entity projectile, Entity player) {
                        PlayerCharacterComponent playerComponent = player.getComponent(PlayerCharacterComponent.class);
                        ProjectileComponent projectileComponent = projectile.getComponent(ProjectileComponent.class);
                        // projectiles do collide with their own player so this should be ignored (this doesnt stop projectiles from applying some force to the player characters though)
                        if (projectileComponent.getOwnerId() ==
                                playerComponent.getPlayerId()) {
                            System.out.println("Projectile collided with owner player"); // collision masks would need to be studied to avoid impulse or force from being applied to the players
                            return;
                        }
                        else // else if projectile collides with another player (not of the same id which launched it)
                        {
                            // damage the player
                            playerComponent.damage(projectileComponent.getProjectileDamageAmount()); // best to define damage amount in spawnProjectile method where the projectiles are actually created to ensure that different damage values arent being assigned in different places);
                            System.out.println("Projectile Damage: " + projectileComponent.getProjectileDamageAmount() + " to Client Id: " + playerComponent.getPlayerId());
                            System.out.println("Player health is now: " + playerComponent.getHealth());
                            projectile.removeFromWorld(); // remove projectile from world

                            // WHEN REMOVING FROM WORLD ALSO ADD A BROADCAST MESSAGE LATER TO MAKE SURE CLIENTS ARE TOLD THAT ITS GONE. CLIENTS SHOULD STOP RENDERING THE CORRECT PROJECTILE DETERMINED BY ITS ID WHEN THIS MESSAGE IS RECEIVED.
                            server.broadcast("PROJECTILE_DEATH" + "," + projectileComponent.getProjectileId());

                        }
                    }
                }
        );

        // COLLISION BETWEEN PROJECTILE AND WALL
        getPhysicsWorld().addCollisionHandler(
                new CollisionHandler(EntityType.PROJECTILE, EntityType.WALL) {

                    @Override
                    protected void onCollisionBegin(Entity projectile, Entity wall) {

                        // if projecitle hits a wall then remove it from the world
                        ProjectileComponent projectileComponent = projectile.getComponent(ProjectileComponent.class);
                        projectile.removeFromWorld(); // remove projectile from world
                        // projectile death message should be sent
                        server.broadcast("PROJECTILE_DEATH" + "," + projectileComponent.getProjectileId());

                    }
                }
        );

    }


    // ======= UPDATE METHOD: broadcast player positions to all clients as long as there are active connections (clients)

    @Override
    protected void onUpdate(double tpf) {
        if (!server.getConnections().isEmpty()) {

            /** If the player entity is not created yet then return; position updates should not be broadcast in this case
             if the player entity is null and the message is broadcast it will cause the server to crash */
            for (Map.Entry<Integer, Entity> player : playerEntities.entrySet()) {

                int id = player.getKey(); // the key is the player/client id
                // DEBUG: System.out.println("Sending movement data of player " + id);
                Entity playerEntity = player.getValue(); // the value of the playerEntities hashmap is hte player entity

                if (playerEntity == null) // ignore if entity is null
                    continue;

                var gameDataMessage = "GAME_DATA," + id + "," + playerEntity.getX() + "," + playerEntity.getY() + "," + playerEntity.getComponent(PlayerCharacterComponent.class).getHealth();
                server.broadcast(gameDataMessage);




            }

            // only broadcast every so often for the projectiles otherwise it causes MASSIVE amounts of lag for the other player
            // and it just looks like the second player is teleporting around due to lag, making movement extremely difficult
            long lastBroadcastTime = System.currentTimeMillis();
            long broadcastInterval = 100;  // broadcast interval time

            for (Map.Entry<Integer, Entity> activeProjectile : activeProjectiles.entrySet()) {
                int projectileId = activeProjectile.getKey();
                Entity projectileEntity = activeProjectile.getValue();

                if (projectileEntity == null) continue;

                long currentTime = System.currentTimeMillis();
                if (currentTime - lastBroadcastTime > broadcastInterval) {
                    // BROADCAST PROJETILE UPDATE TO ALL CLIENTS
                    var broadcastProjectileInfo = "PROJECTILE_UPDATE," + projectileId + "," + projectileEntity.getX() + "," + projectileEntity.getY();
                    server.broadcast(broadcastProjectileInfo);
                    lastBroadcastTime = currentTime;  // update last broadcast time
                }
            }

            //APPLY CORRECT INPUT TO EACH OF THE PLAYERS
            for (int player : playerEntities.keySet()) {
                PlayerActionStates input = inputStates.get(player);
                applyInputToPlayer(player, input);
            }

        }
    }

    //private void initScreenBounds() {
    //Entity walls = entityBuilder()
    //    .type(EntityType.WALL)
    //    .collidable()
    //    .buildScreenBounds(150);

    // getGameWorld().addEntity(walls);
    // }

    private void initGameObjects() {
        // ball = spawn("ball", getAppWidth() / 2 - 5, getAppHeight() / 2 - 5);
        // player1 = spawn("bat", new SpawnData(getAppWidth() / 4, getAppHeight() / 2 - 30).put("isPlayer", true));
        // player2 = spawn("bat", new SpawnData(3 * getAppWidth() / 4 - 20, getAppHeight() / 2 - 30).put("isPlayer", false));

    }

    // ================================= PLAYER AND PLAYER CLIENT STUFF ==========================================
    private void initPlayerSlots() {
        // each player will have its own "slot", for my game i decided to keep 2 slots for 2 players
        for (int i = 0; i < players.length; i++) {
            players[i] = new PlayerClient(i + 1, 0, 0); // add 1 else the first id becomes 0
        }
    }

    public void applyInputToPlayer(int playerId, PlayerActionStates playerInputAction) {
        PlayerCharacterComponent playerComponent = playerComponents.get(playerId);
        if (playerComponent != null) { // if the player component is found then apply input
            playerComponent.applyInput(playerInputAction);
        }
        else {
            // this is mostly for debugging to check if hte component is found or not because
            // there seemed to be a problem with the component never being applied to the entity for some reason
            System.out.println("Player " + playerId + " not found");
        }
    }

    // HANDLE THE INCOMING CONNECTIONS
    private void handlePlayerConnection(Connection<String> connection) {

        PlayerClient assignedClient = null;
        // loop through each player in the players array (which stores connected players). For each player which is in that list
        // but isn't connected, set the isConnected bool to true, store the player in the assigned variable
        for (PlayerClient playerClient : players) {
            if (!playerClient.isConnected) {
                playerClient.isConnected = true;
                assignedClient = playerClient;
                playerCount++; // also increment player count (mainly for debugging)
                System.out.println(playerCount);
                break;
            }
        }
        // if slots aare full and another incoming connection doesnt have an assigned client then assign it  -1 Id
        if (assignedClient == null) {
            // No free slots → spectator
            connection.getLocalSessionData().setValue("ID", -1);
            System.out.println("No free slots left in game. Assigning client an Id of -1");
            return;
        }

        // assign the correct id to the clients
        connection.getLocalSessionData().setValue("ID", assignedClient.clientID);
        System.out.println("Assigned Id = " + assignedClient.clientID);


        // create spawn for the player
        createPlayerSpawn(assignedClient.clientID);
        // initialise the input states
        inputStates.put(assignedClient.clientID, new PlayerActionStates());


        // send spawn message to the client
        String clientIDMessage = "CLIENT_SPAWN_DATA,"+assignedClient.clientID+ "," +assignedClient.startX+ ","+ assignedClient.startY;
        System.out.println("SpawnX: " +assignedClient.startX + " SpawnY: " +assignedClient.startY);
        connection.send(clientIDMessage); // ONLY SEND LOCAL CLIENT SPAWN DATA TO THE LOCAL CLIENT, NOT TO OTHER CLIENTS


        // send the data of existing remote players to the new client
        for (PlayerClient playerClient : players) {
            if (playerClient.isConnected && playerClient.clientID != assignedClient.clientID) {
                connection.send("REMOTE_SPAWN_DATA," + playerClient.clientID + "," + playerClient.startX + "," + playerClient.startY);
            }
        }

        // broadcast the new player to other clients
        String message = "REMOTE_SPAWN_DATA," + assignedClient.clientID + "," + assignedClient.startX + "," + assignedClient.startY;
        // using a for loop here instead of just using broadcast since each client doesnt need to know its spawn data again,
        // thats already been done in CLIENT_SPAWN_DATA
        for (Connection<String> clientConnection : server.getConnections()) {
            if ( clientConnection != connection) {
                clientConnection.send(message);
            }
        }

// spawn the player entity for the client
        spawnPlayerEntity(assignedClient);
    }

    // HANDLE PLAYER DISCONNECTION
    private void handlePlayerDisconnection(int clientID) {
        System.out.println("Client " + clientID + " disconnected."); // log a message showing which client disocnnected

        // remove disconnected entity
        Entity playerEntity = playerEntities.remove(clientID);
        if (playerEntity != null) {
            playerEntity.removeFromWorld();// remove the entity from the game world
        }
        playerComponents.remove(clientID); // remove the entity's component from the player components map

        // free up the PlayerClient slot: this is essential else a "reconnecting" client wont be able to take up an
        // available spot after a player disconnects
        for (PlayerClient playerClient : players) { // loop through players
            if (playerClient != null && playerClient.clientID == clientID) { // if found the disconnected player
                playerClient.isConnected = false; // set isConnected to false
                playerClient.startX = 0; // reset startX and startY values
                playerClient.startY = 0;
                break;

            }
        }
        playerCount--;// decrement player count also
        System.out.println("Player count is:" + playerCount); // log playercount to console for checking

        // broadcast to other players when another client disconnects
        server.broadcast("PLAYER_DISCONNECTED," + clientID);
    }


    // HANDLE PLAYER SPAWN
    private void spawnPlayerEntity(PlayerClient playerClient)
    {
        // this must run on the application thread therefore the spawning entity code block needs to be
        // wrapped in FXGL.runOnce(). this block of code was previously being run on Thread 4 which was
        // causing an error and the entity would not actually spawn properly
        FXGL.runOnce(() ->
        {

// spawn the player entity
            Entity playerEntity = spawn("player", new SpawnData(playerClient.startX,playerClient.startY)
                    .put("playerId", playerClient.clientID)
            );

            System.out.println("Spawned a mew Entity"); // liink the component
            PlayerCharacterComponent playerComponent = playerEntity.getComponent(PlayerCharacterComponent.class);
            playerClient.setPlayerComponent(playerComponent);
            playerComponents.put(playerClient.clientID, playerComponent);
            playerEntities.put(playerClient.clientID, playerEntity);
            if (playerComponent == null)
            {
                System.out.println("Couldnt find component for spawn");
            }
            else
            {
                System.out.println("Player comp found");
            }
        }, Duration.ZERO);
    }

    // CREATE THE PLAYER'S SPAWN
    private void createPlayerSpawn(int id) {
        int spawnX;
        int spawnY;

        // the spawn X and Y positions are different for both players, therefore the id of the player
        // must be checked before the spawnX and spawnY values are determined
        if (id == 1)
        {
            spawnX = 500;
            spawnY = 500;
        }
        else if (id == 2) {
            spawnX = 200;
            spawnY = 200;
        }
        else // an else statement for values of spawnX and spawnY so that they do get initialised in any circumstance
        // (without this line there is an error stating that these two variables may not be initialised). the x,y could just be initialised without this too.
        {
            spawnX = 0;
            spawnY = 0;
        }

        players[id-1].startX = spawnX; // id must be reduced by 1 since arrays start from 0
        players[id-1].startY = spawnY;
        // after determining the spawnX and spawnY positions, player becomes ready to spawn.
        // could add a "readyToSpawn" boolean variable to ensure the entity is ready to spawn also.
    }

    // ================================= LEVEL CREATION STUFF ==========================================
    public static final int TILE_SIZE = 32; // TILE SIZE OF THE MAP TILES. EACH MEASURES 32X32 PIXELS

    private int[][] constructLevelMap()
    {
        // THE MAP IS CREATED AS AN INTEGER ARRAY HERE
        int[][] map = {
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        };

        FXGL.runOnce(() -> {
            for (int y = 0; y < map.length; y++)
            {
                for (int x = 0; x < map[y].length; x++) {
                    int mapTile = map[y][x]; // ARRAYS ARE Y,X NOT X,Y IN JAVA (AND IN C++)
                    // used a switch statement instead of an if statement as originally i intended to add more objects like powerups however due to time constraints this was not prioritised and ultimately was not implemented.
                    switch (mapTile) {
                        case 1:
                            spawn("wall", x * TILE_SIZE, y * TILE_SIZE);
                            break;
                        // rather than sending spawn data i could have just added case 2 and spawned player at the locations
                        // client could then have retrieved values from there on
                        default:
                            // 0 is empty space
                    }
                }
            }

        }, Duration.ZERO);

        return map; // return the map so later it can accessed for serializing the data

    }

    // LEVEL DATA NEEDS TO BE SERIALIZED SINCE IT NEEDS TO BE SENT AS A STRING TO CLIENTS
    // COMMAS CANNOT BE USED AS THE DELIMITERS: THIS WAS DONE INITIALLY AND THIS JUST CAUSED THE
    // PARSING TO BE CARRIED OUT INCORRECTLY SINCE COMMAS ARE USED BY THE PARSER TO DIFFERENTIATE ARGUMENTS.
    // IF TILE DATA IS SEPARATED BY COMMAS THEN EACH NEW TILE VALUE IS GONNA BE CONSIDERED AN ENTIRELY DIFFERENT ARGUMENT.
    private String serializeLevelData(int[][] map) {
        StringBuilder stringLevelData = new StringBuilder();

        for (int y = 0; y < map.length; y++) {
            for (int x = 0; x < map[y].length; x++) {
                stringLevelData.append(map[y][x]);
                if (x < map[y].length - 1) { // ROWS
                    stringLevelData.append(":"); // USING COLON DELIMITER FOR SEPARATING TILE DATA
                }
            }
            if (y < map.length - 1) { // COLUMNS
                stringLevelData.append(";"); // USING SEMI-COLON DELIMITER TO SPLIT ROW DATA
            }
        }

        return stringLevelData.toString(); // CONVERT IT TO STRING SO IT CAN BE SENT TO CLIENTS
    }

    private void sendLevelDataToClients(Connection<String> connection)
    {
        int[][] map = constructLevelMap(); // retrieve the map
        String serializedMapData = serializeLevelData(map); // serialise the map data
        int mapWidth = map[0].length; // store width and heightof map
        int mapHeight = map.length;
        String widthString = String.valueOf(mapWidth); // store their values in strings
        String heightString = String.valueOf(mapHeight);

        // level data message must be sent: widthm height and the serialised map mustbe sent in this message
        String message = "LEVEL_DATA," + widthString + "," + heightString + "," + serializedMapData;
        connection.send(message); // send level data to each connection when they connect

        // also sending a level update complete message to ensure the update is compelete.
        // client must ONLY render the map after it receives this message and sets its readyToRenderMap variable to true.
        connection.send("LEVEL_UPDATE_COMPLETE");
    }

    // ============================ PROJECTILE STUFF ==============================================
    public void spawnProjectile(int clientID, double targetX, double targetY, int projectileId) {

        System.out.println("Creating a projectile entity to spawn"); // DEBUG

        // get the entity which launched the projectile
        Entity playerEntity = playerEntities.get(clientID);
        if (playerEntity == null)
            return;

        PlayerCharacterComponent playerComponent = playerEntity.getComponent(PlayerCharacterComponent.class);

        // projectile's start x and y should be player's x and y
        // could also add an offset to offset the spawn location of projectile so it doesnt always collide with its own player upon spawn
        double projectileStartX = playerEntity.getX();
        double projectileStartY = playerEntity.getY();

        System.out.println("Client is at: " + projectileStartX + ", " + projectileStartY);

        // spawning projectile
        Entity projectile = spawn(
                "newProjectile",
                new SpawnData(playerEntity.getX(), playerEntity.getY())
                        .put("mouseX", targetX)
                        .put("mouseY", targetY)
                        .put("ownerId", clientID)
                        .put("projectileDamageAmount", 25)
                        .put("projectileId", projectileId)
                        .put ("startX", projectileStartX)
                        .put("startY", projectileStartY)
        );

        ProjectileComponent projectileComponent = projectile.getComponent(ProjectileComponent.class);

        System.out.println("Projectile of Id: " + projectileComponent.getProjectileId() + " created at Mouse X: " + targetX + " and Mouse Y: " + targetY + " spawned by Player Id: " + playerComponent.getPlayerId());


        // add projectile to active projectiles
        activeProjectiles.put(projectileId, projectile);

        // projectile spawn message is sent to player to validate that client can spawn the projectile (client will still locally simulate its movement too)
        var spawnMessage = "PROJECTILE_SPAWN" + "," + projectileComponent.getProjectileId() + "," + playerEntity.getX() + "," + playerEntity.getY() + "," + targetX + "," + targetY;
        server.broadcast(spawnMessage); // it must be sent to all players therefore broadcast instead of connecton.send(spawnMessage).
        System.out.println("Spawned a projectile");
    }




    // ===================================== RECEIVING MESSAGES FROM CLIENTS =================================
    @Override
    public void onReceive(Connection<String> connection, String message) {
        // store the client ID by retrieving it from the local session data
        int clientID = connection.getLocalSessionData().getValue("ID");
        if (clientID < 0) return; // return if no client id


        PlayerActionStates playerInputStates = inputStates.get(clientID); // get the input states of client
        if (playerInputStates == null) return;  // if it doesnt exist then return
        String[] tokens = message.split(","); // an array of strings called "tokens"
        // check for disconnect first
        for (String token : tokens) {
            if (token.equals("CLIENT_DISCONNECT")) {
                handlePlayerDisconnection(clientID);
                return; // if disconnected then handle player disconnection and return
            }
            // AFTER CONNECTION CLIENTS REQUEST MAP DATA BEFORE CREATING PLAYERS IN GAME
            if (token.equals("REQUEST_MAP_DATA")) // when this request comes send level data to the clients
            {
                System.out.println("Client requested map data. Sending map data...");
                sendLevelDataToClients(connection); // level data sent to clients
            }

            // CHECK IF PLAYER WANTED TO FIRE A PROJECTILE
            if (token.equals("FIRE")) // when this request comes send level data to the clients
            {
                System.out.println("Client requested to fire a projectile.");
                // parse the target X and Y, and also the projectile Id
                double targetX = Integer.parseInt(tokens[2]);
                double targetY =Integer.parseInt(tokens[3]);
                int projectileId = Integer.parseInt(tokens[4]);
                playerInputStates.castSpell = true; // set player cast spell to true
                spawnProjectile(clientID, targetX, targetY, projectileId); // spawn projectile
                playerInputStates.castSpell = false; // set player cast spell to false
                // currently because of this system the player can keep firing projectiles as much as they want (there is no cooldown), so officially every key press of F or G the player can launch a projectile
            }
        }




        for (String key : tokens) {
            switch (key) {

                // player 1 controls
                case "W_DOWN": if (clientID == 1) playerInputStates.up = true;  break;
                case "W_UP": if (clientID == 1) playerInputStates.up = false; break;

                case "S_DOWN": if (clientID == 1) playerInputStates.down = true; break;
                case "S_UP": if (clientID == 1) playerInputStates.down = false; break;

                case "A_DOWN": if (clientID == 1) playerInputStates.left = true; break;
                case "A_UP": if (clientID == 1) playerInputStates.left = false; break;

                case "D_DOWN": if (clientID == 1) playerInputStates.right = true; break;
                case "D_UP": if (clientID == 1) playerInputStates.right = false; break;



                // ---


                // player 2 controls
                case "I_DOWN": if (clientID == 2) playerInputStates.up = true; break;
                case "I_UP": if (clientID == 2) playerInputStates.up = false; break;

                case "K_DOWN": if (clientID == 2) playerInputStates.down = true; break;
                case "K_UP": if (clientID == 2) playerInputStates.down = false; break;

                case "J_DOWN": if (clientID == 2) playerInputStates.left = true; break;
                case "J_UP": if (clientID == 2) playerInputStates.left = false; break;

                case "L_DOWN": if (clientID == 2) playerInputStates.right = true; break;
                case "L_UP": if (clientID == 2) playerInputStates.right = false; break;

                // ---

            }
        }
    }

    // sending messages to the client(s) over TCP
    static class MessageWriterS implements TCPMessageWriter<String> {

        private OutputStream os; //output stream; sends the stream of data over the socket. The client connection has a "socket",
        // we get the output stream of the socket to then send/receive messages between server-client.
        private PrintWriter out; // wraps os for easier writing of strings

        MessageWriterS(OutputStream os) { // constructor taking os parameter
            this.os = os; // store os
            out = new PrintWriter(os, true); // new "print writer" which wraps "os". Will automatically flush when writing newline.
        }

        @Override
        public void write(String s) throws Exception { // write string method; used to actually send the string message we want to send
            out.print(s.toCharArray()); // convert the string message into character array and print it to output stream
            out.flush(); // force any "buffered" characters to send
        }
    }

    static class MessageReaderS implements TCPMessageReader<String> {

        private BlockingQueue<String> messages = new ArrayBlockingQueue<>(50);

        private InputStreamReader in;

        MessageReaderS(InputStream is) {
            in =  new InputStreamReader(is);

            var t = new Thread(() -> {
                try {

                    char[] buf = new char[36];

                    int len;

                    while ((len = in.read(buf)) > 0) {
                        var message = new String(Arrays.copyOf(buf, len));

                        System.out.println("Recv message: " + message);

                        messages.put(message);
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            });

            t.setDaemon(true);
            t.start();
        }

        @Override
        public String read() throws Exception {
            return messages.take();
        }
    }

    public static void main(String[] args) {
        launch(args);
    }


}
