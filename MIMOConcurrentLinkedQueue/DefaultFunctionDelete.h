#pragma once

class CloneDeleted {
protected:
	CloneDeleted() = default;
	~CloneDeleted() = default;
public:
	CloneDeleted(const CloneDeleted &) = delete;
	CloneDeleted& operator= (const CloneDeleted &) = delete;
};

class NewDeleted {
protected:
	NewDeleted() = default;
	~NewDeleted() = default;
public:
	void* operator new(size_t t) = delete;
	void operator delete(void* ptr) = delete;
};