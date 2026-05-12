module;
#include <string>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>

export module fcl.log.logger;

import fcl.log.appender;
import fcl.log.log_message;

export namespace fcl
{
   inline const std::string default_logger_name = "default";

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
         static logger get( const std::string& name = default_logger_name );
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
         void add_appender( const std::shared_ptr<appender>& a );

      private:
         class impl;
         std::shared_ptr<impl> my;
   };

} // namespace fcl
