#include "GameObjects.h"
#include <SDL_image.h>
#include "MyGame.h"

//                  ============================================================          PLAYER CHARACTER CLASS METHODS         ============================================================
                                                                                // Defining the methods of the Player Character Class
PlayerCharacter::PlayerCharacter(int playerID, SDL_Renderer* renderer, int startX, int startY)
{
    // set the id attribute of the player character to the one which was passed through the constructor
    id = playerID;
    // set spawn positions
    spawnPosX = startX;
    spawnPosY = startY;

    // set serv X and Y also since startX and Y will be retrieved from the server itself
    serverX = startX;
    serverY = startY;

    spritePath = nullptr; // null pointer to sprite path

    // determine the spritesheet to be used depending on the id retrieved through playerID param
    if (id == 1)
    {
        spritePath = "../assets/sprites/player1spritesheet.png"; // Id 1 should equal to player1spritesheet
    }

    else if (id == 2)
    {
        spritePath = "../assets/sprites/player2spritesheet.png"; // Id 2 should equal to player2spritesheet
    }
    // can add an else statement with a default sprite at some point in the future


// CREATION OF SPRITE HAPPENS WITHIN THE CONSTRUCTOR ITSELF TO KEEP LOGIC CONTAINED
// surface for the player's sprite
    SDL_Surface* surface = IMG_Load(spritePath); // load spritepath
    if (!surface) {
        SDL_Log("Failed to load sprite: %s", IMG_GetError());
    }

    // create texture from surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // sprite frames are 64x64 for player
    srcRect = { 0, 0, 64, 64 };
    destRect = { spawnPosX, spawnPosY, 64, 64 };

    x = (float)spawnPosX; // set x and y positions to the spawnX and Y
    y = (float)spawnPosY;
}
void PlayerCharacter::handleRemoteAnimations(int oldX, int oldY, float dt)
{
    if (dt <= 0.0001f) return; // ignore if the time is too less, otherwise sometime it appears as if the movement of the remote char is a bit erratic though it doesnt happen very often

    // difference in x and y represent dx,dy. calculcated by subtracting the player's current x and y from their old x and y values
    float dx = x - oldX;
    float dy = y - oldY;

    // velocity in each direction x,y calculcated by subtracting the obstained difference by time
    velocityX = dx / dt;
    velocityY = dy / dt;

    const float smoothingAlpha = 0.35f; // this is the "alpha" for the lerp method whjich will be called

    // using hte lerp function to interpolate and smooth the movement and calculate the SMOOTHED velocity; wihtout this the movement seem snappy 
    smoothedVelocityX = lerp(smoothedVelocityX, velocityX, smoothingAlpha);
    smoothedVelocityY = lerp(smoothedVelocityY, velocityY, smoothingAlpha);



    // final velocity needs to be calculated
    // // use pythagoras formula for obtaining actual magnitude of the velocity of remote char
    float finalVelocity = sqrt(smoothedVelocityX * smoothedVelocityX + smoothedVelocityY * smoothedVelocityY);


    const float movementThreshold = 0.05f; // MOVEMENT THRESHOLD IS FOR DETERMINING THE THRESHOLD FOR DETERMINING MOVEMENT
    isMoving = finalVelocity >= movementThreshold; // IS MOVING SHOULD ONLY BE TRUE IF THE SPEED IS GREATER THAN MOVEMENT THRESHOLD
    // OTHERWISE IT JUST LOOKS LIKE ALL THE ANIMS ARE PLAYING IN PLACE CONSTANTLY EVEN IF THE CHAR DOESNT MOVE


    // idle state when not moving
    if (!isMoving)
    {
        currentState = AnimationState::Idle;
        return;
    }


    AnimationState newState; // new state animation state for determining the new state the rmote character switches to when moving

    // ABS REFERS TO ABSOLUTE VALUE
    // switch between walking right and left depending on whether or not X vel is greater than Y vel

    // this should be claculcated using smoothing velocity and not the actual final velocity because it could cause jitters in movement otherwise as animations could "snap" or jitter based on actual velocity because of rapid changes
    // there is still some jitter and change in directions sometimes still does not reflect the correct animations so this should be researched further to find a better solution
    if (std::abs(smoothedVelocityX) > std::abs(smoothedVelocityY))
    {
        newState = (smoothedVelocityX > 0) ? AnimationState::WalkingRight:AnimationState::WalkingLeft;
    }
    // switch between walking up or down depending on whether smooth Y vel is greater than 0 or not
    // absolute values should not be used in this case because we need to know the + or - of smooth vel to determine if char is walking up or down
    else
    {
        newState = (smoothedVelocityY > 0) ? AnimationState::WalkingDown:AnimationState::WalkingUp;
    }

    // last movement state should be compared against the new state of the char so that a smooth transition can be made between changes in states without jitter
    // without this the transition between states isnt smooth
    if (newState != lastRemotePlayerState)
    {
        stateLockTimer += dt; // add time to thestatelocktimer 
        if (stateLockTimer >= stateLockTime)
        { // only when the timer is greater than the determined lock time change the state of the remote character
            currentState = newState; // set current anim state  to the new state
            lastRemotePlayerState = newState; // set last movement state to the new state
            stateLockTimer = 0.0f; // reset the timer
        }
    }
    else 
    { // if new state is last remote state then simply set current state to new state and also reset state lock timer
        currentState = newState;
        stateLockTimer = 0.0f;
    }
}

