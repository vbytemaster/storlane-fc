#pragma once
/**
 *  @file exception.hpp
 *  @brief Defines exception's used by fc
 */
#include <fcl/log/logger.hpp>
#include <exception>
#include <functional>
#include <unordered_map>
#include <boost/core/typeinfo.hpp>
#include <boost/interprocess/exceptions.hpp>

namespace fcl
{
   namespace detail { class exception_impl; }

   enum exception_code
   {
       /** for exceptions we threw that don't have an assigned code */
       unspecified_exception_code        = 0,
       unhandled_exception_code          = 1, ///< for unhandled 3rd party exceptions
       timeout_exception_code            = 2, ///< timeout exceptions
       file_not_found_exception_code     = 3,
       parse_error_exception_code        = 4,
       invalid_arg_exception_code        = 5,
       key_not_found_exception_code      = 6,
       bad_cast_exception_code           = 7,
       out_of_range_exception_code       = 8,
       canceled_exception_code           = 9,
       assert_exception_code             = 10,
       eof_exception_code                = 11,
       std_exception_code                = 13,
       invalid_operation_exception_code  = 14,
       unknown_host_exception_code       = 15,
       null_optional_code                = 16,
       udt_error_code                    = 17,
       aes_error_code                    = 18,
       overflow_code                     = 19,
       underflow_code                    = 20,
       divide_by_zero_code               = 21
   };

   /**
    *  @brief Used to generate a useful error report when an exception is thrown.
    *  @ingroup serializable
    *
    *  At each level in the stack where the exception is caught and rethrown a
    *  new log_message is added to the exception.
    *
    *  exception's are designed to be serialized to a variant and
    *  deserialized from an variant.
    *
    *  @see FCL_THROW_EXCEPTION
    *  @see FCL_RETHROW_EXCEPTION
    *  @see FCL_RETHROW_EXCEPTIONS
    */
   class exception : public std::exception
   {
      public:
         static constexpr fcl::microseconds format_time_limit = fcl::milliseconds( 10 ); // limit time spent formatting exceptions

         enum code_enum
         {
            code_value = unspecified_exception_code
         };

         explicit exception( int64_t code = unspecified_exception_code,
                             std::string_view name_value = "exception",
                             std::string_view what_value = "unspecified");
         exception( log_message&&, int64_t code = unspecified_exception_code,
                    std::string_view name_value = "exception",
                    std::string_view what_value = "unspecified");
         exception( log_messages&&, int64_t code = unspecified_exception_code,
                    std::string_view name_value = "exception",
                    std::string_view what_value = "unspecified");
         exception( const log_messages&,
                    int64_t code = unspecified_exception_code,
                    std::string_view name_value = "exception",
                    std::string_view what_value = "unspecified");
         exception( const exception& e );
         exception( exception&& e ) noexcept;
         virtual ~exception();

         const char*          name()const throw();
         int64_t              code()const throw();
         const char*          what()const noexcept override;

         /**
          *   @return a reference to log messages that have
          *   been added to this log.
          */
         const log_messages&  get_log()const;
         void                 append_log( log_message m );

         /**
          *   Generates a detailed string including file, line, method,
          *   and other information that is generally only useful for
          *   developers.
          */
         std::string to_detail_string( log_level ll = log_level::all )const;

         /**
          *   Generates a user-friendly error report.
          */
         std::string to_string( log_level ll = log_level::info  )const;

         /**
          *   Generates a user-friendly error report.
          */
         std::string top_message( )const;

         /**
          *  This is equivalent to:
          *  @code
          *   try { throwAsDynamic_exception(); }
          *   catch( ... ) { return std::current_exception(); }
          *  @endcode
          */
          virtual std::shared_ptr<exception> dynamic_copy_exception()const;

         friend void to_variant( const exception& e, variant& v );
         friend void from_variant( const variant& e, exception& ll );

