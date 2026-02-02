package com.almasb.fxglgames.pong;


// player client is separate from the playercharactercomponent as it only holds data related to networking stuff
// such as spawn data (startX and startY), whether the client is connected, etc.
// this is mostly just used when handling the immediate player connection, disconnection, etc (network related stuff. player character comp contains the actual player related things like physics and movement states).
public class PlayerClient {
    public boolean isConnected = false;
    public int clientID  = 0;
    // public int disconnectedID = 0; this isnt needed
    public int startX = 0;
    public int startY = 0;



    // store a reference to the player character component: each player client should have a player character component
    private PlayerCharacterComponent playerComponent;

    public PlayerClient(int clientID, int startX, int startY) {
        this.clientID = clientID; // set client id, startX and startY
        this.startX = startX; // set start x and y
        this.startY = startY;
    }

    // setter method to link the component
    public void setPlayerComponent(PlayerCharacterComponent component) {
        this.playerComponent = component;
    }

    // getter method to access the player component
    public PlayerCharacterComponent getPlayerComponent() {
        return playerComponent;
    }

}
