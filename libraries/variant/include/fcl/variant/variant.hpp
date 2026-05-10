#pragma once

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fcl/core/string.hpp>
#include <fcl/core/time.hpp>
#include <fcl/core/container/deque_fwd.hpp>
#include <fcl/core/container/flat_fwd.hpp>
#include <boost/multi_index_container_fwd.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <variant>

namespace fcl
{
   /**
    * @defgroup serializable Serializable _types
    * @brief Classes that may be converted to/from an variant
    *
    * To make a class 'serializable' the following methods must be available
    * for your Serializable_type
    *
    *  @code
    *     void   to_variant( const Serializable_type& e, variant& v );
    *     void   from_variant( const variant& e, Serializable_type& ll );
    *  @endcode
    */

   class variant;
   class variant_object;
   class mutable_variant_object;
   class time_point;
   class time_point_sec;
   class microseconds;
   template<typename T> struct safe;

   struct blob { std::vector<char> data; };

   void to_variant( const blob& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  blob& vo );


   template<typename T, typename... Args> void to_variant( const boost::multi_index_container<T,Args...>& s, fcl::variant& v );
   template<typename T, typename... Args> void from_variant( const fcl::variant& v, boost::multi_index_container<T,Args...>& s );

   template<size_t Size>
   using UInt = boost::multiprecision::number<
         boost::multiprecision::cpp_int_backend<Size, Size, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void> >;
   template<size_t Size>
   using Int = boost::multiprecision::number<
         boost::multiprecision::cpp_int_backend<Size, Size, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void> >;

   void to_variant( const UInt<8>& n, fcl::variant& v );
   void from_variant( const fcl::variant& v, UInt<8>& n );

   void to_variant( const UInt<16>& n, fcl::variant& v );
   void from_variant( const fcl::variant& v, UInt<16>& n );

   void to_variant( const UInt<32>& n, fcl::variant& v );
   void from_variant( const fcl::variant& v, UInt<32>& n );

   void to_variant( const UInt<64>& n, fcl::variant& v );
   void from_variant( const fcl::variant& v, UInt<64>& n );

   template<typename T> void to_variant( const boost::multiprecision::number<T>& n, fcl::variant& v );
   template<typename T> void from_variant( const fcl::variant& v, boost::multiprecision::number<T>& n );

   template<typename T> void to_variant( const safe<T>& s, fcl::variant& v );
   template<typename T> void from_variant( const fcl::variant& v, safe<T>& s );
   template<typename T> void to_variant( const std::unique_ptr<T>& s, fcl::variant& v );
   template<typename T> void from_variant( const fcl::variant& v, std::unique_ptr<T>& s );

   template<typename... T> void to_variant( const std::variant<T...>& s, fcl::variant& v );
   template<typename... T> void from_variant( const fcl::variant& v, std::variant<T...>& s );

   void to_variant( const uint8_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  uint8_t& vo );
   void to_variant( const int8_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  int8_t& vo );

   void to_variant( const uint16_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  uint16_t& vo );
   void to_variant( const int16_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  int16_t& vo );

   void to_variant( const uint32_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  uint32_t& vo );
   void to_variant( const int32_t& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  int32_t& vo );

   void to_variant( const unsigned __int128& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  unsigned __int128& vo );
   void to_variant( const __int128& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  __int128& vo );

   void to_variant( const variant_object& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  variant_object& vo );
   void to_variant( const mutable_variant_object& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  mutable_variant_object& vo );
   void to_variant( const std::vector<char>& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  std::vector<char>& vo );