         exception& operator=( const exception& copy );
         exception& operator=( exception&& copy );
      protected:
         std::unique_ptr<detail::exception_impl> my;
   };

   void to_variant( const exception& e, variant& v );
   void from_variant( const variant& e, exception& ll );
   typedef std::shared_ptr<exception> exception_ptr;

   typedef std::optional<exception> oexception;


   /**
    *  @brief re-thrown whenever an unhandled exception is caught.
    *  @ingroup serializable
    *  Any exceptions thrown by 3rd party libraries that are not
    *  caught get wrapped in an unhandled_exception exception.
    *
    *  The original exception is captured as a std::exception_ptr
    *  which may be rethrown.  The std::exception_ptr does not
    *  propgate across process boundaries.
    */
   class unhandled_exception : public exception
   {
      public:
       enum code_enum {
          code_value = unhandled_exception_code,
       };
       unhandled_exception( log_message&& m, std::exception_ptr e = std::current_exception() );
       unhandled_exception( log_messages );
       unhandled_exception( const exception&  );

       std::exception_ptr get_inner_exception()const;

       virtual std::shared_ptr<exception>   dynamic_copy_exception()const;
      private:
       std::exception_ptr _inner;
   };

   /**
    *  @brief wrapper for std::exception 
    *
    *  The original exception is captured as a std::exception_ptr
    *  which may be rethrown.  The std::exception_ptr does not
    *  propgate across process boundaries.
    */
   class std_exception_wrapper : public exception
   {
      public:
       explicit std_exception_wrapper( log_message&& m,
                                       std::exception_ptr e = std::current_exception(),
                                       const std::string& name_value = "exception",
                                       const std::string& what_value = "unspecified");

       std::exception_ptr get_inner_exception()const;

       static std_exception_wrapper from_current_exception(const std::exception& e);

       virtual std::shared_ptr<exception>   dynamic_copy_exception()const;
      private:
       std::exception_ptr _inner;
   };

   template<typename T>
   fcl::exception_ptr copy_exception( T&& e )
   {
#if defined(_MSC_VER) && (_MSC_VER < 1700)
     return std::make_shared<unhandled_exception>( log_message(),
                                                   std::copy_exception(fcl::forward<T>(e)) );
#else
     return std::make_shared<unhandled_exception>( log_message(),
                                                   std::make_exception_ptr(fcl::forward<T>(e)) );
#endif
   }


#define FCL_DECLARE_DERIVED_EXCEPTION( TYPE, BASE, CODE, WHAT ) \
   class TYPE : public BASE  \
   { \
      public: \
       enum code_enum { \
          code_value = CODE, \
       }; \
       explicit TYPE( int64_t code, const std::string& name_value, const std::string& what_value ) \
       :BASE( code, name_value, what_value ){} \
       explicit TYPE( fcl::log_message&& m, int64_t code, const std::string& name_value, const std::string& what_value ) \
       :BASE( std::move(m), code, name_value, what_value ){} \
       explicit TYPE( fcl::log_messages&& m, int64_t code, const std::string& name_value, const std::string& what_value )\
       :BASE( std::move(m), code, name_value, what_value ){}\
       explicit TYPE( const fcl::log_messages& m, int64_t code, const std::string& name_value, const std::string& what_value )\
       :BASE( m, code, name_value, what_value ){}\
       TYPE( const std::string& what_value, const fcl::log_messages& m ) \
       :BASE( m, CODE, BOOST_PP_STRINGIZE(TYPE), what_value ){} \
       TYPE( fcl::log_message&& m ) \
       :BASE( std::move(m), CODE, BOOST_PP_STRINGIZE(TYPE), WHAT ){}\
       TYPE( fcl::log_messages msgs ) \
       :BASE( std::move( msgs ), CODE, BOOST_PP_STRINGIZE(TYPE), WHAT ) {} \
       TYPE( const TYPE& c ) \
       :BASE(c){} \
       TYPE( const BASE& c ) \
       :BASE(c){} \
       TYPE():BASE(CODE, BOOST_PP_STRINGIZE(TYPE), WHAT){}\
       \
       virtual std::shared_ptr<fcl::exception> dynamic_copy_exception()const\
       { return std::make_shared<TYPE>( *this ); } \
   };

