/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "Entity.h"

Entity::Entity() :
    mPosition{ 0.0f, 0.0f }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ DEFAULT_SIZE, DEFAULT_SIZE },
    mColliderDimensions{ DEFAULT_SIZE, DEFAULT_SIZE },
    mTexture{}, mTextureType{ SINGLE }, mSpriteSheetDimensions{},
    mAnimationAtlas{}, mAnimationIndices{}, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f }, mEntityType{ NONE }, mAIType{ WANDERER },
    mAIState{ IDLE } {}

Entity::Entity(
    Vector2     position,
    Vector2     scale,
    const char *textureFilepath,
    EntityType  entityType
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ SINGLE },
    mSpriteSheetDimensions{}, mAnimationAtlas{}, mAnimationIndices{},
    mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f }, mEntityType{ entityType },
    mAIType{ WANDERER }, mAIState{ IDLE } {}

Entity::Entity(
    Vector2               position,
    Vector2               scale,
    const char           *textureFilepath,
    TextureType           textureType,
    Vector2               spriteSheetDimensions,
    const AnimationAtlas &animationAtlas,
    EntityType            entityType
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ textureType },
    mSpriteSheetDimensions{ spriteSheetDimensions },
    mAnimationAtlas{ animationAtlas }, mAnimationIndices{},
    mFrameSpeed{ DEFAULT_FRAME_SPEED }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f }, mEntityType{ entityType }, mAIType{ WANDERER },
    mAIState{ IDLE }
{
    mAnimationIndices = framesFor(ANIM_IDLE);
}

Entity::~Entity() { UnloadTexture(mTexture); }

/*
    ----------- ENTITY-vs-ENTITY COLLISIONS (from lecture) -----------
*/
void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        if (!isColliding(collidableEntity)) continue;

        float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
        float yOverlap  = fabs(
            yDistance - (mColliderDimensions.y / 2.0f) -
            (collidableEntity->mColliderDimensions.y / 2.0f)
        );

        if (mVelocity.y > 0)
        {
            mPosition.y       -= yOverlap;
            mVelocity.y        = 0;
            mIsCollidingBottom = true;
        }
        else if (mVelocity.y < 0)
        {
            mPosition.y    += yOverlap;
            mVelocity.y     = 0;
            mIsCollidingTop = true;
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        if (!isColliding(collidableEntity)) continue;

        float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
        float yOverlap  = fabs(
            yDistance - (mColliderDimensions.y / 2.0f) -
            (collidableEntity->mColliderDimensions.y / 2.0f)
        );

        // Standing on something is not a wall hit.
        if (yOverlap < Y_COLLISION_THRESHOLD) continue;

        float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
        float xOverlap  = fabs(
            xDistance - (mColliderDimensions.x / 2.0f) -
            (collidableEntity->mColliderDimensions.x / 2.0f)
        );

        if (mVelocity.x > 0)
        {
            mPosition.x      -= xOverlap;
            mVelocity.x       = 0;
            mIsCollidingRight = true;
        }
        else if (mVelocity.x < 0)
        {
            mPosition.x     += xOverlap;
            mVelocity.x      = 0;
            mIsCollidingLeft = true;
        }
    }
}

