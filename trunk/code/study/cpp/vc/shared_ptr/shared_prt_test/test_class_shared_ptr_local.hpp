
#if defined( _WIN32 )
#  if defined(CPVE_EXPORT_INTERFACE)
#    define CPVE_API __declspec(dllexport)
#  else
#    define CPVE_API __declspec(dllimport)
#  endif
#else
#  define CPVE_API
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

namespace CSF {
namespace media {
namespace rtp {

	/**
	 * This class template implements a reference-counted smart pointer,
	 * similar in function to the Boost/TR1 shared_ptr, providing access
	 * to its pointee with familiar pointer semantics, and automatic cleanup
	 * when the last shared pointer goes out of scope, or is set to NULL.
	 * 
	 * Unlike Boost/TR1 shared_ptr, it does not throw exceptions, and it is mildly
	 * "intrusive" in that it requires the pointee class to derive from RefCounted.
	 */
	template <typename T>
	class SharedPtr
	{
	public:
		/**
		 * Default constructor, needed for STL collections of SharedPtr<>.
		 */
		SharedPtr();

		/**
		 * Constructor to wrap "raw" pointers,
		 * also allows assignment or comparison to NULL.
		 * 
		 * @remark "Raw" pointers should only occur (temporarily)
		 * in the implementation, never in the public interface.
		 */
		SharedPtr( T* p );

		/**
		 * Copy constructor
		 * 
		 * @remark Copying a SharedPtr increments the reference count on the pointee.
		 */
		SharedPtr( SharedPtr const& p );

		/**
		 * Constructor to allow promotion of compatible types
		 *
		 * If type \p CompatT* is assignable to type \p T*
		 * (for example, if \p CompatT derives from \p T),
		 * then \p SharedPtr<CompatT> is assignable to \p SharedPtr<T>.
		 *
		 * @remark This is primarily useful within the CPVE implementation.
		 */
		template <typename CompatT>
		SharedPtr( SharedPtr<CompatT> const& p );

		/**
		 * Destructor
		 *
		 * @remark Destroying a SharedPtr decrements the reference count on the pointee.
		 * Destruction of the last remaining reference invokes the pointee's destructor.
		 */
		~SharedPtr();

		/**
		 * Pointer operator
		 */
		T* operator-> () const;

		/**
		 * Assignment operator
		 * 
		 * @remark Assignment increments the reference count of the RHS pointee,
		 * and decrements the reference count of the (old) LHS pointee, if any.
		 */
		SharedPtr& operator= ( SharedPtr const& rhs );

		/**
		 * Equality operator
		 *  
		 * @remark Evaluates to \p 'true' if the
		 * two shared pointers refer to the same object,
		 * \b not when two distinct objects are "equal".
		 */
		bool operator== ( SharedPtr const& rhs ) const;

		/**
		 * Inequality operator 
		 * 
		 * @remark Evaluates to \p 'true' if the
		 * two shared pointers do not refer to the same object.
		 */
		bool operator!= ( SharedPtr const& rhs ) const;

		/**
		 * Comparison operator 
		 * 
		 * @remark Evaluates to \p 'true' if the
		 * two shared pointers do not refer to the same object
		 * and the address of the LHS object is "less than" that of the RHS.
		 * Useful as a comparator with std::map, etc.
		 */
		bool operator< ( SharedPtr const& rhs ) const;

		/**
		 * Explicit static_cast, valid for compatible types
		 */
		template <typename From>
		static SharedPtr cast_static( const SharedPtr<From> & from );

		/**
		 * Explicit dynamic_cast, valid for compatible types
		 */
		template <typename From>
		static SharedPtr cast_dynamic( const SharedPtr<From> & from );

	private:
		// Pointer to the managed object
		T* pointee;
	};

	/// @cond IGNORE
	/**
	 * This helper class implements a thread-safe reference counter.
	 * Any class which is to be managed by a SharedPtr must derive
	 * from this base class.
	 */
	class RefCounted
	{
	protected:
		RefCounted();
		virtual ~RefCounted();

		/**
		 * Increment the reference count by 1,
		 * and return the resulting value
		 */
		int addRef();

		/**
		 * Decrement the reference count by 1,
		 * and return the resulting value
		 */
		int decRef();

	private:
		// The reference count, initially 0
		volatile int count;
	};
	/// @endcond

} // namespace rtp
} // namespace media
} // namespace CSF
