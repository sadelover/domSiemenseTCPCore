#pragma once

namespace Project
{
	namespace Tools
	{	
		template<class _Ty>
		struct my_default_delete
		{	// default deleter for unique_ptr
			typedef my_default_delete<_Ty> _Myt;

			my_default_delete()
			{	// default construct
			}

			template<class _Ty2>
			my_default_delete(const my_default_delete<_Ty2>&)
			{	// construct from another default_delete
			}

			void operator()(_Ty *_Ptr) const
			{	// delete a pointer
				if (0 < sizeof (_Ty))	// won't compile for incomplete type
					delete _Ptr;
			}
		};


		// this class is as simple as possbile to hold the stack pointers
		// to make sure that the pointer will be deleted if there is any exception
		// 
		template<typename T1, typename T2 = my_default_delete<T1>>
		class scope_ptr_holder
		{
			T1* original_pointer;
			T2	itsdeleter;
		public:

			explicit scope_ptr_holder(T1* t)  // constructor
				:original_pointer(t){}

			explicit scope_ptr_holder(T1* t , T2 deleter)  // constructor
				:original_pointer(t),
				itsdeleter(deleter){}

			void Reset(T1* newptr)
			{
				if(newptr == original_pointer)
					return;
				Release();
				original_pointer = newptr;
			}
			
			void Release()
			{
				if (original_pointer != NULL)
				{
					itsdeleter(original_pointer);
					original_pointer = NULL;
				}
			}

			~scope_ptr_holder()
			{
				Release();
			}

		protected:
			scope_ptr_holder(const scope_ptr_holder&);
			const scope_ptr_holder& operator=(const scope_ptr_holder&);
		};
	}

}

