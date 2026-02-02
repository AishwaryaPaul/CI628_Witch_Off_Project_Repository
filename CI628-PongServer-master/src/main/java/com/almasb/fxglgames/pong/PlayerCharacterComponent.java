package com.almasb.fxglgames.pong;

import com.almasb.fxgl.dsl.FXGL;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.entity.component.Component;
import com.almasb.fxgl.physics.PhysicsComponent;
import javafx.geometry.Point2D;

import static com.almasb.fxgl.dsl.FXGL.*;


public class PlayerCharacterComponent extends Component {
    private PhysicsComponent physics; // this is the physics component

    private final int playerId; // store player id
    private int health = 100; // health variable for applying damage later
    private int maxHealth; // probably dont need max health since player wont really respawn and there
    // might not be time to add a restart game option

    // movement states and vars
    private double x,y; // x and y positions
    private double velocityX, velocityY; // velocity vars
    private static final double speed = 140; // speed at which player moves

    // input states
    private boolean up, down, left, right;  // movement states
    private boolean playerCastSpell; // state to check if player cast a spell
    // this isnt accessed since i ultimately decided not to add spell projectile cooldown, lifetime, etc here

    public PlayerCharacterComponent(int id) {
        this.playerId = id; // seet player id
    }

    // getter method for retrieving the player id
    public int getPlayerId() {
        return playerId;
    }

    // server calls this after receiving client input
    public void applyInput(PlayerActionStates input) {
        // set component's internal movement/input states basted on what was received from the parameter's input
        up = input.up;
        down = input.down;
        left = input.left;
        right = input.right;
        playerCastSpell = input.castSpell;
    }
    // was not necessary to abstract the input states with this. The player character comp could just have been retrieved
    // in the onReceive method and these vars could have just been set from there

    // update method: apply movement if keys were pressed, set velocity, etc
    @Override
    public void onUpdate(double tpf) {
        // DEBUG: System.out.println("Player velocity Y: " + vy + " and velocity X: " + vx);

        velocityX = 0;  // velocity x
        velocityY = 0;   // velocity y

        // alter velocity in appropriate dimension upon movement
        if (up) velocityY -= speed;
        if (down) velocityY += speed;
        if (left) velocityX -= speed;
        if (right) velocityX += speed;

        // entity.translate(vx * tpf, vy * tpf); its better to use physics than to use entity translate
        physics.setLinearVelocity(velocityX, velocityY); // move the player by setting linear velocity
    }

    // damage player and reduce health by damage amount
    public void damage(int amount) {
        health -= amount; // if damage is called then reduce player health


        // if health is equal to or below 0 then set health to 0 and call the die method
        if (health <= 0) {
            health = 0;
            // killPlayer(); do not call this method because the GAME UPDATE message keeps sending the health updates to clients
            // the server application will crash if entity is removed and cant find the entity component to get health
            // need to have the component so that the game over and restart sequence can occur
        }

    }

    // removes player entity frmo game world; this isnt really needed ultimately in the game
    private void killPlayer() {
        entity.removeFromWorld(); // remove the entity from the game world upon death

    }

    // get health of player (used for sending messages to clients about player health so it can display it in UI)
    public int getHealth()
    {
        return health;
    }

}