  #define FCL_DECLARE_EXCEPTION( TYPE, CODE, WHAT ) \
      FCL_DECLARE_DERIVED_EXCEPTION( TYPE, fcl::exception, CODE, WHAT )

  FCL_DECLARE_EXCEPTION( timeout_exception, timeout_exception_code, "Timeout" );
  FCL_DECLARE_EXCEPTION( file_not_found_exception, file_not_found_exception_code, "File Not Found" );
  /**
   * @brief report's parse errors
   */
  FCL_DECLARE_EXCEPTION( parse_error_exception, parse_error_exception_code, "Parse Error" );
  FCL_DECLARE_EXCEPTION( invalid_arg_exception, invalid_arg_exception_code, "Invalid Argument" );
  /**
   * @brief reports when a key, guid, or other item is not found.
   */
  FCL_DECLARE_EXCEPTION( key_not_found_exception, key_not_found_exception_code, "Key Not Found" );
  FCL_DECLARE_EXCEPTION( bad_cast_exception, bad_cast_exception_code, "Bad Cast" );
  FCL_DECLARE_EXCEPTION( out_of_range_exception, out_of_range_exception_code, "Out of Range" );

  /** @brief if an operation is unsupported or not valid this may be thrown */
  FCL_DECLARE_EXCEPTION( invalid_operation_exception,
                        invalid_operation_exception_code,
                        "Invalid Operation" );
  /** @brief if an host name can not be resolved this may be thrown */
  FCL_DECLARE_EXCEPTION( unknown_host_exception,
                         unknown_host_exception_code,
                         "Unknown Host" );

  /**
   *  @brief used to report a canceled Operation
   */
  FCL_DECLARE_EXCEPTION( canceled_exception, canceled_exception_code, "Canceled" );
  /**
   *  @brief used inplace of assert() to report violations of pre conditions.
   */
  FCL_DECLARE_EXCEPTION( assert_exception, assert_exception_code, "Assert Exception" );
  FCL_DECLARE_EXCEPTION( eof_exception, eof_exception_code, "End Of File" );
  FCL_DECLARE_EXCEPTION( null_optional, null_optional_code, "null optional" );
  FCL_DECLARE_EXCEPTION( udt_exception, udt_error_code, "UDT error" );
  FCL_DECLARE_EXCEPTION( aes_exception, aes_error_code, "AES error" );
  FCL_DECLARE_EXCEPTION( overflow_exception, overflow_code, "Integer Overflow" );
  FCL_DECLARE_EXCEPTION( underflow_exception, underflow_code, "Integer Underflow" );
  FCL_DECLARE_EXCEPTION( divide_by_zero_exception, divide_by_zero_code, "Integer Divide By Zero" );

  std::string except_str();

  void record_assert_trip(
     const char* filename,
     uint32_t lineno,
     const char* expr
     );

  extern bool enable_record_assert_trip;
} // namespace fcl

#if __APPLE__
    #define LIKELY(x)    __builtin_expect((long)!!(x), 1L)
    #define UNLIKELY(x)  __builtin_expect((long)!!(x), 0L)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

/**
 *@brief: Workaround for varying preprocessing behavior between MSVC and gcc
 */
#define FCL_EXPAND_MACRO( x ) x
/**
 *  @brief Checks a condition and throws an assert_exception if the test is FALSE
 */
