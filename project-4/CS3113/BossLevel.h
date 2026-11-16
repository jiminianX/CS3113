/**

* Author: Joseph Jiminian

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#ifndef BOSS_LEVEL_H
#define BOSS_LEVEL_H

#include "GameLevel.h"

class BossLevel : public GameLevel
{
private:
    Entity *mBoss       = nullptr;
    int     mBossHealth = 0;

    bool isStomp(const Entity *player) const;
    void stompBoss();
    void renderHealthBar() const;

protected:
    Entity *createEnemy(const Spawn &spawn) override;
    void    checkEnemyContacts()            override;

public:
    static constexpr int   BOSS_MAX_HEALTH        = 3;
    static constexpr float BOSS_SPEED             = 120.0f;
    static constexpr float BOSS_SPEED_GAIN        = 1.3f;
    static constexpr float BOSS_HIT_INVINCIBILITY = 1.0f;
    static constexpr float STOMP_BOUNCE_POWER     = 650.0f;
    static constexpr float STOMP_GRACE_PERIOD     = 0.4f;

    BossLevel(const LevelConfig &config, const SoundBank *sounds);
    ~BossLevel();

    void initialise() override;
    void renderUI()   override;
    void shutdown()   override;
};

#endif // BOSS_LEVEL_H
