package com.almasb.fxglgames.pong;
import com.almasb.fxgl.entity.component.Component;

public class ProjectileComponent extends Component {

    // private double lifeTime = 2.0; // decided to not implement lifetime for projectiles in game; projectiles despawn on collision with players or walls
    private final int ownerId; // newProjectile also needs tio store the id of the player which launched it
    private final int projectileDamageAmount; // the projectile damage: im not hardcoding the value here in case
    // i decide to add more types of spells in the future which may have different
    // damage values
    private final int projectileId; // store projectile id
    public ProjectileComponent(int ownerId, int projectileDamageAmount, int projectileId) {
        this.ownerId = ownerId; // set the owner id
        this.projectileDamageAmount = projectileDamageAmount; // set damage amount
        this.projectileId = projectileId; // set id
    }

    @Override
    public void onUpdate(double tpf) { // uypdate method for the newProjectile
        //lifeTime -= tpf; // reduce its lifetime every frame
        // if (lifeTime <= 0) { // delete the entity from the game world if the lifetime is at its end
        //   entity.removeFromWorld();
        //}
    }

    // getter method for retrieving the id of the player which launched the newProjectile
    public int getOwnerId() {
        return ownerId;
    }
    // getter method for retrieving damage amount
    public int  getProjectileDamageAmount() {
        return projectileDamageAmount;
    }

    // getter method for retrieving proejctile id
    public int getProjectileId()
    {
        return projectileId;
    }


}
