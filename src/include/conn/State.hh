#pragma once

class Descriptor;
class String;

namespace conn {

struct ClosedState;
struct GetNameState;
struct ConfirmNewNameState;
struct ConfirmNewPassState;
struct GetOldPassState;
struct GetNewPassState;
struct BreakConnectState;
struct ReadMOTDState;
struct ReadNewMOTDState;
struct ReadIMOTDState;
struct GetMudExpState;
struct GetRaceState;
struct GetSexState;
struct GetGuildState;
struct GetAlignmentState;
struct GetDeityState;
struct GetWeaponState;
struct RollStatsState;

struct CopyoverRecoverState;
struct PlayingState;

struct State
{
	static ClosedState  closed;
	static GetNameState getName;
	static ConfirmNewNameState confirmNewName;
	static ConfirmNewPassState confirmNewPass;
	static GetOldPassState getOldPass;
	static GetNewPassState getNewPass;
	static BreakConnectState breakConnect;
	static ReadMOTDState readMOTD;
	static ReadNewMOTDState readNewMOTD;
	static ReadIMOTDState readIMOTD;
	static GetMudExpState getMudExp;
	static GetRaceState getRace;
	static GetSexState getSex;
	static GetGuildState getGuild;
	static GetAlignmentState getAlignment;
	static GetDeityState getDeity;
	static GetWeaponState getWeapon;
	static RollStatsState rollStats;

	static CopyoverRecoverState copyoverRecover;
	static PlayingState playing;

	virtual State *handleInput(Descriptor *, const String&) = 0;
};

struct ClosedState  : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetNameState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct ConfirmNewNameState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct ConfirmNewPassState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetOldPassState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetNewPassState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct BreakConnectState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct ReadMOTDState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct ReadNewMOTDState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct ReadIMOTDState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetMudExpState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetRaceState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetSexState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetGuildState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetAlignmentState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetDeityState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct GetWeaponState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct RollStatsState : public State { virtual State *handleInput(Descriptor *, const String&); };

struct CopyoverRecoverState : public State { virtual State *handleInput(Descriptor *, const String&); };
struct PlayingState : public State { virtual State *handleInput(Descriptor *, const String&); };

} // namespace conn
