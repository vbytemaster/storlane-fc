#pragma once
#include <fcl/raw/raw_variant.hpp>
#include <fcl/reflect/reflect.hpp>
#include <fcl/io/datastream.hpp>
#include <fcl/io/varint.hpp>
#include <fcl/core/fwd.hpp>
#include <fcl/core/array.hpp>
#include <fcl/core/time.hpp>
#include <fcl/core/filesystem.hpp>
#include <fcl/exception/exception.hpp>
#include <fcl/core/safe.hpp>
#include <fcl/variant/static_variant.hpp>
#include <fcl/raw/raw_fwd.hpp>
#include <fcl/crypto/hex.hpp>
#include <fcl/core/bitutil.hpp>

#include <boost/multiprecision/cpp_int.hpp>

#include <array>
#include <map>
#include <deque>
#include <list>

namespace fcl {
    namespace raw {

    template<size_t Size>
    using UInt = boost::multiprecision::number<
          boost::multiprecision::cpp_int_backend<Size, Size, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void> >;
    template<size_t Size>
    using Int = boost::multiprecision::number<
          boost::multiprecision::cpp_int_backend<Size, Size, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void> >;
    template<typename Stream> void pack( Stream& s, const UInt<256>& n );
    template<typename Stream> void unpack( Stream& s,  UInt<256>& n );
    template<typename Stream> void pack( Stream& s, const Int<256>& n );
    template<typename Stream> void unpack( Stream& s,  Int<256>& n );
    template<typename Stream, typename T> void pack( Stream& s, const boost::multiprecision::number<T>& n );
    template<typename Stream, typename T> void unpack( Stream& s,  boost::multiprecision::number<T>& n );
    template<typename Stream> void pack( Stream& s, const fcl::dynamic_bitset& bs );
    template<typename Stream> void unpack( Stream& s,  fcl::dynamic_bitset& bs );

