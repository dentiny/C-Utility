/*
 * This file illustrates surrogate pattern.
 * 
 * Usage:
 * vector<Surrogate> vec;
 * vec[idx] = Surrogate();
 * vec[idx] = Surrogate(Base);
 */

#ifndef _SURROGATE_H_
#define _SURROGATE_H_

class Base {
public:
	virtual void copy() = 0;
	virtual ~Base() = 0;
};

class Derived : public Base {
public:
	void copy() override { }
	~Derived() override { }
};

class Surrogate {
public:
	Surrogate() :
		ptr(nullptr) {
	}

	Surrogate(const Surrogate& rhs) :
		ptr(rhs.ptr->copy()) {
	}

	Surrogate& operator=(const Surrogate& rhs) {
		if (ptr != nullptr) {
			delete ptr;
		}
		ptr = rhs.ptr->copy();
	}

	~Surrogate() {
		delete ptr;
	}

private:
	Base *ptr;
};

#endif // _SURROGATE_H_