/* OLD METHOD
void PlayerCharacter::handleRemoteAnimations(int oldX, int oldY, float dt)
{
// calculate dx and dy
    int dx = x - oldX;
    int dy = y - oldY;


    // if theres only a small change in dx,dy then set the difference to 0 as the chance is too small to be considered
    if (abs(dx) < 2) dx = 0;
    if (abs(dy) < 2) dy = 0;

    // consider character moved if there is a change in dx or dy
    bool movedThisFrame = (dx != 0 || dy != 0);

    // if char moved then set the movement timer else reduce timer each frame
    if (movedThisFrame) {
     movementTimer = movementTime;
    }
       
    else
    {
    movementTimer -= dt;
    }

    isMoving = movementTimer > 0; // isMoving is true when movementTimer is greater than 0

    // idle if not moving
    if (!isMoving) {
        currentState = AnimationState::Idle;
        return;
    }

    // reduce state locke timer
    stateLockTimer -= dt;
    if (stateLockTimer < 0) stateLockTimer = 0; // set to 0 if less than 0


    // if moved and timer is 0 then determine the new state of the player bsaed on changes in dx and dy
    if (movedThisFrame && dirLockTimer == 0.0f)
    {
        AnimationState newState;

        if (abs(dx) > abs(dy)) {
         newState = (dx > 0) ? AnimationState::WalkingRight:AnimationState::WalkingLeft;
        }
        else {
        newState = (dy > 0) ? AnimationState::WalkingDown:AnimationState::WalkingUp;
        }
   
        lastRemotePlayerState = newState;
        stateLockTimer = stateLockTime;

    }

    currentState = lastRemotePlayerState;


}

*/

