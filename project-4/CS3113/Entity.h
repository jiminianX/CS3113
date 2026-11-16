/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, UP, RIGHT, DOWN                };
enum EntityStatus { ACTIVE, INACTIVE                     };
enum EntityType   { PLAYER, NPC, GOAL, NONE              };
enum AIType       { WANDERER, FOLLOWER, FLYER, BOSS      };
enum AIState      { IDLE, WALKING, TELEGRAPHING, CHARGING };
enum AnimState    { ANIM_IDLE, ANIM_RUN, ANIM_JUMP, ANIM_FALL };

typedef std::map<AnimState, std::vector<int>> AnimationAtlas;

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;

    Vector2 mScale;
    Vector2 mColliderDimensions;

    Texture2D   mTexture;
    TextureType mTextureType;
    Vector2     mSpriteSheetDimensions; // { rows, columns }

    AnimationAtlas   mAnimationAtlas;
    std::vector<int> mAnimationIndices;
    AnimState        mAnimState  = ANIM_IDLE;
    Direction        mDirection  = RIGHT;
    int              mFrameSpeed = 0;

    int   mCurrentFrameIndex = 0;
    float mAnimationTime     = 0.0f;
    bool  mFlipsWhenFacingLeft = false;

    bool  mIsJumping    = false;
    float mJumpingPower = 0.0f;

    float mSpeed;
    float mAngle;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;
    float   mAITimer         = 0.0f;
    float   mPatrolMinX      = 0.0f;
    float   mPatrolMaxX      = 0.0f;
    float   mChargeDirection = 1.0f;

    float mInvincibleTimer = 0.0f;

    bool isColliding(const Entity *other) const;

    void checkCollisionY(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionY(Map *map);

    void checkCollisionX(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Map *map);

    void resetColliderFlags()
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    AnimState               resolveAnimState() const;
    const std::vector<int> &framesFor(AnimState state) const;
    void                    animate(float deltaTime);

    bool hasPatrolBounds() const { return mPatrolMinX < mPatrolMaxX; }
    void patrolHorizontally();

    void AIActivate(Entity *target, float deltaTime);
    void AIWander();
    void AIFollow(Entity *target);
    void AIFly(float deltaTime);
    void AIBoss(Entity *target, float deltaTime);

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr float DEFAULT_SPEED         = 200.0f;
    static constexpr int   DEFAULT_FRAME_SPEED   = 10;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

    static constexpr float FOLLOW_RANGE           = 320.0f;
    static constexpr float FLYER_BOB_AMPLITUDE    = 40.0f;
    static constexpr float FLYER_BOB_FREQUENCY    = 3.0f;
    static constexpr float BOSS_WALK_TIME         = 2.2f;
    static constexpr float BOSS_TELEGRAPH_TIME    = 0.7f;
    static constexpr float BOSS_MAX_CHARGE_TIME   = 2.5f;
    static constexpr float BOSS_CHARGE_MULTIPLIER = 3.0f;
    static constexpr float BLINK_PERIOD           = 0.16f;

    Entity();
    Entity(
        Vector2     position,
        Vector2     scale,
        const char *textureFilepath,
        EntityType  entityType
    );
    Entity(
        Vector2               position,
        Vector2               scale,
        const char           *textureFilepath,
        TextureType           textureType,
        Vector2               spriteSheetDimensions,
        const AnimationAtlas &animationAtlas,
        EntityType            entityType
    );
    ~Entity();

    // Entities own a GPU texture, so copying one would double-unload it.
    Entity(const Entity &)            = delete;
    Entity &operator=(const Entity &) = delete;

    void update(
        float   deltaTime,
        Entity *player,
        Map    *map,
        Entity *collidableEntities,
        int     collisionCheckCount
    );
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    // Player-perspective contact check: returns the first active entity in
    // `others` that overlaps this one, or nullptr if there is none.
    Entity *findCollision(const std::vector<Entity *> &others) const;

    void jump()                   { mIsJumping = true;          }
    void bounce(float power)      { mVelocity.y = -power;       }
    void activate()               { mEntityStatus = ACTIVE;     }
    void deactivate()             { mEntityStatus = INACTIVE;   }
    void startInvincibility(float seconds) { mInvincibleTimer = seconds; }
    void displayCollider();

    bool isActive()     const { return mEntityStatus == ACTIVE; }
    bool isInvincible() const { return mInvincibleTimer > 0.0f; }

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mMovement.x = -1; mDirection = LEFT;  }
    void moveRight() { mMovement.x =  1; mDirection = RIGHT; }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()           const { return mPosition;           }
    Vector2     getMovement()           const { return mMovement;           }
    Vector2     getVelocity()           const { return mVelocity;           }
    Vector2     getAcceleration()       const { return mAcceleration;       }
    Vector2     getScale()              const { return mScale;              }
    Vector2     getColliderDimensions() const { return mColliderDimensions; }
    Texture2D   getTexture()            const { return mTexture;            }
    TextureType getTextureType()        const { return mTextureType;        }
    Direction   getDirection()          const { return mDirection;          }
    float       getJumpingPower()       const { return mJumpingPower;       }
    float       getSpeed()              const { return mSpeed;              }
    EntityType  getEntityType()         const { return mEntityType;         }
    AIType      getAIType()             const { return mAIType;             }
    AIState     getAIState()            const { return mAIState;            }

    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }
    bool isCollidingLeft()   const { return mIsCollidingLeft;   }
    bool isCollidingRight()  const { return mIsCollidingRight;  }

    void setPosition(Vector2 newPosition)         { mPosition = newPosition;         }
    void setVelocity(Vector2 newVelocity)         { mVelocity = newVelocity;         }
    void setMovement(Vector2 newMovement)         { mMovement = newMovement;         }
    void setAcceleration(Vector2 newAcceleration) { mAcceleration = newAcceleration; }
    void setScale(Vector2 newScale)               { mScale = newScale;               }
    void setColliderDimensions(Vector2 newDimensions)
        { mColliderDimensions = newDimensions; }
    void setSpeed(float newSpeed)                 { mSpeed = newSpeed;               }
    void setFrameSpeed(int newSpeed)              { mFrameSpeed = newSpeed;          }
    void setJumpingPower(float newJumpingPower)   { mJumpingPower = newJumpingPower; }
    void setAngle(float newAngle)                 { mAngle = newAngle;               }
    void setEntityType(EntityType entityType)     { mEntityType = entityType;        }
    void setAIType(AIType newType)                { mAIType = newType;               }
    void setFlipsWhenFacingLeft(bool flips)       { mFlipsWhenFacingLeft = flips;    }
    void setPatrolBounds(float minX, float maxX)
    {
        mPatrolMinX = minX;
        mPatrolMaxX = maxX;
    }
    void setAIState(AIState newState)
    {
        mAIState = newState;
        mAITimer = 0.0f;
    }
};

#endif // ENTITY_H
