/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "BossLevel.h"

namespace
{
    const char   *BOSS_TEXTURE   = "assets/sprites/creatures.png";
    const Vector2 BOSS_GRID      = { 3.0f, 9.0f };
    const Vector2 BOSS_SIZE      = { 96.0f, 96.0f };
    const Vector2 BOSS_COLLIDER  = { 88.0f, 88.0f };
    const int     BOSS_FRAME_CALM  = 11;
    const int     BOSS_FRAME_ANGRY = 12;
    const int     BOSS_FRAME_SPEED = 4;

    const int   BAR_WIDTH     = 320;
    const int   BAR_HEIGHT    = 22;
    const int   BAR_TOP       = 20;
    const int   BAR_FONT_SIZE = 20;
    const char *BOSS_NAME     = "THE CRATE KING";
}

BossLevel::BossLevel(const LevelConfig &config, const SoundBank *sounds)
    : GameLevel{ config, sounds }
{}

BossLevel::~BossLevel() { shutdown(); }

void BossLevel::initialise()
{
    mBossHealth = BOSS_MAX_HEALTH;
    GameLevel::initialise(); // calls createEnemy() for the 'B' marker
}

Entity *BossLevel::createEnemy(const Spawn &spawn)
{
    if (spawn.kind != Layout::BOSS) return GameLevel::createEnemy(spawn);

    AnimationAtlas atlas = {
        { ANIM_RUN,  { BOSS_FRAME_CALM, BOSS_FRAME_ANGRY } },
        { ANIM_IDLE, { BOSS_FRAME_ANGRY } }, // while telegraphing a charge
    };

    // Boss sprite: Kenney "Pixel Platformer" (CC0).
    Entity *boss = new Entity(
        tileCentre(spawn.row, spawn.col), BOSS_SIZE,
        BOSS_TEXTURE, ATLAS, BOSS_GRID, atlas, NPC
    );
    boss->setAIType(BOSS);
    boss->setAIState(WALKING);
    boss->setSpeed(BOSS_SPEED);
    boss->setColliderDimensions(BOSS_COLLIDER);
    boss->setFrameSpeed(BOSS_FRAME_SPEED);
    boss->setAcceleration({ 0.0f, GRAVITY });

    mBoss = boss;
    return boss;
}

// A stomp is a falling player whose centre is still above the boss's top.
bool BossLevel::isStomp(const Entity *player) const
{
    float bossTop = mBoss->getPosition().y - mBoss->getColliderDimensions().y / 2.0f;
    return player->getVelocity().y > 0.0f && player->getPosition().y < bossTop;
}

void BossLevel::checkEnemyContacts()
{
    Entity *player = mGameState.player;
    Entity *hit    = player->findCollision(mGameState.enemies);

    if (hit == nullptr) return;

    if (hit == mBoss && isStomp(player))
    {
        stompBoss();
        return;
    }

    if (!player->isInvincible()) hurtPlayer();
}

void BossLevel::stompBoss()
{
    Entity *player = mGameState.player;
    player->bounce(STOMP_BOUNCE_POWER);
    player->startInvincibility(STOMP_GRACE_PERIOD);

    if (mBoss->isInvincible()) return; // already reeling from the last hit

    mBossHealth--;
    playSound(mSounds->bossHit);

    if (mBossHealth <= 0)
    {
        mBoss->deactivate();
        mGameState.nextSceneID = mConfig.nextScene;
        return;
    }

    mBoss->startInvincibility(BOSS_HIT_INVINCIBILITY);
    mBoss->setSpeed(mBoss->getSpeed() * BOSS_SPEED_GAIN);
    mBoss->setAIState(WALKING);
}

void BossLevel::renderHealthBar() const
{
    int left = (GetScreenWidth() - BAR_WIDTH) / 2;
    int fill = BAR_WIDTH * mBossHealth / BOSS_MAX_HEALTH;

    int nameWidth = MeasureText(BOSS_NAME, BAR_FONT_SIZE);
    DrawText(BOSS_NAME, (GetScreenWidth() - nameWidth) / 2, BAR_TOP,
             BAR_FONT_SIZE, RAYWHITE);

    int barTop = BAR_TOP + BAR_FONT_SIZE + 6;
    DrawRectangle(left, barTop, BAR_WIDTH, BAR_HEIGHT, ColorFromHex("#3A3A3A"));
    DrawRectangle(left, barTop, fill,      BAR_HEIGHT, ColorFromHex("#E53B44"));
    DrawRectangleLines(left, barTop, BAR_WIDTH, BAR_HEIGHT, RAYWHITE);
}

void BossLevel::renderUI()
{
    GameLevel::renderUI();
    renderHealthBar();
}

void BossLevel::shutdown()
{
    mBoss = nullptr; // deleted with the other enemies below
    GameLevel::shutdown();
}