#define FCL_ASSERT( TEST, ... ) \
  FCL_EXPAND_MACRO( \
    FCL_MULTILINE_MACRO_BEGIN \
      if( UNLIKELY(!(TEST)) ) \
      {                                                                      \
        if( fcl::enable_record_assert_trip )                                  \
           fcl::record_assert_trip( __FILE__, __LINE__, #TEST );              \
        FCL_THROW_EXCEPTION( fcl::assert_exception, #TEST ": "  __VA_ARGS__ ); \
      }                                                                      \
    FCL_MULTILINE_MACRO_END \
  )

#define FCL_CAPTURE_AND_THROW( EXCEPTION_TYPE, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
    throw EXCEPTION_TYPE( FCL_LOG_MESSAGE( error, "", FCL_FORMAT_ARG_PARAMS(__VA_ARGS__) ) ); \
  FCL_MULTILINE_MACRO_END

//#define FCL_THROW( FORMAT, ... )
// FCL_INDIRECT_EXPAND workas around a bug in Visual C++ variadic macro processing that prevents it
// from separating __VA_ARGS__ into separate tokens
#define FCL_INDIRECT_EXPAND(MACRO, ARGS) MACRO ARGS
#define FCL_THROW(  ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
    throw fcl::exception( FCL_INDIRECT_EXPAND(FCL_LOG_MESSAGE, ( error, __VA_ARGS__ )) );  \
  FCL_MULTILINE_MACRO_END

#define FCL_EXCEPTION( EXCEPTION_TYPE, FORMAT, ... ) \
    EXCEPTION_TYPE( FCL_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) )
/**
 *  @def FCL_THROW_EXCEPTION( EXCEPTION, FORMAT, ... )
 *  @param EXCEPTION a class in the Phoenix::Athena::API namespace that inherits
 *  @param format - a const char* string with "${keys}"
 */
#define FCL_THROW_EXCEPTION( EXCEPTION, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
    throw EXCEPTION( FCL_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END


/**
 *  @def FCL_RETHROW_EXCEPTION(ER,LOG_LEVEL,FORMAT,...)
 *  @brief Appends a log_message to the exception ER and rethrows it.
 */
#define FCL_RETHROW_EXCEPTION( ER, LOG_LEVEL, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
    ER.append_log( FCL_LOG_MESSAGE( LOG_LEVEL, FORMAT, __VA_ARGS__ ) ); \
    throw; \
  FCL_MULTILINE_MACRO_END

#define FCL_LOG_AND_RETHROW( )  \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fcl::exception& er ) { \
      wlog( "${details}", ("details",er.to_detail_string()) ); \
      FCL_RETHROW_EXCEPTION( er, warn, "rethrow" ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( warn, "rethrow ${what}: ", ("what",e.what())), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      wlog( "${details}", ("details",sew.to_detail_string()) ); \
      throw sew;\
   } catch( ... ) {  \
      fcl::unhandled_exception e( \
                FCL_LOG_MESSAGE( warn, "rethrow"), \
                std::current_exception() ); \
      wlog( "${details}", ("details",e.to_detail_string()) ); \
      throw e; \
   }

#define FCL_CAPTURE_LOG_AND_RETHROW( ... )  \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fcl::exception& er ) { \
      wlog( "${details}", ("details",er.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
      FCL_RETHROW_EXCEPTION( er, warn, "rethrow", FCL_FORMAT_ARG_PARAMS(__VA_ARGS__) ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( warn, "rethrow ${what}: ", FCL_FORMAT_ARG_PARAMS( __VA_ARGS__ )("what",e.what())), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      wlog( "${details}", ("details",sew.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
      throw sew;\
   } catch( ... ) {  \
      fcl::unhandled_exception e( \
                FCL_LOG_MESSAGE( warn, "rethrow", FCL_FORMAT_ARG_PARAMS( __VA_ARGS__) ), \
                std::current_exception() ); \
      wlog( "${details}", ("details",e.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
      throw e; \
   }

#define FCL_CAPTURE_AND_LOG( ... )  \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fcl::exception& er ) { \
      wlog( "${details}", ("details",er.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( warn, "rethrow ${what}: ",FCL_FORMAT_ARG_PARAMS( __VA_ARGS__  )("what",e.what()) ), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      wlog( "${details}", ("details",sew.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
   } catch( ... ) {  \
      fcl::unhandled_exception e( \
                FCL_LOG_MESSAGE( warn, "rethrow", FCL_FORMAT_ARG_PARAMS( __VA_ARGS__) ), \
                std::current_exception() ); \
      wlog( "${details}", ("details",e.to_detail_string()) ); \
      wdump( __VA_ARGS__ ); \
   }

#define FCL_LOG_AND_DROP( ... )  \
   catch( fcl::exception& er ) { \
      wlog( "${details}", ("details",er.to_detail_string()) ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( warn, "rethrow ${what}: ",FCL_FORMAT_ARG_PARAMS( __VA_ARGS__  )("what",e.what()) ), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ) ; \
      wlog( "${details}", ("details",sew.to_detail_string()) ); \
   } catch( ... ) {  \
      fcl::unhandled_exception e( \
                FCL_LOG_MESSAGE( warn, "rethrow", FCL_FORMAT_ARG_PARAMS( __VA_ARGS__) ), \
                std::current_exception() ); \
      wlog( "${details}", ("details",e.to_detail_string()) ); \
   }

/**
 *  @def FCL_RETHROW_EXCEPTIONS(LOG_LEVEL,FORMAT,...)
 *  @brief  Catchs all exception's, std::exceptions, and ... and rethrows them after
 *   appending the provided log message.
 */
#define FCL_RETHROW_EXCEPTIONS( LOG_LEVEL, FORMAT, ... ) \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fcl::exception& er ) { \
      FCL_RETHROW_EXCEPTION( er, LOG_LEVEL, FORMAT, __VA_ARGS__ ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( LOG_LEVEL, "${what}: " FORMAT,__VA_ARGS__("what",e.what())), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ); \
                throw sew;\
   } catch( ... ) {  \
      throw fcl::unhandled_exception( \
                FCL_LOG_MESSAGE( LOG_LEVEL, FORMAT,__VA_ARGS__), \
                std::current_exception() ); \
   }

#define FCL_CAPTURE_AND_RETHROW( ... ) \
   catch( const boost::interprocess::bad_alloc& ) {\
      throw;\
   } catch( fcl::exception& er ) { \
      FCL_RETHROW_EXCEPTION( er, warn, "", FCL_FORMAT_ARG_PARAMS(__VA_ARGS__) ); \
   } catch( const std::exception& e ) {  \
      fcl::std_exception_wrapper sew( \
                FCL_LOG_MESSAGE( warn, "${what}: ",FCL_FORMAT_ARG_PARAMS(__VA_ARGS__)("what",e.what())), \
                std::current_exception(), \
                BOOST_CORE_TYPEID(e).name(), \
                e.what() ); \
                throw sew;\
   } catch( ... ) {  \
      throw fcl::unhandled_exception( \
                FCL_LOG_MESSAGE( warn, "",FCL_FORMAT_ARG_PARAMS(__VA_ARGS__)), \
                std::current_exception() ); \
   }

#define FCL_CHECK_DEADLINE( DEADLINE, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
    if( DEADLINE < fcl::time_point::maximum() && DEADLINE < fcl::time_point::now() ) { \
       auto log_mgs = FCL_LOG_MESSAGE( error, "deadline ${d} exceeded by ${t}us ", \
             FCL_FORMAT_ARG_PARAMS(__VA_ARGS__)("d", DEADLINE)("t", fcl::time_point::now() - DEADLINE) ); \
       auto msg = log_mgs.get_limited_message(); \
       throw fcl::timeout_exception( std::move( log_mgs ), fcl::timeout_exception_code, "timeout_exception", std::move( msg ) ); \
    } \
  FCL_MULTILINE_MACRO_END
