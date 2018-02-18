// interface for objects that should be serializable to cJSON
#pragma once

namespace JSON {

class Serializable
{
public:
    virtual ~Object();

	virtual cJSON *to_cJSON() const = 0;

private:
    Serializable() {};
	Serializable(const Serializable&);
	Serializable& operator=(const Serializable&);
};

}
