#ifndef _COMMON_OBJECT_
#define _COMMON_OBJECT_

namespace COMMON
{
	// Object: base class with reference counting (Save/Release pattern) and user data storage
	class Object
	{
		int Reference;
		void* UserData;
#ifdef _DEBUG
		bool Debug;
#endif

	public:
		virtual void* GetUserData() const
		{
			return UserData;
		};

		virtual void SetUserData(void* data)
		{
			UserData = data;
		};

	protected:
		Object()
		{
			Reference = 0;
			UserData = nullptr;
#ifdef _DEBUG
			Debug = false;
#endif
		}

	public:
		virtual void Save()
		{
#ifdef _DEBUG
			if (Debug)
				printf("Save\n");
#endif
			++Reference;
		}

		virtual void Release()
		{
#ifdef _DEBUG
			if (Debug)
				printf("Release\n");
#endif
			if (--Reference <= 0)
				delete this;
		}

		virtual void ReleaseNoDelete()
		{
			--Reference;
		}

		virtual int GetRefCount() const
		{
			return Reference;
		}

		virtual void SetDebug(bool value)
		{
#ifdef _DEBUG
			Debug = value;
#endif
		}

		virtual Object* Clone() const
		{
			return (Object*)this;
		}

		virtual ~Object()
		{}

		bool operator ==(const Object& data) const
		{
			return false;
		}

		bool operator >(const Object& data) const
		{
			return false;
		}
	};

	// AutoPtr: smart pointer that automatically calls Save/Release on Object-derived pointers
	template <class type>
	class AutoPtr
	{
		type* Object;

	public:
		AutoPtr()
		{
			Object = NULL;
		}

		AutoPtr(type* object)
		{
			Object = object;

			if (Object)
				Object->Save();
		}

		AutoPtr(const AutoPtr<type>& object)
		{
			Object = object.Object;

			if (Object)
				Object->Save();
		}

		AutoPtr<type>& operator =(type* object)
		{
			if (Object != object)
			{
				if (Object)
					Object->Release();

				Object = object;
				if (Object)
					Object->Save();
			}

			return *this;
		}

		AutoPtr<type>& operator =(const AutoPtr<type>& object)
		{
			if (Object != object.Object)
			{
				if (Object)
					Object->Release();

				Object = object.Object;
				if (Object)
					Object->Save();
			}

			return *this;
		}

		operator type*() const
		{
			return Object;
		}

		template <class type2>
		operator type2*() const
		{
			type2* pObj = dynamic_cast<type2*>(Object);
#if _DEBUG
			assert(pObj != NULL);
#endif
			return pObj;
		}

		template <class type2>
		operator AutoPtr<type2>() const
		{
			return AutoPtr<type2>(static_cast<type2*>(Object));
		}

		type* operator ->() const
		{
			return Object;
		}

		type& operator *() const
		{
			return *Object;
		}

		bool operator ==(type* object) const
		{
			return Object == object;
		}

		bool operator !=(type* object) const
		{
			return Object != object;
		}

		bool operator ==(const AutoPtr<type>& object) const
		{
			return Object == object.Object;
		}

		bool operator !=(const AutoPtr<type>& object) const
		{
			return Object != object.Object;
		}

		bool operator >(const AutoPtr<type>& object) const
		{
			return Object > object.Object;
		}

		type* Detach()
		{
			type* pRes = Object;

			if (Object)
			{
				Object->ReleaseNoDelete();
				Object = NULL;
			}

			return pRes;
		}

		~AutoPtr()
		{
			if (Object)
				Object->Release();
			Object = NULL;
		}
	};

	// TunablePtr: smart pointer with optional auto-release (Auto mode) for flexible lifetime control
	template <class type>
	class TunablePtr
	{
		type* Object;
		bool Auto;

	public:
		TunablePtr()
		{
			Object = NULL;
			Auto = true;
		}

		TunablePtr(type* object, const bool isauto = true)
		{
			Object = object;
			Auto = isauto;

			if (Object && Auto)
				Object->Save();
		}

		TunablePtr(const AutoPtr<type>& object)
		{
			Object = object.Object;
			Auto = true;

			if (Object && Auto)
				Object->Save();
		}

		TunablePtr(const TunablePtr<type>& object)
		{
			Object = object.Object;
			Auto = object.Auto;

			if (Object && Auto)
				Object->Save();
		}

		TunablePtr<type>& operator =(type* object)
		{
			if (Object != object)
			{
				if (Object && Auto)
					Object->Release();

				Object = object;

				if (Object && Auto)
					Object->Save();
			}

			return *this;
		}

		TunablePtr<type>& operator =(const AutoPtr<type>& object)
		{
			if (Object != object.Object)
			{
				if (Object && Auto)
					Object->Release();

				Object = object.Object;
				if (Object && Auto)
					Object->Save();
			}

			return *this;
		}

		TunablePtr<type>& operator =(const TunablePtr<type>& object)
		{
			if (Object != object.Object)
			{
				if (Object && Auto)
					Object->Release();

				Object = object.Object;
				Auto = object.Auto;

				if (Object && Auto)
					Object->Save();
			}

			return *this;
		}

		operator type*() const
		{
			return Object;
		}

		template <class type2>
		operator type2*() const
		{
			type2* pObj = dynamic_cast<type2*>(Object);
#if _DEBUG
			assert(pObj != NULL);
#endif
			return pObj;
		}

		template <class type2>
		operator AutoPtr<type2>() const
		{
			return AutoPtr<type2>(static_cast<type2*>(Object));
		}

		template <class type2>
		operator TunablePtr<type2>() const
		{
			return TunablePtr<type2>(static_cast<type2*>(Object));
		}

		type* operator ->() const
		{
			return Object;
		}

		type& operator *() const
		{
			return *Object;
		}

		bool operator ==(type* object) const
		{
			return Object == object;
		}

		bool operator !=(type* object) const
		{
			return Object != object;
		}

		bool operator ==(const AutoPtr<type>& object) const
		{
			return Object == object.Object;
		}

		bool operator ==(const TunablePtr<type>& object) const
		{
			return Object == object.Object;
		}

		bool operator !=(const AutoPtr<type>& object) const
		{
			return Object != object.Object;
		}

		bool operator !=(const TunablePtr<type>& object) const
		{
			return Object != object.Object;
		}

		bool operator >(const AutoPtr<type>& object) const
		{
			return Object > object.Object;
		}

		bool operator >(const TunablePtr<type>& object) const
		{
			return Object > object.Object;
		}

		type* Detach()
		{
			type* pRes = Object;

			if (Object && Auto)
				Object->ReleaseNoDelete();
			Object = NULL;

			return pRes;
		}

		void SetAuto(const bool value)
		{
			Auto = value;
		}

		bool GetAuto() const
		{
			return Auto;
		}

		~TunablePtr()
		{
			if (Object && Auto)
				Object->Release();
			Object = NULL;
		}
	};

	using ObjectPtr = AutoPtr<Object>;
}

#endif
