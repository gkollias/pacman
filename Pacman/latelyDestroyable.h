#ifndef LATELYDESTROYABLE_H_
#define LATELYDESTROYABLE_H_
#include <iostream>
#include <list>
#include <assert.h>
#include <algorithm>
class LatelyDestroyable;
class DestructionManager {
	static std::list<LatelyDestroyable*> dead;
public:
	static void Register (LatelyDestroyable* o);
	static void Commit (void);
};

class LatelyDestroyable {
	protected :
	friend class DestructionManager;
	bool alive;
	bool inDestruction;
	virtual ~LatelyDestroyable();
	class Delete : public std::unary_function<LatelyDestroyable*, void>
		{ public: void operator()(LatelyDestroyable* o) const; };
	friend class Delete;
public:
	bool IsAlive (void) const;
	void Destroy (void);
	LatelyDestroyable (void);
};


#endif