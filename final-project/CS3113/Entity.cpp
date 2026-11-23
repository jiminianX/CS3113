/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Entity.h"

Entity::Entity()
    : mPosition{ 0.0f, 0.0f }, mMovement{ 0.0f, 0.0f },
      mVelocity{ 0.0f, 0.0f }, mKnockback{ 0.0f, 0.0f },
      mScale{ DEFAULT_SIZE, DEFAULT_SIZE },
      mColliderDimensions{ DEFAULT_SIZE, DEFAULT_SIZE },
      mTexture{}, mTextureType{ SINGLE }, mSpriteSheetDimensions{},
      mAnimationAtlas{}, mAnimationIndices{}, mDirection{ DOWN },
      mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f },
      mEntityType{ ORB }
{}

Entity::Entity(
    Vector2     position,
    Vector2     scale,
    const char *textureFilepath,
    EntityType  entityType
)
    : mPosition{ position }, mMovement{ 0.0f, 0.0f },
      mVelocity{ 0.0f, 0.0f }, mKnockback{ 0.0f, 0.0f }, mScale{ scale },
      mColliderDimensions{ scale }, mTexture{ LoadTexture(textureFilepath) },
      mTextureType{ SINGLE }, mSpriteSheetDimensions{}, mAnimationAtlas{},
      mAnimationIndices{}, mDirection{ DOWN }, mFrameSpeed{ 0 },
      mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f }, mEntityType{ entityType }
{}

Entity::Entity(
    Vector2                               position,
    Vector2                               scale,
    const char                           *textureFilepath,
    TextureType                           textureType,
    Vector2                               spriteSheetDimensions,
    std::map<Direction, std::vector<int>> animationAtlas,
    EntityType                            entityType
)
    : mPosition{ position }, mMovement{ 0.0f, 0.0f },
      mVelocity{ 0.0f, 0.0f }, mKnockback{ 0.0f, 0.0f }, mScale{ scale },
      mColliderDimensions{ scale }, mTexture{ LoadTexture(textureFilepath) },
      mTextureType{ textureType },
      mSpriteSheetDimensions{ spriteSheetDimensions },
      mAnimationAtlas{ animationAtlas }, mAnimationIndices{ animationAtlas.at(DOWN) },
      mDirection{ DOWN }, mFrameSpeed{ DEFAULT_FRAME_SPEED },
      mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f }, mEntityType{ entityType }
{}

Entity::~Entity()
{
    UnloadTexture(mTexture);
}

