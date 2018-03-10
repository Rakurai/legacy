#pragma once

class EQSocket
{
public:
	EQSocket() {}
	virtual ~EQSocket() {}

    char quality = 0;
    char type = 0;

private:
	EQSocket(const EQSocket&);
	EQSocket& operator=(const EQSocket&);
}
