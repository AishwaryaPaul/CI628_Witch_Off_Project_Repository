package com.almasb.fxglgames.pong;

// this class just holds boolean values for up down left right and cast spell. It essentially acts similar to the way how
// the game data struct in the client code holds variables whose values are set during parsing. The input packet values
// are changed in the "on receive" method in the server application, when input is received from the clients.
// then these values are used to set the corresponding variables in the character component class.

// it might not be very significant to use this but it eliminates the need for retrieving the player component of each
// client in the on receive method to then set the values of their variables from there. It becomes like a clean separation
public class PlayerActionStates
{
    // contains states for player inputs/actions such as pressing up, down left right keys or for firing projectiles
    public boolean up;
    public boolean down;
    public boolean left;
    public boolean right;
    public boolean castSpell; // this is true when the player wants to cast a projectile spell

}