/*
    ----------- ENTITY-vs-MAP COLLISIONS -----------
    Same probes as the lecture, but top-down: a probe only counts as a hit if
    it is clearly inside the tile on the other axis too, so walking along a
    wall doesn't snag on it.
*/

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    float halfWidth  = mColliderDimensions.x / 2.0f;
    float halfHeight = mColliderDimensions.y / 2.0f;

    Vector2 topCentreProbe    = { mPosition.x,             mPosition.y - halfHeight };
    Vector2 topLeftProbe      = { mPosition.x - halfWidth, mPosition.y - halfHeight };
    Vector2 topRightProbe     = { mPosition.x + halfWidth, mPosition.y - halfHeight };

    Vector2 bottomCentreProbe = { mPosition.x,             mPosition.y + halfHeight };
    Vector2 bottomLeftProbe   = { mPosition.x - halfWidth, mPosition.y + halfHeight };
    Vector2 bottomRightProbe  = { mPosition.x + halfWidth, mPosition.y + halfHeight };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (moving up)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe,   &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topRightProbe,  &xOverlap, &yOverlap)) &&
        mVelocity.y < 0.0f && xOverlap >= COLLISION_THRESHOLD)
    {
        mPosition.y    += yOverlap * 1.01f; // push down
        mVelocity.y     = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (moving down)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe,   &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomRightProbe,  &xOverlap, &yOverlap)) &&
        mVelocity.y > 0.0f && xOverlap >= COLLISION_THRESHOLD)
    {
        mPosition.y       -= yOverlap * 1.01f; // push up
        mVelocity.y        = 0.0f;
        mIsCollidingBottom = true;
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    float halfWidth  = mColliderDimensions.x / 2.0f;
    float halfHeight = mColliderDimensions.y / 2.0f;

    Vector2 leftCentreProbe  = { mPosition.x - halfWidth, mPosition.y              };
    Vector2 leftTopProbe     = { mPosition.x - halfWidth, mPosition.y - halfHeight };
    Vector2 leftBottomProbe  = { mPosition.x - halfWidth, mPosition.y + halfHeight };

    Vector2 rightCentreProbe = { mPosition.x + halfWidth, mPosition.y              };
    Vector2 rightTopProbe    = { mPosition.x + halfWidth, mPosition.y - halfHeight };
    Vector2 rightBottomProbe = { mPosition.x + halfWidth, mPosition.y + halfHeight };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ON RIGHT (moving right)
    if ((map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightTopProbe,    &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x > 0.0f && yOverlap >= COLLISION_THRESHOLD)
    {
        mPosition.x      -= xOverlap * 1.01f; // push left
        mVelocity.x       = 0.0f;
        mIsCollidingRight = true;
    }

    // COLLISION ON LEFT (moving left)
    if ((map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftTopProbe,    &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x < 0.0f && yOverlap >= COLLISION_THRESHOLD)
    {
        mPosition.x     += xOverlap * 1.01f; // push right
        mVelocity.x      = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    float xDistance =
        fabs(mPosition.x - other->getPosition().x) -
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance =
        fabs(mPosition.y - other->getPosition().y) -
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

/*
    ----------- COMBAT -----------
*/

void Entity::takeDamage(int amount)
{
    mHealth -= amount;
    if (mHealth <= 0) deactivate();
}

// Pushes the entity away from `from`; the push fades out over a few steps.
void Entity::knockBack(Vector2 from, float speed)
{
    Vector2 away = Vector2Subtract(mPosition, from);
    if (GetLength(away) == 0.0f) away = { 0.0f, 1.0f };

    Normalise(&away);
    mKnockback = Vector2Scale(away, speed);
}

/*
    ----------- ANIMATION -----------
*/

void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime        += deltaTime;
    float secondsPerFrame  = 1.0f / mFrameSpeed;

    if (mAnimationTime >= secondsPerFrame)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

/*
    ----------- AI -----------
*/

// Sets the movement towards a point, one axis at a time like the arrow keys.
void Entity::moveTowards(Vector2 point)
{
    resetMovement();

    if      (point.y < mPosition.y - ARRIVE_DISTANCE) moveUp();
    else if (point.y > mPosition.y + ARRIVE_DISTANCE) moveDown();

    if      (point.x < mPosition.x - ARRIVE_DISTANCE) moveLeft();
    else if (point.x > mPosition.x + ARRIVE_DISTANCE) moveRight();

    if (GetLength(mMovement) > 1.0f) normaliseMovement();
}

// Soldiers: patrol between two points, chase the player when they get close.
void Entity::AIWander(Entity *target)
{
    float distance = Vector2Distance(mPosition, target->getPosition());

    switch (mAIState)
    {
    case FOLLOWING:
        if (distance > GIVE_UP_RANGE)
        {
            // Start a new patrol around where the chase ended.
            Vector2 halfPatrol = Vector2Scale(Vector2Subtract(mPatrolEnd, mPatrolStart), 0.5f);
            setPatrolPoints(Vector2Subtract(mPosition, halfPatrol), Vector2Add(mPosition, halfPatrol));
            mAIState = WALKING;
            break;
        }

        moveTowards(target->getPosition());
        mMovement = Vector2Scale(mMovement, CHASE_SPEED_MULTIPLIER);
        break;

    default: // WALKING (patrolling)
    {
        if (distance < SOLDIER_SIGHT_RANGE)
        {
            mAIState = FOLLOWING;
            break;
        }

        Vector2 goal      = mIsGoingToPatrolEnd ? mPatrolEnd : mPatrolStart;
        bool    hitWall   = mIsCollidingLeft || mIsCollidingRight ||
                            mIsCollidingTop  || mIsCollidingBottom;
        bool    isAtGoal  = Vector2Distance(mPosition, goal) < ARRIVE_DISTANCE;

        if (isAtGoal || hitWall) mIsGoingToPatrolEnd = !mIsGoingToPatrolEnd;

        moveTowards(mIsGoingToPatrolEnd ? mPatrolEnd : mPatrolStart);
        break;
    }
    }
}

// Scouts: hover in place until the player comes close, then fly at them.
void Entity::AIFollow(Entity *target)
{
    float distance = Vector2Distance(mPosition, target->getPosition());

    switch (mAIState)
    {
    case IDLE:
        if (distance < SCOUT_SIGHT_RANGE) mAIState = FOLLOWING;
        break;

    case FOLLOWING:
        if (distance > GIVE_UP_RANGE)
        {
            resetMovement();
            mAIState = IDLE;
            break;
        }

        moveTowards(target->getPosition());
        break;

    default:
        break;
    }
}

void Entity::AIActivate(Entity *target)
{
    if (target == nullptr) return;

    switch (mAIType)
    {
    case WANDERER:
        AIWander(target);
        break;

    case FOLLOWER:
        AIFollow(target);
        break;

    default:
        break;
    }
}

/*
    ----------- UPDATE & RENDER -----------
*/

void Entity::update(float deltaTime, Entity *player, Map *map)
{
    if (mEntityStatus == INACTIVE) return;

    if (mInvincibleTimer > 0.0f) mInvincibleTimer -= deltaTime;
    if (mHurtTimer       > 0.0f) mHurtTimer       -= deltaTime;
    if (mPoseTimer       > 0.0f) mPoseTimer       -= deltaTime;
    mBobTimer += deltaTime;

    if (mHasLifeTimer)
    {
        mLifeTimer -= deltaTime;
        if (mLifeTimer <= 0.0f)
        {
            deactivate();
            return;
        }
    }

    // The AI looks at last step's collision flags, so it runs before the reset.
    if (mEntityType == NPC) AIActivate(player);

    resetColliderFlags();

    // Top-down: no gravity, so both axes come straight from the movement.
    mVelocity.x = mMovement.x * mSpeed + mKnockback.x;
    mVelocity.y = mMovement.y * mSpeed + mKnockback.y;
    mKnockback  = Vector2Scale(mKnockback, KNOCKBACK_FADE);

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(map);

    if (mTextureType == ATLAS)
    {
        if (GetLength(mMovement) != 0) animate(deltaTime);
        else
        {
            mAnimationIndices  = mAnimationAtlas.at(mDirection);
            mCurrentFrameIndex = 0; // standing still
        }
    }
}

void Entity::render()
{
    if (mEntityStatus == INACTIVE) return;

    // Blink while invincible.
    if (mInvincibleTimer > 0.0f &&
        fmodf(mInvincibleTimer, BLINK_PERIOD * 2.0f) < BLINK_PERIOD) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
    case SINGLE:
        textureArea = {
            // top-left corner
            0.0f, 0.0f,

            // bottom-right corner (of texture)
            static_cast<float>(mTexture.width),
            static_cast<float>(mTexture.height)
        };
        break;

    case ATLAS:
    {
        int frame = mPoseTimer > 0.0f ? mPoseFrame
                                      : mAnimationIndices[mCurrentFrameIndex];
        textureArea = getUVRectangle(
            &mTexture,
            frame,
            mSpriteSheetDimensions.x,
            mSpriteSheetDimensions.y
        );
        break;
    }

    default:
        break;
    }

    float bob = mBobHeight * sinf(mBobTimer * BOB_SPEED);

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y - bob,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    Color tint = mHurtTimer > 0.0f ? RED : WHITE;

    DrawTexturePro(
        mTexture, textureArea, destinationArea, originOffset, mAngle, tint
    );

    // displayCollider();
}

void Entity::displayCollider()
{
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,
        mPosition.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x,      // Top-left X
        colliderBox.y,      // Top-left Y
        colliderBox.width,  // Width
        colliderBox.height, // Height
        GREEN               // Color
    );
}