//
void PlayerCharacter::handleAnimations(const Uint8* keyboardState)
{
    // 1 represents player1 and 2 represents player2 hence up1 = player1's up and up2 represents player2's up
    // both players will use different keybinds
    bool up1 = keyboardState[SDL_SCANCODE_W];
    bool up2 = keyboardState[SDL_SCANCODE_I];

    bool down1 = keyboardState[SDL_SCANCODE_S];
    bool down2 = keyboardState[SDL_SCANCODE_K];

    bool left1 = keyboardState[SDL_SCANCODE_A];
    bool left2 = keyboardState[SDL_SCANCODE_J];

    bool right1 = keyboardState[SDL_SCANCODE_D];
    bool right2 = keyboardState[SDL_SCANCODE_L];

    // isMoving is true if any of the up,down,left or right keys were pressed
    isMoving = up1 || up2 || down1 || down2 || left1 || left2 || right1 || right2;


    // if character is not moving then play idle animation
    if (!isMoving)
    {
        currentState = AnimationState::Idle;
        return;
    }

    // SET APPROPRIATE ANIMATION STATES FOR EACH ID DEPPENDING ON THEIR MOVEMENT
    if (id == 1)
    {

        if (up1)
        {
            // play walking up animation
            currentState = AnimationState::WalkingUp;
        }

        if (down1)
        {
            // play walking down animation
            currentState = AnimationState::WalkingDown;
        }

        if (right1)
        {
            // play walking right animation
            currentState = AnimationState::WalkingRight;
        }

        if (left1)
        {
            // play walking left animation
            currentState = AnimationState::WalkingLeft;
        }

    }

    if (id == 2)
    {

        if (up2)
        {
            // play walking up animation
            currentState = AnimationState::WalkingUp;
        }

        if (down2)
        {
            // play walking down animation
            currentState = AnimationState::WalkingDown;
        }

        if (right2)
        {
            // play walking right animation
            currentState = AnimationState::WalkingRight;
        }

        if (left2)
        {
            // play walking left animation
            currentState = AnimationState::WalkingLeft;
        }

    }
    /* OLD CODE FOR DETERMINING ANIMS
    if (id == 1) {

        if (keyboardState[SDL_SCANCODE_W])
        {

            isMoving = true;

            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingUp;
            }


        }
        if (keyboardState[SDL_SCANCODE_S])
        {

            isMoving = true;

            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingDown;
            }
        }

        if (keyboardState[SDL_SCANCODE_A]) {
             direction = -1;
            isMoving = true;

            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingLeft;
            }

        }
        if (keyboardState[SDL_SCANCODE_D]) {
            direction = 1;
            isMoving = true;


            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingRight;
            }

        }


    }
    else if (id == 2) {
        if (keyboardState[SDL_SCANCODE_I])
        {

            isMoving = true;
            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingUp;
            }
        }
        if (keyboardState[SDL_SCANCODE_K])
        {

            isMoving = true;

            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingDown;
            }

        }
        if (keyboardState[SDL_SCANCODE_J])
        {

            direction = -1;
            isMoving = true;


            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingLeft;
            }
        }
        if (keyboardState[SDL_SCANCODE_L]) {

            direction = 1;
            isMoving = true;

            if (isMoving) // confirm if character is moving before playing the animation
            {
                currentState = AnimationState::WalkingRight;
            }
        }

    }
    */


    // Apply movement (debug only for testing if the anims actually work within in the client code
   // x += moveX * speed / 60.0f;
  //  y += moveY * speed / 60.0f;
}


void PlayerCharacter::updateAnimations(float dt)
{
    // set destination rect values
    destRect.x = (int)x;
    destRect.y = (int)y;

    // If character is not moving then set to idle animation state and play idle frame
    if (!isMoving) {
        currentState = AnimationState::Idle;
    }

    Uint32 currentTime = SDL_GetTicks(); // ge thte current time

    // use the correct frames for the anims
    switch (currentState) {
    case AnimationState::Idle:
        currentFrame = 1; // idle is frame 1
        break;

    case AnimationState::WalkingDown:
        if (currentTime > lastFrameTime + frameTime) {
            currentFrame++;
            if (currentFrame < 0 || currentFrame > 2) currentFrame = 0; /// walking down is frames 0-2
            lastFrameTime = currentTime;
        }
        break;

    case AnimationState::WalkingUp:
        if (currentTime > lastFrameTime + frameTime) {
            currentFrame++;
            if (currentFrame < 3 || currentFrame > 5) currentFrame = 3; // walking up is 3-5
            lastFrameTime = currentTime;
        }
        break;

    case AnimationState::WalkingLeft:
        if (currentTime > lastFrameTime + frameTime) {
            currentFrame++;
            if (currentFrame < 6 || currentFrame > 8) currentFrame = 6; // frames 6-8 same as walking right
            lastFrameTime = currentTime;
        }
        flip = SDL_FLIP_HORIZONTAL;  // FLIP MUST BE SET TO HORIZONTAL FOR LEFT WALK
        break;



    case AnimationState::WalkingRight:
        if (currentTime > lastFrameTime + frameTime) {
            currentFrame++;
            if (currentFrame < 6 || currentFrame > 8) currentFrame = 6; // frames 6-8
            lastFrameTime = currentTime;
        }
        flip = SDL_FLIP_NONE;  // NO FLIP FOR WALKING RIGHT
        break;
    }

    // update the source rect to show the correct frame
    srcRect.x = currentFrame * frameWidth;
    srcRect.y = 0;
    srcRect.w = frameWidth;
    srcRect.h = frameHeight;
}