/*
    ----------- ENTITY-vs-MAP COLLISIONS (from lecture) -----------
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

    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe,   &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topRightProbe,  &xOverlap, &yOverlap)) &&
        mVelocity.y < 0.0f)
    {
        mPosition.y    += yOverlap; // push down
        mVelocity.y     = 0.0f;
        mIsCollidingTop = true;
    }

    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe,   &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomRightProbe,  &xOverlap, &yOverlap)) &&
        mVelocity.y > 0.0f)
    {
        mPosition.y       -= yOverlap; // push up
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

    if ((map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightTopProbe,    &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x > 0.0f && yOverlap >= Y_COLLISION_THRESHOLD)
    {
        mPosition.x      -= xOverlap * 1.01f; // push left
        mVelocity.x       = 0.0f;
        mIsCollidingRight = true;
    }

    if ((map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftTopProbe,    &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x < 0.0f && yOverlap >= Y_COLLISION_THRESHOLD)
    {
        mPosition.x     += xOverlap * 1.01f; // push right
        mVelocity.x      = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(const Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) -
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) -
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

Entity *Entity::findCollision(const std::vector<Entity *> &others) const
{
    if (!isActive()) return nullptr;

    for (Entity *other : others)
        if (other != nullptr && isColliding(other)) return other;

    return nullptr;
}

/*
    ----------- ANIMATION -----------
*/
AnimState Entity::resolveAnimState() const
{
    bool isAffectedByGravity = mAcceleration.y != 0.0f;

    if (isAffectedByGravity && !mIsCollidingBottom)
        return mVelocity.y < 0.0f ? ANIM_JUMP : ANIM_FALL;

    return mMovement.x != 0.0f ? ANIM_RUN : ANIM_IDLE;
}

const std::vector<int> &Entity::framesFor(AnimState state) const
{
    static const std::vector<int> NO_FRAMES;

    AnimationAtlas::const_iterator match = mAnimationAtlas.find(state);
    if (match != mAnimationAtlas.end()) return match->second;

    match = mAnimationAtlas.find(ANIM_RUN);
    if (match != mAnimationAtlas.end()) return match->second;

    return mAnimationAtlas.empty() ? NO_FRAMES : mAnimationAtlas.begin()->second;
}

void Entity::animate(float deltaTime)
{
    AnimState state = resolveAnimState();

    if (state != mAnimState)
    {
        mAnimState         = state;
        mCurrentFrameIndex = 0;
        mAnimationTime     = 0.0f;
    }

    mAnimationIndices = framesFor(mAnimState);
    if (mAnimationIndices.empty() || mFrameSpeed <= 0) return;

    mAnimationTime += deltaTime;
    float secondsPerFrame = 1.0f / mFrameSpeed;

    if (mAnimationTime >= secondsPerFrame)
    {
        mAnimationTime = 0.0f;
        mCurrentFrameIndex++;
    }

    mCurrentFrameIndex %= mAnimationIndices.size();
}

void Entity::displayCollider()
{
    DrawRectangleLines(
        mPosition.x - mColliderDimensions.x / 2.0f,
        mPosition.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y,
        GREEN
    );
}

/*
    ----------- AI -----------
*/

// Walks in the current direction and turns around at walls or at the edges
// of the patrol range (used to keep walkers on their platform).
void Entity::patrolHorizontally()
{
    bool isBlockedLeft  = mIsCollidingLeft  ||
        (hasPatrolBounds() && mPosition.x <= mPatrolMinX);
    bool isBlockedRight = mIsCollidingRight ||
        (hasPatrolBounds() && mPosition.x >= mPatrolMaxX);

    if (mDirection == LEFT) { if (isBlockedLeft)  moveRight(); else moveLeft();  }
    else                    { if (isBlockedRight) moveLeft();  else moveRight(); }
}

void Entity::AIWander() { patrolHorizontally(); }

void Entity::AIFollow(Entity *target)
{
    if (target == nullptr) return;

    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < FOLLOW_RANGE)
            mAIState = WALKING;
        break;

    case WALKING:
    {
        bool isTargetLeft = mPosition.x > target->getPosition().x;

        if (isTargetLeft) moveLeft();
        else              moveRight();

        // Stop at the edge of our territory instead of walking into a pit.
        bool isAtLeftEdge  = hasPatrolBounds() && mPosition.x <= mPatrolMinX;
        bool isAtRightEdge = hasPatrolBounds() && mPosition.x >= mPatrolMaxX;
        if ((isTargetLeft && isAtLeftEdge) || (!isTargetLeft && isAtRightEdge))
            mMovement.x = 0.0f;
        break;
    }

    default:
        break;
    }
}

