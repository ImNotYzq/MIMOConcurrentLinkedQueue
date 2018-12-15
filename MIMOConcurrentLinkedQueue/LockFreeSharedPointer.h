#pragma once

#include <type_traits>
#include <atomic>
#include "DefaultFunctionDelete.h"
#include "HazardPointer.h"

namespace concurrent {
	template<typename T>
	class LockFreeSharedPointer;

	class LockFreeSharedPointedBase
	{
		template<typename T>
		friend class LockFreeSharedPointer;
	public:
		LockFreeSharedPointedBase * operator &() = delete;
		const LockFreeSharedPointedBase * operator &() const = delete;
	protected:
		struct helper {};

		LockFreeSharedPointedBase();
		~LockFreeSharedPointedBase();

		template<typename T>
		LockFreeSharedPointer<T> GetThisPointer();
	private:
		std::atomic<uint32_t> counter;

		bool CheckAddCounter();
		bool CheckRemoveCounter();
	};

	template<typename T>
	class LockFreeSharedPointer final
		: public NewDeleted
	{
		friend class LockFreeSharedPointedBase;
		template<typename O>
		friend class LockFreeSharedPointer;
	public:
		template<typename ... Args>
		static LockFreeSharedPointer Create(Args ... args);

		LockFreeSharedPointer();
		LockFreeSharedPointer(nullptr_t);
		~LockFreeSharedPointer();

		LockFreeSharedPointer(const LockFreeSharedPointer & from);
		template<class O, std::enable_if_t<std::is_convertible<O *,T *>::value, int> = 0>
		LockFreeSharedPointer(const LockFreeSharedPointer<O> from);

		LockFreeSharedPointer & operator= (const LockFreeSharedPointer & from);
		template<class O, std::enable_if_t<std::is_convertible<O *, T *>::value, int> = 0>
		LockFreeSharedPointer & operator= (const LockFreeSharedPointer<O> & from);

		bool compare_and_exchange_weak(LockFreeSharedPointer & _Exp, LockFreeSharedPointer _Value, std::memory_order _Order = std::memory_order_seq_cst);
		bool compare_and_exchange_strong(LockFreeSharedPointer & _Exp, LockFreeSharedPointer _Value, std::memory_order _Order = std::memory_order_seq_cst);

		template<class O>
		bool operator ==(O pcomp) const;

		template<class O>
		bool operator !=(O pcomp) const;

		T & operator*() const noexcept;

		T * operator->() const noexcept;
	private:
		std::atomic<T *> pointer;

		LockFreeSharedPointer(T * p);

		T * TakePointerOut() const;
		void PutPointerIn(T * aim);
		void ReleasePointer(T * p);
	};

	template<typename T>
	inline LockFreeSharedPointer<T> LockFreeSharedPointedBase::GetThisPointer()
	{
		if (CheckAddCounter())
		{
			return LockFreeSharedPointer<T>(static_cast<T *>(this));
		}
		return nullptr;
	}

	template<typename T>
	inline LockFreeSharedPointer<T>::LockFreeSharedPointer()
		: LockFreeSharedPointer(nullptr)
	{
	}

	template<typename T>
	inline LockFreeSharedPointer<T>::LockFreeSharedPointer(nullptr_t)
		: pointer(nullptr)
	{
	}

	template<typename T>
	inline LockFreeSharedPointer<T>::LockFreeSharedPointer(T * p)
		: pointer(p)
	{
	}

	template<typename T>
	inline T * LockFreeSharedPointer<T>::TakePointerOut() const
	{
		HazardPointer hp;
		T * p;

		for(;;)
		{
			p = hp.HoldPointer(pointer);
			if (p == nullptr || p->CheckAddCounter())
				return p;
		}
	}

	template<typename T>
	inline void LockFreeSharedPointer<T>::PutPointerIn(T * aim)
	{
		T * p;

		do
		{
			p = pointer;
		} while (!pointer.compare_exchange_weak(p, aim));

		ReleasePointer(p);
	}

	template<typename T>
	inline void LockFreeSharedPointer<T>::ReleasePointer(T * p)
	{
		if (p != nullptr && p->CheckRemoveCounter())
		{
			HazardPointer::AddDeallocate<T>(p);
		}
	}

	template<typename T>
	inline LockFreeSharedPointer<T>::~LockFreeSharedPointer()
	{
		PutPointerIn(nullptr);
	}

	template<typename T>
	inline LockFreeSharedPointer<T>::LockFreeSharedPointer(const LockFreeSharedPointer & from)
		: pointer(nullptr)
	{
		PutPointerIn(from.TakePointerOut());
	}
	
	template<typename T>
	template<class O, std::enable_if_t<std::is_convertible<O *, T *>::value, int>>
	inline LockFreeSharedPointer<T>::LockFreeSharedPointer(const LockFreeSharedPointer<O> from)
		: pointer(nullptr)
	{
		PutPointerIn(from.TakePointerOut());
	}

	template<typename T>
	inline LockFreeSharedPointer<T> & LockFreeSharedPointer<T>::operator=(const LockFreeSharedPointer & from)
	{
		PutPointerIn(from.TakePointerOut());
		return *this;
	}

	template<typename T>
	inline bool LockFreeSharedPointer<T>::compare_and_exchange_weak(LockFreeSharedPointer & _Exp, LockFreeSharedPointer _Value, std::memory_order _Order)
	{
		T * aim = _Value.TakePointerOut();
		T * comparor = _Exp.pointer;

		if (pointer.compare_exchange_weak(comparor, aim, _Order))
		{
			ReleasePointer(comparor);
			return true;
		}
		else
		{
			ReleasePointer(aim);
			return false;
		}
		return false;
	}

	template<typename T>
	inline bool LockFreeSharedPointer<T>::compare_and_exchange_strong(LockFreeSharedPointer & _Exp, LockFreeSharedPointer _Value, std::memory_order _Order)
	{
		T * aim = _Value.TakePointerOut();
		T * comparor = _Exp.pointer;
		if (pointer.compare_exchange_strong(comparor, aim, _Order))
		{
			ReleasePointer(comparor);
			return true;
		}
		else
		{
			ReleasePointer(aim);
			return false;
		}
		return false;
	}

	template<typename T>
	template<class O, std::enable_if_t<std::is_convertible<O *, T *>::value, int>>
	inline LockFreeSharedPointer<T> & LockFreeSharedPointer<T>::operator=(const LockFreeSharedPointer<O> & from)
	{
		PutPointerIn(from.TakePointerOut());
		return *this;
	}

	template<typename T>
	inline T & LockFreeSharedPointer<T>::operator*() const noexcept
	{
		return *pointer;
	}

	template<typename T>
	inline T * LockFreeSharedPointer<T>::operator->() const noexcept
	{
		return pointer;
	}

	template<typename T>
	template<typename ... Args>
	inline LockFreeSharedPointer<T> LockFreeSharedPointer<T>::Create(Args ... args)
	{
		return LockFreeSharedPointer<T>(new T(LockFreeSharedPointedBase::helper{}, args...));
	}

	template<typename T>
	template<class O>
	inline bool LockFreeSharedPointer<T>::operator==(O pcomp) const
	{
		return pcomp == pointer;
	}

	template<typename T>
	template<class O>
	inline bool LockFreeSharedPointer<T>::operator!=(O pcomp) const
	{
		return pcomp != pointer;
	}
}

