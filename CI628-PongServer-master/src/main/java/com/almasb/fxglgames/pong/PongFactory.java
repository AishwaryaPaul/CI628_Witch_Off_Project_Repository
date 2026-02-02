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

import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.EntityFactory;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.entity.Spawns;
import com.almasb.fxgl.entity.components.CollidableComponent;
import com.almasb.fxgl.particle.ParticleComponent;
import com.almasb.fxgl.particle.ParticleEmitter;
import com.almasb.fxgl.particle.ParticleEmitters;
import com.almasb.fxgl.physics.BoundingShape;
import com.almasb.fxgl.physics.HitBox;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.physics.box2d.dynamics.BodyType;
import com.almasb.fxgl.physics.box2d.dynamics.FixtureDef;
import javafx.beans.binding.Bindings;
import javafx.geometry.Point2D;
import javafx.scene.effect.BlendMode;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;

import static com.almasb.fxgl.dsl.FXGL.*;
import static com.almasb.fxglgames.pong.PongApp.TILE_SIZE; // THIS IMPORT STATEMENT IS NEEDED TO RETRIEVE THE TILE_SIZE VALUE FROM THE
// MAIN PONG APP APPLICATION FILE. THIS IS BETTER TO DO THAN DECLARE TILE_SIZE TWICE
// IN TWO DIFFERENT FILES

/**
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class PongFactory implements EntityFactory {

    // OLD CODE FROM THE PONG GAME IS STILL KEPT HERE SO IT CAN BE REVIEWED SOMETIMES FOR REFERENCE/LEARNING
    @Spawns("ball")
    public Entity newBall(SpawnData data) {
        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.DYNAMIC);
        physics.setFixtureDef(new FixtureDef().density(0.3f).restitution(1.0f));
        physics.setOnPhysicsInitialized(() -> physics.setLinearVelocity(5 * 60, -5 * 60));

        var endGame = getip("player1score").isEqualTo(10).or(getip("player2score").isEqualTo(10));

        ParticleEmitter emitter = ParticleEmitters.newFireEmitter();
        emitter.startColorProperty().bind(
                Bindings.when(endGame)
                        .then(Color.LIGHTYELLOW)
                        .otherwise(Color.LIGHTYELLOW)
        );

        emitter.endColorProperty().bind(
                Bindings.when(endGame)
                        .then(Color.RED)
                        .otherwise(Color.LIGHTBLUE)
        );

        emitter.setBlendMode(BlendMode.SRC_OVER);
        emitter.setSize(5, 10);
        emitter.setEmissionRate(1);

        return entityBuilder(data)
                .type(EntityType.BALL)
                .bbox(new HitBox(BoundingShape.circle(5)))
                .with(physics)
                .with(new CollidableComponent(true))
                .with(new ParticleComponent(emitter))
                .with(new BallComponent())
                .build();
    }

    @Spawns("bat")
    public Entity newBat(SpawnData data) {
        boolean isPlayer = data.get("isPlayer");

        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.KINEMATIC);

        return entityBuilder(data)
                .type(isPlayer ? EntityType.PLAYER_BAT : EntityType.ENEMY_BAT)
                .viewWithBBox(new Rectangle(20, 60, Color.LIGHTGRAY))
                .with(new CollidableComponent(true))
                .with(physics)
                .with(new BatComponent())
                .build();
    }

    // === PLAYER FACTORY
    @Spawns("player")
    public Entity player(SpawnData data) {
        System.out.println("SPAWN METHOD CALLED for playerId=" + data.get("playerId"));

        // get player id from the data
        int playerId = data.get("playerId");

        // add physics comp
        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.DYNAMIC); // player will be a dynamic body

        return entityBuilder(data)
                .type(EntityType.PLAYER)
                .bbox(new HitBox(BoundingShape.box(64, 64))) // BOUNDING BOX: THIS IS FOR COLLISIONS. View with bbox can be used for debug
                .with(physics)
                .with(new PlayerCharacterComponent(playerId))
                .with(new CollidableComponent(true)) // COLLIDABLE COMP MUST BE TRUE TO ALLOW FOR COLLISION DETECTION
                .build();
    }


    // === WALL FACTORY
    @Spawns("wall")
    public Entity newWall(SpawnData data) {

        PhysicsComponent physics = new PhysicsComponent(); // physics comp
        physics.setBodyType(BodyType.STATIC); // static body type for wall since it should just block the players movement
        return entityBuilder(data)
                .bbox(new HitBox(BoundingShape.box(TILE_SIZE, TILE_SIZE))) // MUST HAVE A HITBOX WITH CORRECT TILE SIZE TO DETECT COLLISIONS
                .type(EntityType.WALL)
                .with(physics)
                .with(new CollidableComponent(true)) // MUST BE COLLIDABLE TO BLOCK PLAYERS FROM WALKING THROUGH THE WALLS
                .build();
    }


    // === PROJECTILE FACTORY
    @Spawns("newProjectile")
    public Entity newProjectile(SpawnData data) {

        double mouseX = data.get("mouseX");
        double mouseY = data.get("mouseY");

        // get owner id and projectile damage values from the data
        int ownerId = data.get("ownerId");
        int projectileDamage = data.get("projectileDamageAmount");
        int projectileId = data.get("projectileId");

        double projectileStartX = data.get("startX");
        double projectileStartY = data.get("startY");


        // PROJECTILES WILL USE A KINEMATIC PHYSICS BODY
        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.KINEMATIC);

        // these fixture def settings are meant to try to reduce the push effects of the projectiles
        physics.setFixtureDef(new FixtureDef().density(0.3f).restitution(1.0f));


        // store entity as projectile entity so that we can later do the physics calculations
        Entity projectile = entityBuilder(data)
                .type(EntityType.PROJECTILE)
                .bbox(new HitBox(BoundingShape.box(16, 16))) // 16X16 HITBOX FOR PROJECTILES
                .with(physics)
                .with(new CollidableComponent(true))
                .with(new ProjectileComponent(ownerId,projectileDamage,projectileId))

                .build();

        // ----- ADD LINEAR VELOCITY TO MAKE PROJECTILE MOVE TO MOUSEX, MOUSEY
        // without using set on physics intialised it logs a bug to the server console stating physics not initialised yet
        physics.setOnPhysicsInitialized(() -> {

            // SIMILAR METHOD TO FIREATTARGET FROM CLIENT CODE SHOULD BE USED HERE

            // calculcate dx and dy, difference between target X and Y and start X and Y
            double dx = mouseX - projectileStartX;
            double dy = mouseY - projectileStartY;

            // magnitude of the vector must be calculated using pythagoras formula
            double magnitude = Math.sqrt(dx * dx + dy * dy);
            if (magnitude != 0) {  // MUST NOT DIVIDEBY 0
                dx /= magnitude;  // this is the NORMALIZED X direction
                dy /= magnitude;  // this is the NORMALIZED Y direction
            }

            // speed of the projectile SHOULD BE SAME AS IN CLIENT CODE
            double speed = 600;

            // calculate the velocity -> velocity = distance in one direction multiplied by speed
            double velocityX = dx * speed;
            double velocityY = dy * speed;
            // SHOULD VELOCITY X AND Y BE MOVED INTO PROJECTILE COMPONENT CLASS ALSO?

            // set linear velocity
            physics.setLinearVelocity(new Point2D(velocityX, velocityY));
        });


        return projectile;

    }





}