//flyers ignore gravity: they patrol sideways while bobbing on a sine wave.
void Entity::AIFly(float deltaTime)
{
    mAITimer += deltaTime;
    patrolHorizontally();

    mVelocity.y = FLYER_BOB_AMPLITUDE * FLYER_BOB_FREQUENCY *
        cosf(mAITimer * FLYER_BOB_FREQUENCY);
}

//walk toward the player -> stop and flash (telegraph) -> charge until a wall.
void Entity::AIBoss(Entity *target, float deltaTime)
{
    if (target == nullptr) return;

    mAITimer += deltaTime;
    bool isTargetLeft = mPosition.x > target->getPosition().x;

    switch (mAIState)
    {
    case TELEGRAPHING:
        mMovement.x = 0.0f;
        if (mAITimer >= BOSS_TELEGRAPH_TIME)
        {
            mChargeDirection = isTargetLeft ? -1.0f : 1.0f;
            setAIState(CHARGING);
        }
        break;

    case CHARGING:
        if (mChargeDirection < 0.0f) moveLeft();
        else                         moveRight();
        mMovement.x = mChargeDirection * BOSS_CHARGE_MULTIPLIER;

        if (mIsCollidingLeft || mIsCollidingRight ||
            mAITimer >= BOSS_MAX_CHARGE_TIME)
            setAIState(WALKING);
        break;

    default: // IDLE / WALKING
        if (isTargetLeft) moveLeft();
        else              moveRight();

        if (mAITimer >= BOSS_WALK_TIME) setAIState(TELEGRAPHING);
        break;
    }
}

void Entity::AIActivate(Entity *target, float deltaTime)
{
    switch (mAIType)
    {
    case WANDERER: AIWander();                  break;
    case FOLLOWER: AIFollow(target);            break;
    case FLYER:    AIFly(deltaTime);            break;
    case BOSS:     AIBoss(target, deltaTime);   break;
    default:                                    break;
    }
}

/*
    ----------- UPDATE & RENDER -----------
*/
void Entity::update(
    float   deltaTime,
    Entity *player,
    Map    *map,
    Entity *collidableEntities,
    int     collisionCheckCount
)
{
    if (mEntityStatus == INACTIVE) return;

    if (mInvincibleTimer > 0.0f) mInvincibleTimer -= deltaTime;

    // AI reads last frame's collision flags, so it runs before the reset.
    if (mEntityType == NPC) AIActivate(player, deltaTime);

    resetColliderFlags();

    mVelocity.x  = mMovement.x * mSpeed;
    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    if (mIsJumping)
    {
        mIsJumping   = false;
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);

    if (mTextureType == ATLAS) animate(deltaTime);
}

void Entity::render()
{
    if (mEntityStatus == INACTIVE) return;

    // Blink while invincible: skip every other BLINK_PERIOD.
    if (isInvincible() &&
        fmodf(mInvincibleTimer, BLINK_PERIOD) < BLINK_PERIOD / 2.0f)
        return;

    Rectangle textureArea;

    if (mTextureType == ATLAS)
    {
        if (mAnimationIndices.empty()) return;

        textureArea = getUVRectangle(
            &mTexture,
            mAnimationIndices[mCurrentFrameIndex],
            mSpriteSheetDimensions.x,
            mSpriteSheetDimensions.y
        );
    }
    else
    {
        textureArea = {
            0.0f, 0.0f,
            static_cast<float>(mTexture.width),
            static_cast<float>(mTexture.height)
        };
    }

    // A negative source width mirrors the sprite horizontally.
    if (mFlipsWhenFacingLeft && mDirection == LEFT)
        textureArea.width = -textureArea.width;

    Rectangle destinationArea = { mPosition.x, mPosition.y, mScale.x, mScale.y };
    Vector2   originOffset    = { mScale.x / 2.0f, mScale.y / 2.0f };

    Color tint = (mAIType == BOSS && mAIState == TELEGRAPHING) ? RED : WHITE;

    DrawTexturePro(
        mTexture, textureArea, destinationArea, originOffset, mAngle, tint
    );
}
