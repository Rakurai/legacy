#pragma once

class RoomPrototype;

namespace quest {

class Targetable;

class Objective {
public:
	class Target {
	public:
		enum Type {
			None,
			Mob,
			MobProto,
			Obj,
			ObjProto,
			Questgiver,
		};

		Target(Type type, Targetable *ptr) :
			type(type), ptr(ptr) {}

		Type type;
		Targetable *ptr;

	private:
		Target();
		Target(const Target&);
		Target& operator=(const Target&);
	};

	enum Type {
		speak_to_mob,
		find_obj,
		give_obj_to_mob,
		kill_mob_proto,
		report,
	};

	// construct with a subject and target
	Objective(
		Type type,
		const RoomPrototype *location,
		Target::Type target_type, Targetable *target_ptr
		Target::Type subject_type, Targetable *subject_ptr) :
		type(type),
		location(location),
		target(target_type, target_ptr)
		subject(subject_type, subject_ptr)
		{}

	// construct with only a target
	Objective(
		Type type,
		const RoomPrototype *location,
		Target::Type target_type, Targetable *target_ptr
		Target::Type subject_type, Targetable *subject_ptr) :
		Objective(type, location, target_type, target_ptr, Target::None, nullptr) {}
	Objective(const Objective& t) :
		type(t.type), location(t.location), target(t.target.type, t.target.ptr) {}
	virtual ~Objective() {}

	bool is_target(const Targetable *) const;

	const Type type;
	const RoomPrototype *location;
	Target target; // thing to do objective 'to'
	Target subject; // thing to do objective 'with'
	bool is_complete = false;

private:
	Objective& operator=(const Objective&);
};

} // namespace quest