   template<typename K, typename T>
   void to_variant( const std::unordered_map<K,T>& var,  fcl::variant& vo );
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::unordered_map<K,T>& vo );

   template<typename K, typename T>
   void to_variant( const std::map<K,T>& var,  fcl::variant& vo );
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::map<K,T>& vo );
   template<typename K, typename T>
   void to_variant( const std::multimap<K,T>& var,  fcl::variant& vo );
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::multimap<K,T>& vo );


   template<typename T>
   void to_variant( const std::unordered_set<T>& var,  fcl::variant& vo );
   template<typename T>
   void from_variant( const fcl::variant& var,  std::unordered_set<T>& vo );

   template<typename T>
   void to_variant( const std::deque<T>& var,  fcl::variant& vo );
   template<typename T>
   void from_variant( const fcl::variant& var,  std::deque<T>& vo );

   template<typename T, typename... U>
   void to_variant( const boost::container::deque<T, U...>& d, fcl::variant& vo );
   template<typename T, typename... U>
   void from_variant( const fcl::variant& v, boost::container::deque<T, U...>& d );

   template<typename T>
   void to_variant( const std::set<T>& var,  fcl::variant& vo );
   template<typename T>
   void from_variant( const fcl::variant& var,  std::set<T>& vo );

   template<typename T, std::size_t S>
   void to_variant( const std::array<T,S>& var,  fcl::variant& vo );
   template<typename T, std::size_t S>
   void from_variant( const fcl::variant& var,  std::array<T,S>& vo );

   template<typename T>
   void to_variant( const std::initializer_list<T>& var,  fcl::variant& vo );

   void to_variant( const time_point& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  time_point& vo );

   void to_variant( const time_point_sec& var,  fcl::variant& vo );
   void from_variant( const fcl::variant& var,  time_point_sec& vo );

   void to_variant( const microseconds& input_microseconds,  fcl::variant& output_variant );
   void from_variant( const fcl::variant& input_variant,  microseconds& output_microseconds );

   #ifdef __APPLE__
   void to_variant( size_t s, fcl::variant& v );
   #elif !defined(_MSC_VER)
   void to_variant( long long int s, fcl::variant& v );
   void to_variant( unsigned long long int s, fcl::variant& v );
   #endif
   void to_variant( const std::string& s, fcl::variant& v );

   template<typename T>
   void to_variant( const std::shared_ptr<T>& var,  fcl::variant& vo );

   template<typename T>
   void from_variant( const fcl::variant& var,  std::shared_ptr<T>& vo );

   using variants = std::vector<fcl::variant>;
   template<typename A, typename B>
   void to_variant( const std::pair<A,B>& t, fcl::variant& v );
   template<typename A, typename B>
   void from_variant( const fcl::variant& v, std::pair<A,B>& p );



   /**
    * @brief stores null, int64, uint64, double, bool, string, std::vector<variant>,
    *        and variant_object's.
    *
    * variant's allocate everything but strings, arrays, and objects on the
    * stack and are 'move aware' for values allcoated on the heap.
    *
    * Memory usage on 64 bit systems is 16 bytes and 12 bytes on 32 bit systems.
    */
   class variant
   {
      public:
        enum type_id
        {
           null_type   = 0,
           int64_type  = 1,
           uint64_type = 2,
           double_type = 3,
           bool_type   = 4,
           string_type = 5,
           array_type  = 6,
           object_type = 7,
           blob_type   = 8
        };

        /// Constructs a null_type variant
        variant();
        /// Constructs a null_type variant
        variant( nullptr_t );

        /// @param str - UTF8 string
        variant( const char* str );
        variant( char* str );
        variant( wchar_t* str );
        variant( const wchar_t* str );
        variant( float val );
        variant( uint8_t val );
        variant( int8_t val );
        variant( uint16_t val );
        variant( int16_t val );
        variant( uint32_t val );
        variant( int32_t val );
        variant( uint64_t val );
        variant( int64_t val );
        variant( double val );
        variant( bool val );
        variant( blob val );
        variant( std::string val );
        variant( variant_object );
        variant( mutable_variant_object );
        variant( variants );
        variant( const variant& );
        variant( variant&& );
       ~variant();

        /**
         *  Read-only access to the content of the variant.
         */
        class visitor
        {
           public:
              virtual ~visitor(){}
              /// handles null_type variants
              virtual void handle()const                         = 0;
              virtual void handle( const int64_t& v )const       = 0;
              virtual void handle( const uint64_t& v )const      = 0;
              virtual void handle( const double& v )const        = 0;
              virtual void handle( const bool& v )const          = 0;
              virtual void handle( const std::string& v )const   = 0;
              virtual void handle( const variant_object& v)const = 0;
              virtual void handle( const variants& v)const       = 0;
              virtual void handle( const blob& v)const           = 0;
        };

        void  visit( const visitor& v )const;

        type_id                     get_type()const;

        bool                        is_null()const;
        bool                        is_string()const;
        bool                        is_bool()const;
        bool                        is_int64()const;
        bool                        is_uint64()const;
        bool                        is_double()const;
        bool                        is_object()const;
        bool                        is_array()const;
        bool                        is_blob()const;
        /**
         *   int64, uint64, double,bool
         */
        bool                        is_numeric()const;
        /**
         *   int64, uint64, bool
         */
        bool                        is_integer()const;

        int64_t                     as_int64()const;
        uint64_t                    as_uint64()const;
        bool                        as_bool()const;
        double                      as_double()const;

        blob&                       get_blob();
        const blob&                 get_blob()const;
        blob                        as_blob()const;

        /** Convert's double, ints, bools, etc to a string
         * @throw if get_type() == array_type | get_type() == object_type
         */
        std::string                 as_string()const;

        /// @pre  get_type() == string_type
        const std::string&          get_string()const;

        /// @throw if get_type() != array_type | null_type
        variants&                   get_array();

        /// @throw if get_type() != array_type
        const variants&             get_array()const;

        /// @throw if get_type() != object_type | null_type
        variant_object&             get_object();

        /// @throw if get_type() != object_type
        const variant_object&       get_object()const;

        /// @pre is_object()
        const variant&              operator[]( const char* )const;
        /// @pre is_array()
        const variant&              operator[]( size_t pos )const;
        /// @pre is_array()
        size_t                      size()const;

        size_t                      estimated_size()const;
        /**
         *  _types that use non-intrusive variant conversion can implement the
         *  following method to implement conversion from variant to T.
         *
         *  <code>
         *  void from_variant( const Variant& var, T& val )
         *  </code>
         *
         *  The above form is not always convienant, so the this templated
         *  method is used to enable conversion from Variants to other
         *  types.
         */
        template<typename T>
        T as()const
        {
           T tmp;
           from_variant( *this, tmp );
           return tmp;
        }

        template<typename T>
        void as( T& v )const
        {
           from_variant( *this, v );
        }

        variant& operator=( variant&& v );
        variant& operator=( const variant& v );

        template<typename T>
        variant& operator=( T&& v )
        {
           return *this = variant( fcl::forward<T>(v) );
        }

        template<typename T>
        explicit variant( const std::optional<T>& v )
        {
           if( v.has_value() ) *this = variant(*v);
        }

        template<typename T>
        explicit variant( const T& val );

        template<typename T>
        explicit variant( const T& val, const fcl::yield_function_t& yield );

        void    clear();
      private:
        void    init();
        //enough room to store pointers, doubles, uint64s. doubled to allow 1 extra byte to store type at the end
        alignas(double) std::array<char, std::max(sizeof(uintmax_t ), sizeof(double)) * 2> _data = {};
   };

   typedef std::optional<variant> ovariant;

   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  std::string& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  fcl::variants& vo );
   void from_variant( const fcl::variant& var,  fcl::variant& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  int64_t& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  uint64_t& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  bool& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  double& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  float& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  int32_t& vo );
   /** @ingroup Serializable */
   void from_variant( const fcl::variant& var,  uint32_t& vo );
   /** @ingroup Serializable */
   template<typename T>
   void from_variant( const variant& var,  std::optional<T>& vo )
   {
      if( var.is_null() ) vo = std::optional<T>();
      else
      {
          vo = T();
          from_variant( var, *vo );
      }
   }
   template<typename T>
   void to_variant( const std::unordered_set<T>& var,  fcl::variant& vo )
   {
       if( var.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
       variants vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = fcl::variant(*itr);
       vo = std::move(vars);
   }
   template<typename T>
   void from_variant( const fcl::variant& var,  std::unordered_set<T>& vo )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      vo.clear();
      vo.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as<T>() );
   }


   template<typename K, typename T>
   void to_variant( const std::unordered_map<K, T>& var,  fcl::variant& vo )
   {
       if( var.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
       std::vector< fcl::variant > vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = fcl::variant(*itr);
       vo = vars;
   }
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::unordered_map<K, T>& vo )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      vo.clear();
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as< std::pair<K,T> >() );

   }
   template<typename K, typename T>
   void to_variant( const std::map<K, T>& var,  fcl::variant& vo )
   {
       if( var.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
       std::vector< fcl::variant > vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = fcl::variant(*itr);
       vo = vars;
   }
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::map<K, T>& vo )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      vo.clear();
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as< std::pair<K,T> >() );
   }

   template<typename K, typename T>
   void to_variant( const std::multimap<K, T>& var,  fcl::variant& vo )
   {
       if( var.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
       std::vector< fcl::variant > vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = fcl::variant(*itr);
       vo = vars;
   }
   template<typename K, typename T>
   void from_variant( const fcl::variant& var,  std::multimap<K, T>& vo )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      vo.clear();
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as< std::pair<K,T> >() );
   }


   template<typename T>
   void to_variant( const std::set<T>& var,  fcl::variant& vo )
   {
       if( var.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
       variants vars(var.size());
       size_t i = 0;
       for( auto itr = var.begin(); itr != var.end(); ++itr, ++i )
          vars[i] = fcl::variant(*itr);
       vo = std::move(vars);
   }
   template<typename T>
   void from_variant( const fcl::variant& var,  std::set<T>& vo )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      vo.clear();
      //vo.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         vo.insert( itr->as<T>() );
   }

   /** @ingroup Serializable */
   template<typename T>
   void from_variant( const fcl::variant& var, std::deque<T>& tmp )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      tmp.clear();
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         tmp.push_back( itr->as<T>() );
   }

   /** @ingroup Serializable */
   template<typename T>
   void to_variant( const std::deque<T>& t, fcl::variant& v )
   {
      if( t.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      variants vars(t.size());
      for( size_t i = 0; i < t.size(); ++i )
         vars[i] = fcl::variant(t[i]);
      v = std::move(vars);
   }

   /** @ingroup Serializable */
   template<typename T, typename... U>
   void from_variant( const fcl::variant& v, boost::container::deque<T, U...>& d )
   {
      const variants& vars = v.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      d.clear();
      d.resize( vars.size() );
      for( uint32_t i = 0; i < vars.size(); ++i ) {
         from_variant( vars[i], d[i] );
      }
   }

   /** @ingroup Serializable */
   template<typename T, typename... U>
   void to_variant( const boost::container::deque<T, U...>& d, fcl::variant& vo )
   {
      if( d.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      variants vars(d.size());
      for( size_t i = 0; i < d.size(); ++i ) {
         vars[i] = fcl::variant( d[i] );
      }
      vo = std::move( vars );
   }

   /** @ingroup Serializable */
   template<typename T>
   void from_variant( const fcl::variant& var, std::vector<T>& tmp )
   {
      const variants& vars = var.get_array();
      if( vars.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      tmp.clear();
      tmp.reserve( vars.size() );
      for( auto itr = vars.begin(); itr != vars.end(); ++itr )
         tmp.push_back( itr->as<T>() );
   }

   /** @ingroup Serializable */
   template<typename T>
   void to_variant( const std::vector<T>& t, fcl::variant& v )
   {
      if( t.size() > MAX_NUM_ARRAY_ELEMENTS ) throw std::range_error( "too large" );
      variants vars(t.size());
       for( size_t i = 0; i < t.size(); ++i )
          vars[i] = fcl::variant(t[i]);
       v = std::move(vars);
   }

   /** @ingroup Serializable */
   template<typename T, std::size_t S>
   void from_variant( const fcl::variant& var, std::array<T,S>& tmp )
   {
      const variants& vars = var.get_array();
      if( vars.size() != S) throw std::length_error( "mismatch between variant vector size and expected array size" );
      for( std::size_t i = 0; i < S; ++i )
         tmp[i] = vars.at(i).as<T>();
   }

   /** @ingroup Serializable */
   template<typename T, std::size_t S>
   void to_variant( const std::array<T,S>& t, fcl::variant& v )
   {
      variants vars(S);
      for( std::size_t i = 0; i < S; ++i )
         vars[i] = fcl::variant(t[i]);
      v = std::move(vars);
   }

   /** @ingroup Serializable */
   template<typename T>
   void to_variant( const std::initializer_list<T>& t, fcl::variant& v )
   {
      auto sz{t.size()};
      variants vars(sz);
      for( std::size_t i = 0; i < sz; ++i )
         vars[i] = fcl::variant(*(t.begin()+i));
      v = std::move(vars);
   }

   /** @ingroup Serializable */
   template<typename A, typename B>
   void to_variant( const std::pair<A,B>& t, fcl::variant& v )
   {
      variants vars(2);
      vars[0] = fcl::variant(t.first);
      vars[1] = fcl::variant(t.second);
      v = std::move(vars);
   }

   /** @ingroup Serializable */
   template<typename A, typename B>
   void from_variant( const fcl::variant& v, std::pair<A,B>& p )
   {
      const variants& vars = v.get_array();
      if( vars.size() > 0 )
         vars[0].as<A>( p.first );
      if( vars.size() > 1 )
         vars[1].as<B>( p.second );
   }

   template<typename T>
   variant::variant( const T& val )
   {
      to_variant( val, *this );
   }

   template<typename T>
   variant::variant( const T& val, const fcl::yield_function_t& yield )
   {
      to_variant( val, *this, yield );
   }

   #ifdef __APPLE__
   inline void to_variant( size_t s, fcl::variant& v ) { v = fcl::variant(uint64_t(s)); }
   #endif
   template<typename T>
   void to_variant( const std::shared_ptr<T>& var,  fcl::variant& vo )
   {
      if( var ) to_variant( *var, vo );
      else vo = nullptr;
   }

   template<typename T>
   void from_variant( const fcl::variant& var,  std::shared_ptr<T>& vo )
   {
      if( var.is_null() ) vo = nullptr;
      else if( vo ) from_variant( var, *vo );
      else {
          vo = std::make_shared<T>();
          from_variant( var, *vo );
      }
   }
   template<typename T>
   void to_variant( const std::unique_ptr<T>& var,  fcl::variant& vo )
   {
      if( var ) to_variant( *var, vo );
      else vo = nullptr;
   }

   template<typename T>
   void from_variant( const fcl::variant& var,  std::unique_ptr<T>& vo )
   {
      if( var.is_null() ) vo.reset();
      else if( vo ) from_variant( var, *vo );
      else {
          vo.reset( new T() );
          from_variant( var, *vo );
      }
   }


   template<typename T>
   void to_variant( const safe<T>& s, fcl::variant& v ) { v = s.value; }

   template<typename T>
   void from_variant( const fcl::variant& v, safe<T>& s ) { s.value = v.as_uint64(); }

   template<typename T, typename... Args> void to_variant( const boost::multi_index_container<T,Args...>& c, fcl::variant& v )
   {
       variants vars;
       vars.reserve( c.size() );
       for( const auto& item : c )
          vars.emplace_back( fcl::variant(item) );
       v = std::move(vars);
   }

   template<typename T, typename... Args> void from_variant( const fcl::variant& v, boost::multi_index_container<T,Args...>& c )
   {
      const variants& vars = v.get_array();
      c.clear();
      for( const auto& item : vars )
         c.insert( item.as<T>() );
   }
   template<typename T> void to_variant( const boost::multiprecision::number<T>& n, fcl::variant& v ) {
      v = n.str();
   }
   template<typename T> void from_variant( const fcl::variant& v, boost::multiprecision::number<T>& n ) {
      n = boost::multiprecision::number<T>(v.get_string());
   }

   fcl::variant operator + ( const fcl::variant& a, const fcl::variant& b );
   fcl::variant operator - ( const fcl::variant& a, const fcl::variant& b );
   fcl::variant operator * ( const fcl::variant& a, const fcl::variant& b );
   fcl::variant operator / ( const fcl::variant& a, const fcl::variant& b );

   bool operator == ( const fcl::variant& a, const fcl::variant& b );
   bool operator != ( const fcl::variant& a, const fcl::variant& b );
   bool operator < ( const fcl::variant& a, const fcl::variant& b );
   bool operator > ( const fcl::variant& a, const fcl::variant& b );
   bool operator ! ( const fcl::variant& a );
} // namespace fcl

#include <fcl/reflect/reflect.hpp>
FCL_REFLECT_TYPENAME( fcl::variant )
FCL_REFLECT_ENUM( fcl::variant::type_id, (null_type)(int64_type)(uint64_type)(double_type)(bool_type)(string_type)(array_type)(object_type)(blob_type) )
FCL_REFLECT( fcl::blob, (data) );
