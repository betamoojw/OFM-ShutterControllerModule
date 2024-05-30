#pragma once
#include "ModeBase.h"

class ModeShading : public ModeBase
{
    std::string _name;
    uint8_t _index;
    bool _recalcAllowed = true;
    bool _allowed = false;
    int16_t koChannelOffset();
    GroupObject& getKo(uint8_t ko);
public:
    ModeShading(uint8_t index);
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool allowed() override;
    void start() override;
    void stop() override;
    void processInputKo(GroupObject &ko) override;
};