    template<typename Stream, typename Arg0, typename... Args>
    inline void pack( Stream& s, const Arg0& a0, const Args&... args ) {
       pack( s, a0 );
       pack( s, args... );
    }
    template<typename Stream, typename Arg0, typename... Args>
    inline void unpack( Stream& s, Arg0& a0, Args&... args ) {
       unpack( s, a0 );
       unpack( s, args... );
    }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::exception& e )
    {
       fcl::raw::pack( s, e.code() );
       fcl::raw::pack( s, std::string(e.name()) );
       fcl::raw::pack( s, std::string(e.what()) );
       fcl::raw::pack( s, e.get_log() );
    }
    template<typename Stream>
    inline void unpack( Stream& s, fcl::exception& e )
    {
       int64_t code;
       std::string name, what;
       log_messages msgs;

       fcl::raw::unpack( s, code );
       fcl::raw::unpack( s, name );
       fcl::raw::unpack( s, what );
       fcl::raw::unpack( s, msgs );

       e = fcl::exception( std::move(msgs), code, name, what );
    }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::log_message& msg )
    {
       fcl::raw::pack( s, variant(msg) );
    }
    template<typename Stream>
    inline void unpack( Stream& s, fcl::log_message& msg )
    {
       fcl::variant vmsg;
       fcl::raw::unpack( s, vmsg );
       msg = vmsg.as<log_message>();
    }

    template<typename Stream>
    inline void pack( Stream& s, const std::filesystem::path& tp )
    {
       fcl::raw::pack( s, tp.generic_string() );
    }

    template<typename Stream>
    inline void unpack( Stream& s, std::filesystem::path& tp )
    {
       std::string p;
       fcl::raw::unpack( s, p );
       tp = p;
    }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::time_point_sec& tp )
    {
       uint32_t usec = tp.sec_since_epoch();
       s.write( (const char*)&usec, sizeof(usec) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fcl::time_point_sec& tp )
    { try {
       uint32_t sec;
       s.read( (char*)&sec, sizeof(sec) );
       tp = fcl::time_point_sec{sec};
    } FCL_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::time_point& tp )
    {
       uint64_t usec = tp.time_since_epoch().count();
       s.write( (const char*)&usec, sizeof(usec) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fcl::time_point& tp )
    { try {
       uint64_t usec;
       s.read( (char*)&usec, sizeof(usec) );
       tp = fcl::time_point() + fcl::microseconds(usec);
    } FCL_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::microseconds& usec )
    {
       uint64_t usec_as_int64 = usec.count();
       s.write( (const char*)&usec_as_int64, sizeof(usec_as_int64) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fcl::microseconds& usec )
    { try {
       uint64_t usec_as_int64;
       s.read( (char*)&usec_as_int64, sizeof(usec_as_int64) );
       usec = fcl::microseconds(usec_as_int64);
    } FCL_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream, NotTrivialScalar T, size_t N>
    inline void pack( Stream& s, const fcl::array<T,N>& v)
    {
       static_assert( N <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       for (uint64_t i = 0; i < N; ++i)
         fcl::raw::pack(s, v.data[i]);
    }

    template<typename Stream, TrivialScalar T, size_t N>
    inline void pack( Stream& s, const fcl::array<T,N>& v)
    {
       static_assert( N <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       s.write((const char*)&v.data[0], N*sizeof(T));
    }

    template<typename Stream, NotTrivialScalar T, size_t N>
    inline void unpack( Stream& s, fcl::array<T,N>& v)
    { try {
       static_assert( N <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       for (uint64_t i = 0; i < N; ++i)
          fcl::raw::unpack(s, v.data[i]);
    } FCL_RETHROW_EXCEPTIONS( warn, "fcl::array<${type},${length}>", ("type",fcl::get_typename<T>::name())("length",N) ) }

    template<typename Stream, TrivialScalar T, size_t N>
    inline void unpack( Stream& s, fcl::array<T,N>& v)
    { try {
       static_assert( N <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       s.read((char*)&v.data[0], N*sizeof(T));
    } FCL_RETHROW_EXCEPTIONS( warn, "fcl::array<${type},${length}>", ("type",fcl::get_typename<T>::name())("length",N) ) }

    template<typename Stream, typename T, size_t N>
    requires (!std::is_same_v<std::remove_cv_t<T>, char>)
    inline void pack( Stream& s, T (&v)[N]) {
      fcl::raw::pack( s, unsigned_int((uint32_t)N) );
      for (uint64_t i = 0; i < N; ++i)
         fcl::raw::pack(s, v[i]);
    }

    template<typename Stream, typename T, size_t N>
    requires (!std::is_same_v<std::remove_cv_t<T>, char>)
    inline void unpack( Stream& s, T (&v)[N])
    { try {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value == N );
      for (uint64_t i = 0; i < N; ++i)
         fcl::raw::unpack(s, v[i]);
    } FCL_RETHROW_EXCEPTIONS( warn, "${type} (&v)[${length}]", ("type",fcl::get_typename<T>::name())("length",N) ) }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::shared_ptr<T>& v)
    {
      fcl::raw::pack( s, bool(!!v) );
      if( !!v ) fcl::raw::pack( s, *v );
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::shared_ptr<T>& v)
    { try {
      bool b; fcl::raw::unpack( s, b );
      if( b ) {
         // want to be able to unpack std::shared_ptr<const T>
         auto tmp = std::make_shared<std::remove_const_t<T>>();
         fcl::raw::unpack( s, *tmp );
         v = std::move(tmp);
      } else { v.reset(); }
    } FCL_RETHROW_EXCEPTIONS( warn, "std::shared_ptr<T>", ("type",fcl::get_typename<T>::name()) ) }

    template<typename Stream> inline void pack( Stream& s, const signed_int& v ) {
      uint32_t val = (v.value<<1) ^ (v.value>>31);              //apply zigzag encoding
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      } while( val );
    }

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      }while( val );
    }

    template<typename Stream> inline void unpack( Stream& s, signed_int& vi ) {
      uint32_t v = 0; char b = 0; int by = 0;
      do {
        s.get(b);
        v |= uint32_t(uint8_t(b) & 0x7f) << by;
        by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value= (v>>1) ^ (~(v&1)+1ull);                         //reverse zigzag encoding
    }

    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi ) {
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
          s.get(b);
          v |= uint32_t(uint8_t(b) & 0x7f) << by;
          by += 7;
      } while( uint8_t(b) & 0x80 && by < 32 );
      vi.value = static_cast<uint32_t>(v);
    }

    template<typename Stream> inline void pack( Stream& s, const char* v ) {
       size_t sz = std::strlen(v);
       FCL_ASSERT( sz <= MAX_SIZE_OF_BYTE_ARRAYS );
       fcl::raw::pack( s, unsigned_int(sz));
       if( sz ) s.write( v, sz );
    }

    template<typename Stream, typename T>
    void pack( Stream& s, const safe<T>& v ) { fcl::raw::pack( s, v.value ); }

    template<typename Stream, typename T>
    void unpack( Stream& s, fcl::safe<T>& v ) { fcl::raw::unpack( s, v.value ); }

    template<typename Stream, typename T, unsigned int S, typename Align>
    void pack( Stream& s, const fcl::fwd<T,S,Align>& v ) {
       fcl::raw::pack( s, *v );
    }

    template<typename Stream, typename T, unsigned int S, typename Align>
    void unpack( Stream& s, fcl::fwd<T,S,Align>& v ) {
       fcl::raw::unpack( s, *v );
    }

    // optional
    template<typename Stream, typename T>
    void pack( Stream& s, const std::optional<T>& v ) {
      fcl::raw::pack( s, v.has_value() );
      if( v ) fcl::raw::pack( s, *v );
    }

    template<typename Stream, typename T>
    void unpack( Stream& s, std::optional<T>& v )
    { try {
      bool b; fcl::raw::unpack( s, b );
      if( b ) { v = T(); fcl::raw::unpack( s, *v ); }
      else { v.reset(); } // in case v has already has a value
    } FCL_RETHROW_EXCEPTIONS( warn, "optional<${type}>", ("type",fcl::get_typename<T>::name() ) ) }

    // std::vector<char>
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value ) {
      FCL_ASSERT( value.size() <= MAX_SIZE_OF_BYTE_ARRAYS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      if( value.size() )
        s.write( &value.front(), (uint32_t)value.size() );
    }
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_SIZE_OF_BYTE_ARRAYS );
      value.resize(size.value);
      if( value.size() )
        s.read( value.data(), value.size() );
    }

    // fcl::string
    template<typename Stream> inline void pack( Stream& s, const std::string& v )  {
      FCL_ASSERT( v.size() <= MAX_SIZE_OF_BYTE_ARRAYS );
      fcl::raw::pack( s, unsigned_int((uint32_t)v.size()));
      if( v.size() ) s.write( v.c_str(), v.size() );
    }

    template<typename Stream> inline void unpack( Stream& s, std::string& v )  {
      std::vector<char> tmp; fcl::raw::unpack(s,tmp);
      if( tmp.size() )
         v = std::string(tmp.data(),tmp.data()+tmp.size());
      else v = std::string();
    }
    // bool
    template<typename Stream> inline void pack( Stream& s, const bool& v ) { fcl::raw::pack( s, uint8_t(v) );             }
    template<typename Stream> inline void unpack( Stream& s, bool& v )
    {
       uint8_t b;
       fcl::raw::unpack( s, b );
       FCL_ASSERT( (b & ~1) == 0 );
       v=(b!=0);
    }

    namespace detail {

      template<typename Stream, typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        void operator()( const char* name )const {
          fcl::raw::pack( s, c.*p );
        }
        private:
          const Class& c;
          Stream&      s;
      };

      template<typename Stream, typename Class>
      struct unpack_object_visitor : public fcl::reflector_init_visitor<Class> {
        unpack_object_visitor(Class& _c, Stream& _s)
        : fcl::reflector_init_visitor<Class>(_c), s(_s){}

        template<typename T, typename C, T(C::*p)>
        inline void operator()( const char* name )const
        { try {
          // `const_cast` because we want to be able to populate `const` members of a class, which
          // are typically set only in the constructor, but because of the `reflect` and `raw`
          // interfaces, we have to create the object first and then populate the members.
          // -------------------------------------------------------------------------------------
          fcl::raw::unpack( s, const_cast<std::remove_const_t<T>&>(this->obj.*p) );
        } FCL_RETHROW_EXCEPTIONS( warn, "Error unpacking field ${field}", ("field",name) ) }

        private:
          Stream& s;
      };

      template<typename IsClass=fcl::true_type>
      struct if_class{
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { s << v; }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { s >> v; }
      };

      template<>
      struct if_class<fcl::false_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          s.write( (char*)&v, sizeof(v) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          s.read( (char*)&v, sizeof(v) );
        }
      };

      template<typename IsEnum=fcl::false_type>
      struct if_enum {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          fcl::reflector<T>::visit( pack_object_visitor<Stream,T>( v, s ) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          fcl::reflector<T>::visit( unpack_object_visitor<Stream,T>( v, s ) );
        }
      };
      template<>
      struct if_enum<fcl::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          fcl::raw::pack(s, (int64_t)v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          int64_t temp;
          fcl::raw::unpack(s, temp);
          v = (T)temp;
        }
      };

      template<typename IsReflected=fcl::false_type>
      struct if_reflected {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          if_class<typename fcl::is_class<T>::type>::pack(s,v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          if_class<typename fcl::is_class<T>::type>::unpack(s,v);
        }
      };
      template<>
      struct if_reflected<fcl::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          if_enum< typename fcl::reflector<T>::is_enum >::pack(s,v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          if_enum< typename fcl::reflector<T>::is_enum >::unpack(s,v);
        }
      };

    } // namesapce detail

    // allow users to verify version of fc calls reflector_init on unpacked reflected types
    constexpr bool has_feature_reflector_init_on_unpacked_reflected_types = true;

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::unordered_set<T>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fcl::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::unordered_set<T>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.clear();
      value.reserve(size.value);
      for( uint32_t i = 0; i < size.value; ++i )
      {
          T tmp;
          fcl::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }

    template<typename Stream, typename... Ts >
    inline void pack( Stream& s, const std::tuple<Ts...>& tup ) {
       auto l = [&s](const auto&... v) { fcl::raw::pack( s, v... ); };
       std::apply(l, tup);
    }
    template<typename Stream, typename... Ts >
    inline void unpack( Stream& s, std::tuple<Ts...>& tup ) {
       auto l = [&s](auto&... v) { fcl::raw::unpack( s, v... ); };
       std::apply(l, tup);
    }

    template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::pair<K,V>& value ) {
       fcl::raw::pack( s, value.first );
       fcl::raw::pack( s, value.second );
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::pair<K,V>& value )
    {
       fcl::raw::unpack( s, value.first );
       fcl::raw::unpack( s, value.second );
    }

   template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::unordered_map<K,V>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fcl::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::unordered_map<K,V>& value )
    {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.clear();
      value.reserve(size.value);
      for( uint32_t i = 0; i < size.value; ++i )
      {
          std::pair<K,V> tmp;
          fcl::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }
    template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::map<K,V>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fcl::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::map<K,V>& value )
    {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.clear();
      for( uint32_t i = 0; i < size.value; ++i )
      {
          std::pair<K,V> tmp;
          fcl::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::deque<T>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      for( const auto& i : value ) {
         fcl::raw::pack( s, i );
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::deque<T>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.resize(size.value);
      for( auto& i : value ) {
         fcl::raw::unpack( s, i );
      }
    }

    template<typename Stream, typename T, typename... U>
    inline void pack( Stream& s, const boost::container::deque<T, U...>& value ) {
       FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
       fcl::raw::pack( s, unsigned_int( (uint32_t) value.size() ) );
       for( const auto& i : value ) {
          fcl::raw::pack( s, i );
       }
    }

    template<typename Stream, typename T, typename... U>
    inline void unpack( Stream& s, boost::container::deque<T, U...>& value ) {
       unsigned_int size;
       fcl::raw::unpack( s, size );
       FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
       value.resize( size.value );
       for( auto& i : value ) {
          fcl::raw::unpack( s, i );
       }
    }

    template<typename Stream>
    inline void pack( Stream& s, const fcl::dynamic_bitset& value ) {
      // pack the size of the bitset, not the number of blocks
      const auto num_blocks = value.num_blocks();
      FCL_ASSERT( num_blocks <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int(value.size()) );
      [[maybe_unused]] constexpr size_t word_size = sizeof(fcl::dynamic_bitset::block_type) * CHAR_BIT;
      assert(num_blocks == (value.size() + word_size - 1) / word_size);
      // convert bitset to a vector of blocks
      std::vector<fcl::dynamic_bitset::block_type> blocks;
      blocks.resize(num_blocks);
      boost::to_block_range(value, blocks.begin());
      // pack the blocks
      for (const auto& b: blocks) {
         fcl::raw::pack( s, b );
      }
    }

    template<typename Stream>
    inline void unpack( Stream& s, fcl::dynamic_bitset& value ) {
      // the packed size is the number of bits in the set, not the number of blocks
      unsigned_int size; fcl::raw::unpack( s, size );
      constexpr size_t word_size = sizeof(fcl::dynamic_bitset::block_type) * CHAR_BIT;
      size_t num_blocks = (size + word_size - 1) / word_size;
      FCL_ASSERT( num_blocks <= MAX_NUM_ARRAY_ELEMENTS );
      std::vector<fcl::dynamic_bitset::block_type> blocks(num_blocks);
      for( size_t i = 0; i < num_blocks; ++i ) {
         fcl::raw::unpack( s, blocks[i] );
      }
      value = { blocks.cbegin(), blocks.cend() };
      value.resize(size.value);
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::vector<T>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      for( const auto& i : value ) {
         fcl::raw::pack( s, i );
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::vector<T>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.resize(size.value);
      for( auto& i : value ) {
         fcl::raw::unpack( s, i );
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::list<T>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      for( const auto& i : value ) {
         fcl::raw::pack( s, i );
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::list<T>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.clear();
      while( size.value-- ) {
         T i;
         fcl::raw::unpack( s, i );
         value.emplace_back( std::move( i ) );
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::set<T>& value ) {
      FCL_ASSERT( value.size() <= MAX_NUM_ARRAY_ELEMENTS );
      fcl::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fcl::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::set<T>& value ) {
      unsigned_int size; fcl::raw::unpack( s, size );
      FCL_ASSERT( size.value <= MAX_NUM_ARRAY_ELEMENTS );
      value.clear();
      for( uint64_t i = 0; i < size.value; ++i )
      {
        T tmp;
        fcl::raw::unpack( s, tmp );
        value.insert( std::move(tmp) );
      }
    }

    template<typename Stream, TrivialScalar T, std::size_t S>
    inline void pack( Stream& s, const std::array<T, S>& value )
    {
       static_assert( S <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       s.write((const char*)value.data(), S * sizeof(T));
    }

    template<typename Stream, NotTrivialScalar T, std::size_t S>
    inline void pack( Stream& s, const std::array<T, S>& value )
    {
       static_assert( S <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       for( std::size_t i = 0; i < S; ++i ) {
          fcl::raw::pack( s, value[i] );
       }
    }

    template<typename Stream, TrivialScalar T, std::size_t S>
    inline void unpack( Stream& s, std::array<T, S>& value )
    {
       static_assert( S <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       s.read((char*)value.data(), S * sizeof(T));
    }

    template<typename Stream, NotTrivialScalar T, std::size_t S>
    inline void unpack( Stream& s, std::array<T, S>& value )
    {
       static_assert( S <= MAX_NUM_ARRAY_ELEMENTS, "number of elements in array is too large" );
       for( std::size_t i = 0; i < S; ++i ) {
          fcl::raw::unpack( s, value[i] );
       }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const T& v ) {
      fcl::raw::detail::if_reflected< typename fcl::reflector<T>::is_defined >::pack(s,v);
    }
    template<typename Stream, typename T>
    inline void unpack( Stream& s, T& v )
    { try {
      fcl::raw::detail::if_reflected< typename fcl::reflector<T>::is_defined >::unpack(s,v);
    } FCL_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fcl::get_typename<T>::name() ) ) }

    template<typename T>
    inline size_t pack_size(  const T& v )
    {
      datastream<size_t> ps;
      fcl::raw::pack(ps,v );
      return ps.tellp();
    }

    template<typename T>
    inline std::vector<char> pack(  const T& v ) {
      datastream<size_t> ps;
      fcl::raw::pack(ps,v );
      std::vector<char> vec(ps.tellp());

      if( vec.size() ) {
        datastream<char*>  ds( vec.data(), size_t(vec.size()) );
        fcl::raw::pack(ds,v);
      }
      return vec;
    }

    template<typename T, typename... Next>
    inline std::vector<char> pack(  const T& v, Next... next ) {
      datastream<size_t> ps;
      fcl::raw::pack(ps,v,next...);
      std::vector<char> vec(ps.tellp());

      if( vec.size() ) {
        datastream<char*>  ds( vec.data(), size_t(vec.size()) );
        fcl::raw::pack(ds,v,next...);
      }
      return vec;
    }


    template<typename T>
    inline T unpack( const std::vector<char>& s )
    { try  {
      T tmp;
      datastream<const char*>  ds( s.data(), size_t(s.size()) );
      fcl::raw::unpack(ds,tmp);
      return tmp;
    } FCL_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fcl::get_typename<T>::name() ) ) }

    template<typename T>
    inline void unpack( const std::vector<char>& s, T& tmp )
    { try  {
      datastream<const char*>  ds( s.data(), size_t(s.size()) );
      fcl::raw::unpack(ds,tmp);
    } FCL_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fcl::get_typename<T>::name() ) ) }

    template<typename T>
    inline void pack( char* d, uint32_t s, const T& v ) {
      datastream<char*> ds(d,s);
      fcl::raw::pack(ds,v );
    }

    template<typename T>
    inline T unpack( const char* d, uint32_t s )
    { try {
      T v;
      datastream<const char*>  ds( d, s );
      fcl::raw::unpack(ds,v);
      return v;
    } FCL_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fcl::get_typename<T>::name() ) ) }

    template<typename T>
    inline void unpack( const char* d, uint32_t s, T& v )
    { try {
      datastream<const char*>  ds( d, s );
      fcl::raw::unpack(ds,v);
    } FCL_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fcl::get_typename<T>::name() ) ) }

   template<typename Stream>
   struct pack_static_variant
   {
      Stream& stream;
      pack_static_variant( Stream& s ):stream(s){}

      typedef void result_type;
      template<typename T> void operator()( const T& v )const
      {
         fcl::raw::pack( stream, v );
      }
   };

   template<typename Stream>
   struct unpack_static_variant
   {
      Stream& stream;
      unpack_static_variant( Stream& s ):stream(s){}

      typedef void result_type;
      template<typename T> void operator()( T& v )const
      {
         fcl::raw::unpack( stream, v );
      }
   };


    template<typename Stream, typename... T>
    void pack( Stream& s, const std::variant<T...>& sv )
    {
       fcl::raw::pack( s, unsigned_int(sv.index()) );
       std::visit( pack_static_variant<Stream>(s), sv );
    }

    template<typename Stream, typename... T> void unpack( Stream& s, std::variant<T...>& sv )
    {
       unsigned_int w;
       fcl::raw::unpack( s, w );
       fcl::from_index(sv, w.value);
       std::visit( unpack_static_variant<Stream>(s), sv );
    }



    template<typename Stream, typename T> void pack( Stream& s, const boost::multiprecision::number<T>& n ) {
      static_assert( sizeof( n ) == (std::numeric_limits<boost::multiprecision::number<T>>::digits+1)/8, "unexpected padding" );
      s.write( (const char*)&n, sizeof(n) );
    }
    template<typename Stream, typename T> void unpack( Stream& s,  boost::multiprecision::number<T>& n ) {
      static_assert( sizeof( n ) == (std::numeric_limits<boost::multiprecision::number<T>>::digits+1)/8, "unexpected padding" );
      s.read( (char*)&n, sizeof(n) );
    }

    template<typename Stream> void pack( Stream& s, const UInt<256>& n ) {
       pack( s, static_cast<UInt<128>>(n) );
       pack( s, static_cast<UInt<128>>(n >> 128) );
    }
    template<typename Stream> void unpack( Stream& s,  UInt<256>& n ) {
       UInt<128> tmp[2];
       unpack( s, tmp[0] );
       unpack( s, tmp[1] );
       n = tmp[1];
       n <<= 128;
       n |= tmp[0];
    }

} } // namespace fcl::raw
