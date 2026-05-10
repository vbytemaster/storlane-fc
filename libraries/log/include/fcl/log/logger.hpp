#pragma once
#include <fcl/core/time.hpp>
#include <fcl/log/log_message.hpp>
#include <string>

inline const std::string DEFAULT_LOGGER = "default";

namespace fcl
{

   class appender;

   /**
    *
    *
    @code
      void my_class::func()
      {
         fcl_dlog( my_class_logger, "Format four: ${arg}  five: ${five}", ("arg",4)("five",5) );
      }
    @endcode
    */
   class logger
   {
      public:
         static logger& default_logger();
         static logger get( const std::string& name = DEFAULT_LOGGER );
         static void update( const std::string& name, logger& log );

         logger();
         logger( const std::string& name, const logger& parent = nullptr );
         logger( std::nullptr_t );
         logger( const logger& c );
         logger( logger&& c ) noexcept;
         ~logger();
         logger& operator=(const logger&);
         logger& operator=(logger&&) noexcept;
         friend bool operator==( const logger&, nullptr_t );
         friend bool operator!=( const logger&, nullptr_t );

         logger&    set_log_level( log_level e );
         log_level  get_log_level()const;
         logger&    set_parent( const logger& l );
         logger     get_parent()const;

         void  set_name( const std::string& n );
         std::string get_name()const;

         void set_enabled( bool e );
         bool is_enabled( log_level e )const;
         bool is_enabled()const;
         void log( log_message m );

      private:
         friend struct log_config;
         void add_appender( const std::shared_ptr<appender>& a );

      private:
         class impl;
         std::shared_ptr<impl> my;
   };

} // namespace fcl

// suppress warning "conditional expression is constant" in the while(0) for visual c++
// http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define FCL_MULTILINE_MACRO_BEGIN do {
#ifdef _MSC_VER
# define FCL_MULTILINE_MACRO_END \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) \
    } while (0) \
    __pragma(warning(pop))
#else
# define FCL_MULTILINE_MACRO_END  } while (0)
#endif

#define fcl_tlog( LOGGER, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fcl::log_level::all ) ) \
      (LOGGER).log( FCL_LOG_MESSAGE( all, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END

#define fcl_dlog( LOGGER, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fcl::log_level::debug ) ) \
      (LOGGER).log( FCL_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END

#define fcl_ilog( LOGGER, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fcl::log_level::info ) ) \
      (LOGGER).log( FCL_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END

#define fcl_wlog( LOGGER, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fcl::log_level::warn ) ) \
      (LOGGER).log( FCL_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END

#define fcl_elog( LOGGER, FORMAT, ... ) \
  FCL_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fcl::log_level::error ) ) \
      (LOGGER).log( FCL_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FCL_MULTILINE_MACRO_END

#define tlog( FORMAT, ... ) \
   fcl_tlog( fcl::logger::default_logger(), FORMAT, __VA_ARGS__)

#define dlog( FORMAT, ... ) \
   fcl_dlog( fcl::logger::default_logger(), FORMAT, __VA_ARGS__)

#define ilog( FORMAT, ... ) \
   fcl_ilog( fcl::logger::default_logger(), FORMAT, __VA_ARGS__)

#define wlog( FORMAT, ... ) \
   fcl_wlog( fcl::logger::default_logger(), FORMAT, __VA_ARGS__)

#define elog( FORMAT, ... ) \
   fcl_elog( fcl::logger::default_logger(), FORMAT, __VA_ARGS__)

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>

#define FCL_FORMAT_ARG(r, unused, base) \
  BOOST_PP_STRINGIZE(base) ": ${" BOOST_PP_STRINGIZE( base ) "} "

#define FCL_FORMAT_ARGS(r, unused, base) \
  BOOST_PP_LPAREN() BOOST_PP_STRINGIZE(base),fcl::variant(base) BOOST_PP_RPAREN()

#define FCL_FORMAT( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FCL_FORMAT_ARG, v, SEQ )

// takes a ... instead of a SEQ arg because it can be called with an empty SEQ
// from FCL_CAPTURE_AND_THROW()
#define FCL_FORMAT_ARG_PARAMS( ... )\
    BOOST_PP_SEQ_FOR_EACH( FCL_FORMAT_ARGS, v, __VA_ARGS__ )

#define idump( SEQ ) \
    ilog( FCL_FORMAT(SEQ), FCL_FORMAT_ARG_PARAMS(SEQ) )
#define wdump( SEQ ) \
    wlog( FCL_FORMAT(SEQ), FCL_FORMAT_ARG_PARAMS(SEQ) )
#define edump( SEQ ) \
    elog( FCL_FORMAT(SEQ), FCL_FORMAT_ARG_PARAMS(SEQ) )

// this disables all normal logging statements -- not something you'd normally want to do,
// but it's useful if you're benchmarking something and suspect logging is causing
// a slowdown.
#ifdef FCL_DISABLE_LOGGING
# undef ulog
# define ulog(...) FCL_MULTILINE_MACRO_BEGIN FCL_MULTILINE_MACRO_END
# undef elog
# define elog(...) FCL_MULTILINE_MACRO_BEGIN FCL_MULTILINE_MACRO_END
# undef wlog
# define wlog(...) FCL_MULTILINE_MACRO_BEGIN FCL_MULTILINE_MACRO_END
# undef ilog
# define ilog(...) FCL_MULTILINE_MACRO_BEGIN FCL_MULTILINE_MACRO_END
# undef dlog
# define dlog(...) FCL_MULTILINE_MACRO_BEGIN FCL_MULTILINE_MACRO_END
#endif