void PlayerCharacter::setPos(int newX, int newY)
{
    x = (float)newX;
    y = (float)newY;

    destRect.x = newX;
    destRect.y = newY;

    /*
    if (id == 1) {
        destRect.x = game_data.playerX;
        destRect.y = game_data.playerY;

    }
    else if (id == 2) {
        destRect.x = game_data.player2X;
        destRect.y = game_data.player2Y;
    }
    */

}




void PlayerCharacter::render(SDL_Renderer* renderer)
{
    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0, nullptr, flip); // FLIP ADDED TO MAKE SURE THE CORRECT FRAMES ARE FLIPPED DURING THE CORREFCT ANIMS
}

void PlayerCharacter::setAuthoritativePositions(int rawX, int rawY) {
    serverX = rawX; // set rawX and rawY (actual X and Y position of player) to be equal to the server X and Y positions
    serverY = rawY;

}


//                       ============================================================          PROJECTILE CLASS METHODS         ============================================================
                                                                                    // Defining the methods of the Projectile Class

Projectile::Projectile(SDL_Renderer* renderer, int startX, int startY)
{
    // set x y to the start positions obbtained from the parameters
    x = startX;
    y = startY;

    // no velocity initially
    velocityX = 0.0f;
    velocityY = 0.0f;

    spritePath = "../assets/sprites/projectile.png"; // set sprite path

    // create surface, load sprite, create texture from surface
    SDL_Surface* surface = IMG_Load(spritePath);
    if (!surface) {
        SDL_Log("Failed to load sprite: %s", IMG_GetError());
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);


    // set souce rect 16x16 as thats the sprite size
    srcRect = { 0, 0, 16, 16 };
    destRect = { startX, startY, 16, 16 }; // set destination rect

}


// ONLY USE FIRE AT TARGET IF DECIDED TO IMPLEMENT CLIENT PREDICTION IF ENOUGH TIME IS LEFT. ELSE LET SERVER AUTHORITATIVELY AND SOLELY DO THE CALCULATIONS AND DO NOT USE THIS TO CALCULATE PROJECTILE FIRING ON CLIENTS
void Projectile::fireAtTarget(float startX, float startY, float targetX, float targetY)
{
    // if the projectile isnt active then set it to active
    if (!isActive)
    {
        isActive = true;

        //set x and y to obtained start positions (player's positions)
        // could add a bit of distance from the start position both on client and server to make sure the player doesnt collide with the projecitle (as it currently does, player gets a bit pushed by it as well)
        x = startX;
        y = startY;
        speed = 600; // set speed to thye same as the speed in server code, ensuring both client and server make the same calculations

        // calculcate differences in x and y
        float dx = targetX - x;
        float dy = targetY - y;

        // SOH CAH TOA to calculcate angle velocity x and y
        angle = atan2(dy, dx);
        velocityX = cos(angle) * speed;
        velocityY = sin(angle) * speed;
    }
}


void Projectile::update(float dt)
{
    // calculcate the movement
    x += velocityX * dt;
    y += velocityY * dt;

    // update destination rect so sprite will be rendered at the correct positions
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);

}


void Projectile::render(SDL_Renderer* renderer)
{
    SDL_RenderCopy(renderer, texture, &srcRect, &destRect); // render
}
