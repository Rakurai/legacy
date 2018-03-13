#include "conn/State.hh"

namespace conn {

ClosedState State::closed;
GetNameState State::getName;
GetNewNameState State::getNewName;
ConfirmNewNameState State::confirmNewName;
ConfirmNewPassState State::confirmNewPass;
GetOldPassState State::getOldPass;
GetNewPassState State::getNewPass;
BreakConnectState State::breakConnect;
ReadMOTDState State::readMOTD;
ReadNewMOTDState State::readNewMOTD;
ReadIMOTDState State::readIMOTD;
GetMudExpState State::getMudExp;
GetRaceState State::getRace;
RollStatsState State::rollStats;
GetSexState State::getSex;
GetGuildState State::getGuild;
GetAlignmentState State::getAlignment;
GetDeityState State::getDeity;
GetWeaponState State::getWeapon;

CopyoverRecoverState State::copyoverRecover;
PlayingState State::playing;

} // namespace conn
