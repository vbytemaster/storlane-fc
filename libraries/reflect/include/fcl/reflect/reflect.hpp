#pragma once
/**
 * @file fc/reflect.hpp
 *
 * @brief Defines types and macros used to provide reflection.
 *
 */

#include <fcl/core/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <stdint.h>
#include <string.h>
#include <string>

#include <fcl/reflect/typename.hpp>

namespace fcl {

/**
 *  @brief defines visit functions for T
 *  Unless this is specialized, visit() will not be defined for T.
 *
 *  @tparam T - the type that will be visited.
 *
 *  The @ref FCL_REFLECT(TYPE,MEMBERS) or FCL_STATIC_REFLECT_DERIVED(TYPE,BASES,MEMBERS) macro is used to specialize this
 *  class for your type.
 */
template<typename T>
struct reflector{
    typedef T type;
    typedef fcl::false_type is_defined;
    typedef fcl::false_type is_enum;

    /**
     *  @tparam Visitor a function object of the form:
     *
     *    @code
     *     struct functor {
     *        template<typename Member, class Class, Member (Class::*member)>
     *        void operator()( const char* name )const;
     *     };
     *    @endcode
     *
     *  If reflection requires an init (what a constructor might normally do) then
     *  derive your Visitor publicly from fcl::reflector_init_visitor, derive your reflected
     *  type from fcl::reflect_init and implement a reflector_init() method
     *  on your reflected type. reflector_init() needs to be public or you can friend:
     *     friend struct fcl::reflector_init_visitor<your_reflected_class>;
     *     friend struct fcl::has_reflector_init<your_reflected_class>;
     *  Note that if your attributes are also protected/private then you also need:
     *     friend struct fcl::reflector<your_reflected_class>;
     *
     *    @code
     *     template<typename Class>
     *     struct functor : public fcl::reflector_init_visitor<Class>  {
     *        functor(Class& _c)
     *        : fcl::reflector_init_visitor<Class>(_c) {}
     *
     *        template<typename Member, class Class, Member (Class::*member)>
     *        void operator()( const char* name )const;
     *     };
     *    @endcode
     *
     *  If T is an enum then the functor has the following form:
     *    @code
     *     struct functor {
     *        template<int Value>
     *        void operator()( const char* name )const;
     *     };
     *    @endcode
     *
     *  @param v a functor that will be called for each member on T
     *
     *  @note - this method is not defined for non-reflected types.
     */
    #ifdef DOXYGEN
    template<typename Visitor>
    static inline void visit( const Visitor& v );
    #endif // DOXYGEN
};

void throw_bad_enum_cast( int64_t i, const char* e );
void throw_bad_enum_cast( const char* k, const char* e );

template<typename C>
struct has_reflector_init {
private:
   template<typename T>
   static auto test( int ) -> decltype( std::declval<T>().reflector_init(), std::true_type() ) { return {}; }
   template<typename>
   static std::false_type test( long ) { return {}; }
public:
   static constexpr bool value = std::is_same<decltype( test<C>( 0 ) ), std::true_type>::value;
};

struct reflect_init {};

template <typename Class>
struct reflector_init_visitor {
   explicit reflector_init_visitor( Class& c )
     : obj(c) {}

   void reflector_init() const {
      reflect_init( obj );
   }
   void reflector_init() {
      reflect_init( obj );
   }

 private:

   // 0 matches int if Class derived from reflect_init (SFINAE)
   template<class T>
   typename std::enable_if<std::is_base_of<fcl::reflect_init, T>::value>::type
   init_imp(T& t) {
      t.reflector_init();
   }

   // SFINAE if Class not derived from reflect_init
   template<class T>
   typename std::enable_if<not std::is_base_of<fcl::reflect_init, T>::value>::type
   init_imp(T& t) {}

   template<typename T>
   auto reflect_init(T& t) -> decltype(init_imp(t), void()) {
      init_imp(t);
   }

 protected:
   Class& obj;
};

} // namespace fcl


#ifndef DOXYGEN

#define FCL_REFLECT_VISIT_BASE(r, visitor, base) \
  fcl::reflector<base>::visit_base( visitor );


#ifndef _MSC_VER
  #define FCL_TEMPLATE template
#else
  // Disable warning C4482: nonstandard extention used: enum 'enum_type::enum_value' used in qualified name
  #pragma warning( disable: 4482 )
  #define FCL_TEMPLATE
#endif

#define FCL_REFLECT_VISIT_MEMBER( r, visitor, elem ) \
{ typedef decltype((static_cast<type*>(nullptr))->elem) member_type;  \
  visitor.FCL_TEMPLATE operator()<member_type,type,&type::elem>( BOOST_PP_STRINGIZE(elem) ); \
}


#define FCL_REFLECT_BASE_MEMBER_COUNT( r, OP, elem ) \
  OP fcl::reflector<elem>::total_member_count

#define FCL_REFLECT_MEMBER_COUNT( r, OP, elem ) \
  OP 1

#define FCL_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit_base( Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} \
template<typename Visitor>\
static inline void visit( Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
    init( std::forward<Visitor>(v) ); \
}

#endif // DOXYGEN


#define FCL_REFLECT_VISIT_ENUM( r, enum_type, elem ) \
  v.operator()(BOOST_PP_STRINGIZE(elem), int64_t(enum_type::elem) );
#define FCL_REFLECT_ENUM_TO_STRING( r, enum_type, elem ) \
   case enum_type::elem: return BOOST_PP_STRINGIZE(elem);
#define FCL_REFLECT_ENUM_TO_FC_STRING( r, enum_type, elem ) \
   case enum_type::elem: return std::string(BOOST_PP_STRINGIZE(elem));

