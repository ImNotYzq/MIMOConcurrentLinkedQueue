#pragma once

#include "DefaultFunctionDelete.h"

enum SingletonType {
	GlobalLazy,
	GlobalEager,
	ThreadLazy,
	ThreadEager,
};

class SingletonBase
{
public:
	SingletonBase(const CloneDeleted &) = delete;
	SingletonBase& operator =(const SingletonBase &) = delete;
	
	void* operator new(size_t t) = delete;
	void operator delete(void* ptr) = delete;

	SingletonBase * operator &() = delete;
	const SingletonBase * operator &() const = delete;
protected:
	SingletonBase() = default;
	~SingletonBase() = default;
};

template<class T, SingletonType ST = SingletonType::GlobalLazy>
class Singleton
	: public SingletonBase
{
public:
	static T & GetInstance()
	{
		static T instance(helper {});
		return instance;
	}
protected:
	struct helper {};
};

template<class T>
class Singleton<T, GlobalEager>
	: public SingletonBase
{
public:
	static T & GetInstance()
	{
		return instance;
	}
private:
	static T instance;
protected:
	struct helper {};
};

template<class T>
T Singleton<T, GlobalEager>::instance(helper {});

template<class T>
class Singleton<T, ThreadLazy>
	: public SingletonBase
{
public:
	static T & GetInstance()
	{
		thread_local static T instance(helper {});
		return instance;
	}
protected:
	struct helper {};
};

template<class T>
class Singleton<T, ThreadEager>
	: public SingletonBase
{
public:
	static T & GetInstance()
	{
		return instance;
	}
private:
	thread_local static T instance;
protected:
	struct helper {};
};

template<class T>
inline thread_local T Singleton<T, ThreadEager>::instance(helper{});