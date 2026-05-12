module;
#include <memory>

export module fcl.log.appender;

import fcl.log.log_message;
import fcl.variant;


export namespace fcl {
   class appender;

   class appender_factory {
      public:
       typedef std::shared_ptr<appender_factory> ptr;

       virtual ~appender_factory() = default;
       virtual std::shared_ptr<appender> create( const variant& args ) = 0;
   };

   namespace detail {
      template<typename T>
      class appender_factory_impl : public appender_factory {
        public:
           std::shared_ptr<appender> create( const variant& args ) override {
              return std::shared_ptr<appender>(new T(args));
           }
      };
   }

   class appender {
      public:
         typedef std::shared_ptr<appender> ptr;

         virtual ~appender() = default;
         virtual void initialize() = 0;
         virtual void log( const log_message& m ) = 0;
   };
}

