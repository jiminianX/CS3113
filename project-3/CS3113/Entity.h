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

#include "cs3113.h"

// PLAYER   : the lander, steered ONLY through mAcceleration
// BLOCK    : ice terrain (touching it fails the mission)
// PLATFORM : landing pads and the moving barge
// HAZARD   : drifting meteors
enum EntityType { PLAYER, BLOCK, PLATFORM, HAZARD };

// The lander's sprite sheet has one row per state.
enum AnimationState { IDLE, THRUST_UP, THRUST_LEFT, THRUST_RIGHT, CRASHED };

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
    Vector2     mSpriteSheetDimensions;

    std::map<AnimationState, std::vector<int>> mAnimationAtlas;
    std::vector<int>                           mAnimationIndices;
    AnimationState                             mAnimationState;
    int                                        mFrameSpeed;

    int   mCurrentFrameIndex = 0;
    float mAnimationTime     = 0.0f;

    int   mSpeed;
    float mAngle;

    EntityType mEntityType;

    void animate(float deltaTime);

public:
    static constexpr int DEFAULT_SIZE        = 50;
    static constexpr int DEFAULT_SPEED       = 100;
    static constexpr int DEFAULT_FRAME_SPEED = 12;

    Entity();
    Entity(
        Vector2                                    position,
        Vector2                                    scale,
        const char                                *textureFilepath,
        TextureType                                textureType,
        Vector2                                    spriteSheetDimensions,
        std::map<AnimationState, std::vector<int>> animationAtlas,
        EntityType                                 entityType
    );
    ~Entity();

    void update(float deltaTime);
    void render();

    bool isColliding(Entity *other) const;

    // Switches to another row of the sprite sheet, restarting its frames.
    void setAnimation(AnimationState newState);

    void moveUp()    { mMovement.y = -1; }
    void moveDown()  { mMovement.y =  1; }
    void moveLeft()  { mMovement.x = -1; }
    void moveRight() { mMovement.x =  1; }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2        getPosition()           const { return mPosition;           }
    Vector2        getMovement()           const { return mMovement;           }
    Vector2        getVelocity()           const { return mVelocity;           }
    Vector2        getAcceleration()       const { return mAcceleration;       }
    Vector2        getScale()              const { return mScale;              }
    Vector2        getColliderDimensions() const { return mColliderDimensions; }
    Texture2D      getTexture()            const { return mTexture;            }
    AnimationState getAnimationState()     const { return mAnimationState;     }
    EntityType     getEntityType()         const { return mEntityType;         }
    int            getSpeed()              const { return mSpeed;              }
    float          getAngle()              const { return mAngle;              }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setVelocity(Vector2 newVelocity)
        { mVelocity = newVelocity;                 }
    void setAcceleration(Vector2 newAcceleration)
        { mAcceleration = newAcceleration;         }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    void setColliderDimensions(Vector2 newDimensions)
        { mColliderDimensions = newDimensions;     }
    void setSpeed(int newSpeed)
        { mSpeed = newSpeed;                       }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;                  }
    void setAngle(float newAngle)
        { mAngle = newAngle;                       }
    void setEntityType(EntityType newType)
        { mEntityType = newType;                   }
};

#endif // ENTITY_H
