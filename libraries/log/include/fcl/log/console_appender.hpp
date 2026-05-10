#pragma once
#include <fcl/log/appender.hpp>
#include <fcl/log/logger.hpp>
#include <vector>

namespace fcl
{
   class console_appender final : public appender
   {
       public:
            struct color
            {
                enum type {
                   red,
                   green,
                   brown,
                   blue,
                   magenta,
                   cyan,
                   white,
                   console_default,
                };
            };

            struct stream { enum type { std_out, std_error }; };

            struct level_color
            {
               level_color( log_level l=log_level::all,
                            color::type c=color::console_default )
               :level(l),color(c){}

               log_level                         level;
               console_appender::color::type     color;
            };

            struct config
            {
               config()
               :stream(console_appender::stream::std_error),flush(true){}

               console_appender::stream::type     stream;
               std::vector<level_color>           level_colors;
               bool                               flush;
            };


            explicit console_appender( const variant& args );
            explicit console_appender( const config& cfg );
            console_appender();

            ~console_appender();
            void initialize() override {}
            virtual void log( const log_message& m ) override;

            void print( const std::string& text_to_print,
                        color::type text_color = color::console_default );

            void configure( const config& cfg );

       private:
            class impl;
            std::unique_ptr<impl> my;
   };
} // namespace fcl

#include <fcl/reflect/reflect.hpp>
FCL_REFLECT_ENUM( fcl::console_appender::stream::type, (std_out)(std_error) )
FCL_REFLECT_ENUM( fcl::console_appender::color::type, (red)(green)(brown)(blue)(magenta)(cyan)(white)(console_default) )
FCL_REFLECT( fcl::console_appender::level_color, (level)(color) )
FCL_REFLECT( fcl::console_appender::config, (stream)(level_colors)(flush) )
