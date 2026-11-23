/**
* Author: Joseph Jiminian
* Assignment: seven star rising
* Date due: n/a, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { DOWN, UP, LEFT, RIGHT       }; // same order as the sprite sheet columns
enum EntityStatus { ACTIVE, INACTIVE            };
enum EntityType   { PLAYER, NPC, BLAST, ORB     };
enum AIType       { WANDERER, FOLLOWER          };
enum AIState      { WALKING, IDLE, FOLLOWING    };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mKnockback;

    Vector2 mScale;
    Vector2 mColliderDimensions;

    Texture2D   mTexture;
    TextureType mTextureType;
    Vector2     mSpriteSheetDimensions;

    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int>                      mAnimationIndices;
    Direction                             mDirection;
    int                                   mFrameSpeed;

    int   mCurrentFrameIndex = 0;
    float mAnimationTime     = 0.0f;

    int   mSpeed;
    float mAngle;

    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType  = WANDERER;
    AIState mAIState = IDLE;

    // ----- Top-down additions ----- //
    int   mHealth          = 1;
    float mInvincibleTimer = 0.0f; // blinks and can't be hurt while > 0
    float mHurtTimer       = 0.0f; // tinted red while > 0
    int   mPoseFrame       = 0;    // a single frame shown instead of walking
    float mPoseTimer       = 0.0f;
    float mLifeTimer       = 0.0f; // blasts disappear when this runs out
    bool  mHasLifeTimer    = false;
    float mBobHeight       = 0.0f; // hovering characters float up and down
    float mBobTimer        = 0.0f;

    Vector2 mPatrolStart = { 0.0f, 0.0f }; // soldiers walk between these two points
    Vector2 mPatrolEnd   = { 0.0f, 0.0f };
    bool    mIsGoingToPatrolEnd = true;

    void checkCollisionY(Map *map);
    void checkCollisionX(Map *map);

    void resetColliderFlags()
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void moveTowards(Vector2 point);
    void AIActivate(Entity *target);
    void AIWander(Entity *target);
    void AIFollow(Entity *target);

public:
    static constexpr int   DEFAULT_SIZE          = 48;
    static constexpr int   DEFAULT_SPEED         = 200;
    static constexpr int   DEFAULT_FRAME_SPEED   = 8;
    static constexpr float COLLISION_THRESHOLD   = 0.5f;
    static constexpr float ARRIVE_DISTANCE       = 4.0f;
    static constexpr float SOLDIER_SIGHT_RANGE   = 230.0f;
    static constexpr float SCOUT_SIGHT_RANGE     = 300.0f;
    static constexpr float GIVE_UP_RANGE         = 400.0f;
    static constexpr float CHASE_SPEED_MULTIPLIER = 1.8f;
    static constexpr float KNOCKBACK_FADE        = 0.85f; // kept each step
    static constexpr float BLINK_PERIOD          = 0.1f;
    static constexpr float BOB_SPEED             = 5.0f;

    Entity();
    Entity(
        Vector2     position,
        Vector2     scale,
        const char *textureFilepath,
        EntityType  entityType
    );
    Entity(
        Vector2                               position,
        Vector2                               scale,
        const char                           *textureFilepath,
        TextureType                           textureType,
        Vector2                               spriteSheetDimensions,
        std::map<Direction, std::vector<int>> animationAtlas,
        EntityType                            entityType
    );
    ~Entity();

    void update(float deltaTime, Entity *player, Map *map);
    void render();
    void normaliseMovement() { Normalise(&mMovement); }
    bool isColliding(Entity *other) const;

    void activate()   { mEntityStatus = ACTIVE;   }
    void deactivate() { mEntityStatus = INACTIVE; }
    void displayCollider();

    bool isActive()     { return mEntityStatus == ACTIVE; }
    bool isInvincible() { return mInvincibleTimer > 0.0f; }

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mMovement.x = -1; mDirection = LEFT;  }
    void moveRight() { mMovement.x =  1; mDirection = RIGHT; }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    void takeDamage(int amount);
    void knockBack(Vector2 from, float speed);
    void showPose(int frame, float seconds)  { mPoseFrame = frame; mPoseTimer = seconds; }
    void startInvincibility(float seconds)   { mInvincibleTimer = seconds; }
    void flashRed(float seconds)             { mHurtTimer = seconds;       }

    Vector2     getPosition()           const { return mPosition;           }
    Vector2     getMovement()           const { return mMovement;           }
    Vector2     getVelocity()           const { return mVelocity;           }
    Vector2     getScale()              const { return mScale;              }
    Vector2     getColliderDimensions() const { return mColliderDimensions; }
    Direction   getDirection()          const { return mDirection;          }
    int         getSpeed()              const { return mSpeed;              }
    int         getHealth()             const { return mHealth;             }
    EntityType  getEntityType()         const { return mEntityType;         }
    AIType      getAIType()             const { return mAIType;             }
    AIState     getAIState()            const { return mAIState;            }

    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }
    bool isCollidingLeft()   const { return mIsCollidingLeft;   }
    bool isCollidingRight()  const { return mIsCollidingRight;  }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;             }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;             }
    void setKnockback(Vector2 newKnockback)
        { mKnockback = newKnockback;           }
    void setColliderDimensions(Vector2 newDimensions)
        { mColliderDimensions = newDimensions; }
    void setSpeed(int newSpeed)
        { mSpeed = newSpeed;                   }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;              }
    void setHealth(int newHealth)
        { mHealth = newHealth;                 }
    void setBobHeight(float newHeight)
        { mBobHeight = newHeight;              }
    void setLifeTimer(float seconds)
        { mLifeTimer = seconds; mHasLifeTimer = true; }
    void setDirection(Direction newDirection)
        { mDirection = newDirection;           }
    void setAIType(AIType newType)
        { mAIType = newType;                   }
    void setAIState(AIState newState)
        { mAIState = newState;                 }
    void setPatrolPoints(Vector2 start, Vector2 end)
    {
        mPatrolStart        = start;
        mPatrolEnd          = end;
        mIsGoingToPatrolEnd = true;
    }
};

#endif // ENTITY_H
