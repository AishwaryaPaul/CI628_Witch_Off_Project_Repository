
#pragma once
#include <iostream> // include input-output stream
#include <vector> // include vector
#include <string> // include string

#include <SDL.h> // include the SDL Header file

//                  ============================================================          PLAYER CHARACTER CLASS          ============================================================
class PlayerCharacter {
public:

    // -- THE CONSTRUCTOR
    // creating a constructor of the PlayerCharacter class; this constructor will automatically be called whenever an instance of the class is created
    PlayerCharacter(int playerID, SDL_Renderer* renderer, int startX, int startY); // constructor creating parameters for player ID, sprite path,
    // reference for renderer, and starting x and y positions
    // ---
//

// -- THE MAIN METHODS
    void handleAnimations(const Uint8* keyboardState); // for changing animation state depending on the key pressed (up key, down key, etc).
    void PlayerCharacter::handleRemoteAnimations(int oldX, int oldY, float deltaTime); // a separate method for calculating remote player animations based on positions is needed
    void updateAnimations(float dt); // update method
    // void applyReceivedInput();
    void render(SDL_Renderer* renderer); // render method for displaying the player characters
    // Getters for x and y positions so that x and y variables do not have to be made public; its better to keep them private 
    float getX() const { return x; } // getter for x pos
    float getY() const { return y; } // getter for y pos

    // --

    // POSITION METHODS
    void setPos(int x, int y); // set x and y positions for the player char; not to be used unless implementing client side prediction to lpocally predict player movements first 
    void setAuthoritativePositions(int rawX, int rawY); // set the server dictated positions for the player. if implementing client side prediction and server reconciliation do not use since this doesnt interpolate/lerp the differences in positions smoothly this just snaps to the positions


    // -- INTERPOLATION METHOD (LERP)_
    // for linear interpolation the start parameter is the initial value, end param is the tyarget value and alpha represents the interpolation factor
    float lerp(float start, float end, float alpha)
    {
        return start + alpha * (end - start);
    };
    // --

    // -- ANIMATION STUFF
    // 
    // animation states for hte player character are held in an enum class; similar to how enum headings would be used in unity
    enum class AnimationState {
        Idle, // Idle is frame 1 
        WalkingLeft, // frames 6-8 but flippped the opposite way
        WalkingRight, // frames 6-8 
        WalkingDown, // frames 0-2
        WalkingUp //  frames 3-5
    };
    // ---


private:
    int id; // private player id: this is mostly just used for determining what sprite to display for hte player

    // -- texture and sprite stuff
    SDL_Texture* texture; // player texture attribute for the sprite
    const char* spritePath; // for the spritesheet file path
    SDL_Rect srcRect; // player's source rect
    SDL_Rect destRect; // player's destination rect
    // --

    // -- position values and vars
    int spawnPosX, spawnPosY; // s[awn positions for the player
    float x, y; // the actual x and y positions of the player
    float speed = 200.0f; // plauer speed
    int direction = 1; // 1 means right -1 means left: used to determine whether to display walkingLeft or walkingRight (flipped verison of walking elft)
    // velocity variables used for animations
    float velocityX = 0;
    float velocityY = 0;
    // smooth velocity variables for the "smooth" velocity from interpolating later
    float smoothedVelocityX = 0;
    float smoothedVelocityY = 0;
    float serverX, serverY; // the actual serverX and Y positions  of the char
    // --

    // --animation related vars
    SDL_RendererFlip flip = SDL_FLIP_NONE; // FLIP FOR SETTING WALKING RIGHT FRAMES TO BE FLIPPED FOR WALKING LEFT ANIM
    int frameCount = 9;       // Number of frames in the sprite sheet
    int currentFrame = 0;     // Current frame to display
    int frameWidth = 64;      // Width of each frame
    int frameHeight = 64;     // Height of each frame
    int frameTime = 100;      // Time per frame in milliseconds
    Uint32 lastFrameTime = 0; // Last time frame changed
    bool isMoving = false;    // Track whether the player is moving
    AnimationState currentState = AnimationState::Idle; // default anim state is idle
    AnimationState lastRemotePlayerState = AnimationState::WalkingDown; // used to track the last remoet player animstate; walking down by default
    float stateLockTimer = 0.0f; // LOCK TIMER TO START TIMING 
    float stateLockTime = 0.15f;  // ADJUST THIS LOCK TIME FURHTER TO MAKE SURE THE "DELAY" IN ANIMATIONS OF THE REMOTE PLAYER IS LESS NOTICEABLE. 0.15 STILL HAS A SLIGHT DELAY
    // -- 
};



//                  ============================================================          PROJECTILE CLASS         ============================================================
class Projectile {
public:

    // -- THE CONSTRUCTOR
    Projectile(SDL_Renderer* renderer, int startX, int startY); // ref to renderer, startX and startY positions
    // --


    bool isActive = false;     // is active var

    // -- position and movement variables
    void setX(float newX) { x = newX; } // setter for x
    void setY(float newY) { y = newY; } // setter for y
    float getX() { return x; } // getter for x 
    float getY() { return y; } // getter for y
    // --


    // -- MAIN METHODS
    void update(float dt); // update
    void render(SDL_Renderer* r); // render
    void fireAtTarget(float startX, float startY, float targetX, float targetY); // fire at target


    // -- INTERPOLATION
    float lerp(float start, float end, float alpha) // same lerp function for projectiles to smooth projectile movement
    {
        return start + alpha * (end - start);
    };
    // --

private:

    // -- position values
    float x, y = 0; // x y pos
    float velocityX, velocityY = 0; // velocity x y
    float angle = 0; // angle 
    float speed = 0; // speed
    // --


    int id = 0; // id of projectile object though it isnt really used since i just mainly used the projectileData struct for this

    // -- sprite and texture stuff
    const char* spritePath;
    SDL_Texture* texture; // texture atribute for projectile sprite
    SDL_Rect srcRect; // source rect of projectile
    SDL_Rect destRect; // destintion rect of projectile
    // --



    // COULD ADD ANIMATIONS FOR THE SPELLS LATER AS WELL

};