#define FCL_REFLECT_ENUM_FROM_STRING( r, enum_type, elem ) \
  if( strcmp( s, BOOST_PP_STRINGIZE(elem)  ) == 0 ) return enum_type::elem;
#define FCL_REFLECT_ENUM_FROM_STRING_CASE( r, enum_type, elem ) \
   case enum_type::elem:

#define FCL_REFLECT_ENUM( ENUM, FIELDS ) \
namespace fcl { \
template<> struct reflector<ENUM> { \
    typedef fcl::true_type is_defined; \
    typedef fcl::true_type is_enum; \
    static const char* to_string(ENUM elem) { \
      switch( elem ) { \
        BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_ENUM_TO_STRING, ENUM, FIELDS ) \
        default: \
           fcl::throw_bad_enum_cast( std::to_string(int64_t(elem)).c_str(), BOOST_PP_STRINGIZE(ENUM) ); \
      }\
      return nullptr; \
    } \
    static const char* to_string(int64_t i) { \
      return to_string(ENUM(i)); \
    } \
    static std::string to_fc_string(ENUM elem) { \
      switch( elem ) { \
        BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_ENUM_TO_FC_STRING, ENUM, FIELDS ) \
      } \
      return std::to_string(int64_t(elem)); \
    } \
    static std::string to_fc_string(int64_t i) { \
      return to_fc_string(ENUM(i)); \
    } \
    static ENUM from_int(int64_t i) { \
      ENUM e = ENUM(i); \
      switch( e ) \
      { \
        BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_ENUM_FROM_STRING_CASE, ENUM, FIELDS ) \
          break; \
        default: \
          fcl::throw_bad_enum_cast( i, BOOST_PP_STRINGIZE(ENUM) ); \
      } \
      return e;\
    } \
    static ENUM from_string( const char* s ) { \
        BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_ENUM_FROM_STRING, ENUM, FIELDS ) \
        int64_t i = 0; \
        try \
        { \
           i = boost::lexical_cast<int64_t>(s); \
        } \
        catch( const boost::bad_lexical_cast& e ) \
        { \
           fcl::throw_bad_enum_cast( s, BOOST_PP_STRINGIZE(ENUM) ); \
        } \
        return from_int(i); \
    } \
    template< typename Visitor > \
    static void visit( Visitor& v ) \
    { \
        BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_ENUM, ENUM, FIELDS ) \
    } \
};  \
template<> struct get_typename<ENUM>  { static const char* name()  { return BOOST_PP_STRINGIZE(ENUM);  } }; \
}

/*  Note: FCL_REFLECT_ENUM previously defined this function, but I don't think it ever
 *        did what we expected it to do.  I've disabled it for now.
 *
 *  template<typename Visitor> \
 *  static inline void visit( const Visitor& v ) { \
 *      BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_VISIT_ENUM, ENUM, FIELDS ) \
 *  }\
 */

/**
 *  @def FCL_REFLECT_DERIVED(TYPE,INHERITS,MEMBERS)
 *
 *  @brief Specializes fcl::reflector for TYPE where
 *         type inherits other reflected classes
 *
 *  @param INHERITS - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define FCL_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, INHERITS, MEMBERS ) \
namespace fcl {  \
  template<BOOST_PP_SEQ_ENUM(TEMPLATE_ARGS)> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
template<BOOST_PP_SEQ_ENUM(TEMPLATE_ARGS)> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef fcl::true_type  is_defined; \
    typedef fcl::false_type is_enum; \
    template<typename Visitor> \
    static auto init_imp(Visitor&& v, int) -> decltype(std::forward<Visitor>(v).reflector_init(), void()) { \
       std::forward<Visitor>(v).reflector_init(); \
    } \
    template<typename Visitor> \
    static auto init_imp(Visitor&& v, long) -> decltype(v, void()) {} \
    template<typename Visitor> \
    static auto init(Visitor&& v) -> decltype(init_imp(std::forward<Visitor>(v), 0), void()) { \
       init_imp(std::forward<Visitor>(v), 0); \
    } \
    enum  member_count_enum {  \
      local_member_count = 0  BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_MEMBER_COUNT, +, MEMBERS ),\
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( FCL_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    FCL_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
    static_assert( not fcl::has_reflector_init<TYPE>::value || \
                   std::is_base_of<fcl::reflect_init, TYPE>::value, "must derive from fcl::reflect_init" ); \
    static_assert( not std::is_base_of<fcl::reflect_init, TYPE>::value || \
                   fcl::has_reflector_init<TYPE>::value, "must provide reflector_init() method" ); \
}; }

#define FCL_REFLECT_DERIVED( TYPE, INHERITS, MEMBERS ) \
   FCL_REFLECT_DERIVED_TEMPLATE( (), TYPE, INHERITS, MEMBERS )

//BOOST_PP_SEQ_SIZE(MEMBERS),

/**
 *  @def FCL_REFLECT(TYPE,MEMBERS)
 *  @brief Specializes fcl::reflector for TYPE
 *
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 *
 *  @see FCL_REFLECT_DERIVED
 */
#define FCL_REFLECT( TYPE, MEMBERS ) \
    FCL_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define FCL_REFLECT_TEMPLATE( TEMPLATE_ARGS, TYPE, MEMBERS ) \
    FCL_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define FCL_REFLECT_EMPTY( TYPE ) \
    FCL_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, BOOST_PP_SEQ_NIL )

#define FCL_REFLECT_TYPENAME( TYPE ) \
namespace fcl { \
  template<> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
}
