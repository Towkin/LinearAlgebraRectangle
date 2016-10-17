#pragma once


class UniqueID {
private:
	unsigned int ID;
	static unsigned int sID;

protected:
	UniqueID();
	UniqueID(const UniqueID& aCopy);
	~UniqueID();

	unsigned int GetID() const { return ID; }
};

